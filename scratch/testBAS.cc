/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Rohan Patidar <rpatidar@uw.edu>
 */


// This script outputs the throughput at 6Mbps rate with respect to distace 
// between the nodes for UDP, 

// The simulation assumes a single station in an infrastructure network:
//
//  STA     AP
//    *     *
//    |     |
//   n1     n2

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/gnuplot.h"
#include <cmath>
#include <iostream>
#include <fstream>
//#include "ns3/netanim-module.h"


#define PI 3.14159265


using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Test1Example");

int main (int argc, char *argv[])
{
     
  uint32_t nWifi = 5;//节点数目
  uint32_t cwmin = 31; //初始退避窗口
  uint32_t cwmax = 1023; //最大退避窗口

//  int x = 0;
//  int y = 0;
//
//  fstream fout;
//  fout.open("output.txt");

  double simulationTime = 50; //seconds仿真时间
  bool shortGuardInterval = false;  

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("simulationTime", "simulation time", simulationTime);
  cmd.AddValue ("cwmin", "Minimum contention window size", cwmin);
  cmd.AddValue ("cwmax", "Maximum contention window size", cwmax);
  cmd.Parse (argc, argv);

// Time::SetResolution (Time::NS);
// LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
// LogComponentEnable ("UdpServer", LOG_LEVEL_ALL);


// No fragmentation and no RTS/CTS

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",StringValue ("99999990"));//设置数据包分包长度
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));//设置RTS阈值大小
  Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSsrc", UintegerValue (10000)); //backoff phase for RTS； RTS/CTS模式的重传次数
  Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue (10000)); //backoff phase for packet (using 8 it stay at 1023 twice as per standard) //基本接入模式的重传次数

//Config::SetDefault ("ns3::Dcf::MinCw", UintegerValue (cwmin));
//Config::SetDefault ("ns3::Dcf::MaxCw", UintegerValue (cwmax));

  Config::SetDefault ("ns3::Txop::MinCw", UintegerValue (cwmin)); //设置初始退避窗口
  Config::SetDefault ("ns3::Txop::MaxCw", UintegerValue (cwmax)); //设置最大退避窗口
 
  uint32_t payloadsize = 1900; //数据包长
   
   //for (uint32_t payloadsize = 1250; payloadsize < 7000; payloadsize += 625)   
      
       {

         //uint32_t payloadsize = 1400;
          std::cout << payloadsize << " bytes" << std::endl;

          //testFile<<nWifi<<""<<std::endl;
          NodeContainer wifiStaNode;
          wifiStaNode.Create (nWifi); //创建节点
          NodeContainer wifiApNode;  
          wifiApNode.Create (1);      //a AP
          
          // Set channel type
          YansWifiChannelHelper channel = YansWifiChannelHelper::Default (); //设置默认参数下的助手
          YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();//
          phy.SetChannel (channel.Create ());//将信道与物理层助手相连
          // Set guard interval
          phy.Set ("ShortGuardEnabled", BooleanValue (shortGuardInterval));
          
          //设置MAC层
          WifiMacHelper mac;
          WifiHelper wifi;
          wifi.SetStandard (WIFI_PHY_STANDARD_80211a);//选择wifi标准

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"), "ControlMode", StringValue("OfdmRate6Mbps"));//设置数据包发送速率和控制帧发送速率

          Ssid ssid = Ssid ("ns3-80211a");
          mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

          //设置网络设备，mac层设置

          NetDeviceContainer staDevice;//网络设备容器
          staDevice = wifi.Install (phy, mac, wifiStaNode); //将物理层、MAC层、节点置于网络设备中

          mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));

          NetDeviceContainer apDevice;
          apDevice = wifi.Install (phy, mac, wifiApNode); //AP配置物理层、MAC层等置于网络设备中

          // mobility移动模型,一般在创建node时设定.
          MobilityHelper mobility;//设置节点位置
          Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
          positionAlloc->Add (Vector (0.0, 0.0, 0.0));//第一个位置参数

          float rho=0.01; //半径

        //设置各节点的位置，环绕着以圆心（0,0），半径0.5的圆分布
       for (double i=0; i<nWifi; i++)
         {
           double theta = i*2*PI/nWifi;
           positionAlloc->Add (Vector (rho * cos(theta), rho * sin(theta), 0.0));  //附加多个位置参数在队列后面
         } 
          mobility.SetPositionAllocator (positionAlloc); //位置参数队列与位置分配器绑定

          mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");//移动模型选择

          mobility.Install (wifiApNode);//将AP置于圆心，分配位置
          mobility.Install (wifiStaNode); //站点Station置于圆上分布

          /* Internet stack*/
          InternetStackHelper stack;//协议栈
          stack.Install (wifiApNode);
          stack.Install (wifiStaNode);

          Ipv4AddressHelper address; //设置IP协议地址
          address.SetBase ("192.168.1.0", "255.255.255.0");


          Ipv4InterfaceContainer staNodeInterface;
          Ipv4InterfaceContainer apNodeInterface;

          staNodeInterface = address.Assign (staDevice);
          apNodeInterface = address.Assign (apDevice);

          /* Setting applications */
          //ApplicationContainer serverApp;
          
          //UDP flow
          uint16_t port = 8000; 
          UdpServerHelper server (port); //服务端端口
          ApplicationContainer serverApp;
          serverApp = server.Install (wifiApNode.Get (0));
          serverApp.Start (Seconds (0.0));
          serverApp.Stop (Seconds (simulationTime + 1));

          UdpClientHelper client (apNodeInterface.GetAddress (0), port);//将服务器端口地址告诉客户端
          client.SetAttribute ("MaxPackets", UintegerValue (4294967295u)); //数据包
          client.SetAttribute ("Interval", TimeValue (Time ("0.1"))); //数据包到达间隔
          client.SetAttribute ("PacketSize", UintegerValue (payloadsize)); //数据包长
          ApplicationContainer clientApp[nWifi];
          
          for (uint16_t i=0; i<=nWifi-1; i++)
          {
              clientApp[i] = client.Install (wifiStaNode.Get (i));
              clientApp[i].Start (Seconds (1.0));
              clientApp[i].Stop (Seconds (simulationTime + 1));
            }

          Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

          Simulator::Stop (Seconds (simulationTime + 1));
             
          //AnimationInterface anim("dcf.xml");
          Simulator::Run ();

          double throughput = 0;
          uint64_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();//Ap接收到的数据包个数
          std::cout << "totalPacketsThrough: " << totalPacketsThrough << std::endl;
          throughput = totalPacketsThrough * payloadsize * 8 / (simulationTime * 1000000.0); //Mbit/s计算吞吐量
          std::cout << "Throughput: " << throughput << " Mbit/s" << std::endl;
          
          Simulator::Destroy ();
         }

  return 0;
}

