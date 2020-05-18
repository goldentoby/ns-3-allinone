#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <ctime>

#include "ns3/applications-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MultimediaPacket");
void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port, uint16_t mdata);
void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev);
void dstSocketRecv (Ptr<Socket> socket);

int 
main (int argc, char *argv[])
{

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);
  int segment_size = 1446;
  std::string queue_disc_type = "CoDelQueueDisc";
  std::string bottleneckBandwidth = "10Mbps";
  std::string bottleneckDelay = "10ms";
  std::string accessBandwidth = "100Mbps";
  std::string accessDelay = "10ms";

  Ptr<Node> nSrc = CreateObject<Node> ();
  Ptr<Node> nDst = CreateObject<Node> ();
  Ptr<Node> gw0 = CreateObject<Node> ();
  Ptr<Node> gw1 = CreateObject<Node> ();

  NodeContainer c = NodeContainer (nSrc, gw0, gw1, nDst);

  std::string queue_disc = std::string ("ns3::") + queue_disc_type;
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::DelAckTimeout", TimeValue (Seconds (0)));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocketBase::LimitedTransmit", BooleanValue (false));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (segment_size));
  Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (true));
  Config::SetDefault (queue_disc + "::MaxSize", QueueSizeValue (QueueSize ("200p")));

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));
  TrafficControlHelper tch;
  tch.SetRootQueueDisc (queue_disc);

  InternetStackHelper internet;
  internet.Install (c);

  // Point-to-point links
  NodeContainer nSrcGw0 = NodeContainer (nSrc, gw0);
  NodeContainer nGw0Gw1 = NodeContainer (gw0, gw1);
  NodeContainer nGw1Dst = NodeContainer (gw1, nDst);

  // We create the channels first without any IP addressing information
  PointToPointHelper accessLink;
  accessLink.SetDeviceAttribute ("DataRate", StringValue (accessBandwidth));
  accessLink.SetChannelAttribute ("Delay", StringValue (accessDelay));

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue (bottleneckBandwidth));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue (bottleneckDelay));
  
  
  NetDeviceContainer dSrcGw0 = accessLink.Install (nSrcGw0);
  NetDeviceContainer dGw1Dst = accessLink.Install (nGw1Dst);
  NetDeviceContainer dGw0Gw1 = bottleneckLink.Install (nGw0Gw1);

  tchPfifo.Install(dSrcGw0);
  tch.Install(dGw0Gw1);
  tchPfifo.Install(dGw1Dst);

  // Later, we add IP addresses.
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces_src;
  Ipv4InterfaceContainer interfaces_dst;
  Ipv4InterfaceContainer interfaces_gateway;
  
  address.SetBase("10.10.1.0", "255.255.255.0");
  // address.NewNetwork ();
  interfaces_src = address.Assign(dSrcGw0);

  address.SetBase("10.20.1.0", "255.255.255.0");
  // address.NewNetwork ();
  interfaces_gateway = address.Assign (dGw0Gw1);

  address.SetBase("10.30.1.0", "255.255.255.0");
  // address.NewNetwork ();
  interfaces_dst = address.Assign (dGw1Dst);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ptr<Socket> srcSocket = Socket::CreateSocket (nSrc, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  uint16_t srcport = 12344;
  Ipv4Address srcaddr ("10.10.1.2");
  InetSocketAddress src = InetSocketAddress (srcaddr, srcport);
  srcSocket->Bind (src);
  // srcSocket->Bind ();

  Ptr<Socket> dstSocket = Socket::CreateSocket (nDst, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  uint16_t dstport = 12345;
  Ipv4Address dstaddr ("10.30.1.2");
  InetSocketAddress dst = InetSocketAddress (dstaddr, dstport);
  dstSocket->Bind (dst);
  dstSocket->SetRecvCallback (MakeCallback (&dstSocketRecv));
  
  AsciiTraceHelper ascii;
  bottleneckLink.EnableAsciiAll (ascii.CreateFileStream ("socket-bound-static-routing.tr"));
  bottleneckLink.EnablePcapAll ("socket-bound-static-routing");

  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnable ("MultimediaPacket", LOG_LEVEL_INFO);

  std::ifstream in;
  in.open("./scratch/traffic_h264.txt");
  // in.open("./scratch/traffic.txt");
  int v[900];
  int element;
  if (in.is_open()) {
      int i = 0;
      while (in >> element) {
          // NS_LOG_INFO ("packet size: " << element );
          v[i++] = element;
      }
  }in.close();

  int idx =0;
  for (float time = 0.0 ; time<5.0; time+=0.03, idx++){
      if (v[idx] > 50000){
        Simulator::Schedule (Seconds (time), &SendStuff, srcSocket, dstaddr, dstport, v[idx]/2);
        Simulator::Schedule (Seconds (time), &SendStuff, srcSocket, dstaddr, dstport, v[idx]/2);
      }else {
        Simulator::Schedule (Seconds (time),&SendStuff, srcSocket, dstaddr, dstport, v[idx]);
      }
  }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port, uint16_t mdata)
{
  // unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  Ptr<Packet> p = Create<Packet> (mdata);
  // NS_LOG_INFO ("time"<<now<<", data: "<<mdata);
  sock->SendTo (p, 0, InetSocketAddress (dstaddr,port));

  return;
}

void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev)
{
  sock->BindToNetDevice (netdev);
  return;
}

void
dstSocketRecv (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet = socket->RecvFrom (from);
  packet->RemoveAllPacketTags ();
  packet->RemoveAllByteTags ();
  InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
  NS_LOG_INFO ("Destination Received " << packet->GetSize () << " bytes from " << address.GetIpv4 ());
}