/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 */

/*
    头文件：
            被存放在build/NS-3目录中，编译时被复制到build/debug目录下。
*/
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


/*
    命名空间：
            将ns-3的命名空间引入当前声明域中。
            注意：此时并没有引入C++标准命名空间std，故使用C++标准库函数应加前缀std::
*/
using namespace ns3;

/*
    日志生成：
            生成一个LogComponent类型的日志对象，并命名为FirstScriptExample
*/
NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  /*

  */
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  /*
      时钟：
              此处定义为NS-3内部时钟，而并非系统时钟。
  */
  Time::SetResolution (Time::NS);
  
  /*
      实例化日志组件：
              利用LogComponentEnable函数使上方日志对象生效。
              由命名信息可知，日志被内建在echo client与echo server, 即客户端与服务端应用中。
  */
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

/*
    生成网络节点：
            创建一个NodeContainer类的对象nodes
            使用内部方法Create()创建了2个节点，将其指针存储起来。
            ❤Helper类的作用——抽象化网络设备：提供接口来创建、管理和使用网络中的对象。
*/
  NodeContainer nodes;
  nodes.Create (2);

/*
    设置网络设备和信道属性：
            初始化一个PointToPointHelper对象，称为pointToPoint
            配置属性：数据速率5Mbit/s, 传输时延2ms
            作用——在2个节点之间创建PointToPointNetDevices和wirePointToPointChannel对象。
            一个PointToPointChannel对象被创建，2个PointToPointNetDevices与之连接。
*/
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

/*
    创建设备列表：
            使用NetDeviceContainer对象来存放设备列表，称为devices
            使用内部方法Install()后，上述设备及其配置被分配到nodes的两个节点中，形成一个点到点信道。
            至此一个简单的点到点通信系统被部署：两个设备被配置在一个5Mbit/s传输速率，2ms传输时延的信道上。
*/
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

/*
    安装协议栈：
            InternetStackHelper类为节点安装网络协议栈。
            通过Install()方法实现。
*/
  InternetStackHelper stack;
  stack.Install (nodes);

/*
    IP层网络协议：
            Ipv4AddressHelper类为节点设备设置IP地址。
            可以看出，它仍然是一个Helper类，从而管理IP地址的分配。
            使用SetBase()函数设置了基IP地址——10.1.1.0和子网掩码——255.255.255.0
            地址分配规则：从基IP地址开始单调增长1。
*/
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

/*
    关联设备与IP地址：
            使用Ipv4InterfaceContainer类实例化一个对象，称为interfaces
            将devices和address用Assign()方法关联起来。
            实质：产生一个Ipv4Interface对象列表
*/
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

/*
    安装应用层：
            1、UdpEchoServerHelper类——帮助创建服务器应用的容器类，称为echoServer
                   参数为服务器和客户端所共知的一个端口号，此处为9。
            2、利用ApplicationContainer容器类安装服务器应用，给设备节点分配索引号1并完成安装。
            3、服务端应用开始于1.0s, 结束于10.0s
            4、UdpEchoClientHelper类——帮助创建客户端应用的容器类，称为echoClient
                   设置了客户端的远端地址为服务器节点的IP地址，发送第二个数据分组到9端口。
            5、设置属性：MaxPackets——允许发送的最大数据分组个数；Interval——在2个数据分组之间等待的时间；PacketSize——数据分组承载的数据数量
            6、客户端应用开始于2.0s, 结束于10.0s

*/
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

/*
    运行模拟器：
            执行方法——遍历事件列表
            Start执行后，会给服务端传送一个数据分组来开始仿真的数据传送阶段
*/
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
