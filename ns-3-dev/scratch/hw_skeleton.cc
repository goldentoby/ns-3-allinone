#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HomeWork1.1");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  std::string bottleneckBandwidth = "5Mbps";
  std::string bottleneckDelay = "5ms";
  std::string accessBandwidth = "100Mbps";
  std::string accessDelay = "0.1ms";
  
  uint32_t queueDiscSize = 1000;  //in packets
  // uint32_t queueSize = 10;        //in packets
  // uint32_t pktSize = 1458;        //in bytes. 1458 to prevent fragments
  // float startTime = 0.1f;
  // float simDuration = 60;         //in seconds

  // bool isPcapEnabled = true;
  // std::string pcapFileName = "pcapFilePfifoFast.pcap";
  // std::string cwndTrFileName = "cwndPfifoFast.tr";
  Time::SetResolution (Time::NS);
  
  NS_LOG_INFO ("Create nodes.");
  Ptr<Node> src = CreateObject<Node> ();
  Names::Add ("SrcNode", src);
  Ptr<Node> dst = CreateObject<Node> ();
  Names::Add ("DstNode", dst);
  Ptr<Node> a = CreateObject<Node> ();
  Names::Add ("RouterA", a);
  Ptr<Node> b = CreateObject<Node> ();
  Names::Add ("RouterB", b);
  NodeContainer net1 (src, a);
  NodeContainer net2 (a, b);
  NodeContainer net3 (b, dst);
  NodeContainer routers (a, b);
  NodeContainer nodes (src, dst);


  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma_access, csma_bottleNeck;
  csma_access.SetChannelAttribute ("DataRate", StringValue(accessBandwidth));
  csma_access.SetChannelAttribute ("Delay", StringValue(accessDelay));
  csma_bottleNeck.SetChannelAttribute ("DataRate", StringValue(bottleneckBandwidth));
  csma_bottleNeck.SetChannelAttribute ("Delay", StringValue(bottleneckDelay));


  // No need to set the ripRouting
  NS_LOG_INFO ("Router Config.");
  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false);
  internet.Install (routers);
  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false);
  internetNodes.Install (nodes);
  
  
  // Access link traffic control configuration
  NS_LOG_INFO ("AMQ Setting.");
  TrafficControlHelper tchPfifoFastAccess;
  tchPfifoFastAccess.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "MaxSize", StringValue ("1000p"));
  // Bottleneck link traffic control configuration
  TrafficControlHelper tchPfifo;
  tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "MaxSize", StringValue (std::to_string(queueDiscSize) + "p"));
  TrafficControlHelper tchCoDel;
  tchCoDel.SetRootQueueDisc ("ns3::CoDelQueueDisc");
  Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize", StringValue (std::to_string(queueDiscSize) + "p"));


  NS_LOG_INFO ("Assign IPv4 Addresses.");
  NetDeviceContainer ndc1 = csma_access.Install (net1);
  NetDeviceContainer ndc2 = csma_bottleNeck.Install (net2);
  NetDeviceContainer ndc3 = csma_access.Install (net3);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);
  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);
  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  
  NS_LOG_INFO ("Linkup the AMQ with NDC.");
  tchPfifoFastAccess.Install(ndc1);
  tchPfifoFastAccess.Install(ndc3);
  // TODO: default as pFIFO
  tchPfifo.Install(ndc2);
  
  
  NS_LOG_INFO ("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  
  NS_LOG_INFO ("Application Test.");
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (iic3.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
