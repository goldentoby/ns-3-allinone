#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/internet-module.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"
#include  <string>

// wireless
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/tcp-westwood.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("HomeWork1.2");
std::string dir = "MixTraffic_wireless/";
int tcpConnectNum = 1;
int udpConnectNum = 0;

void CheckQueueSize (Ptr<QueueDisc> queue,std::string queue_disc_type)
{
  double qSize = queue->GetCurrentSize ().GetValue ();
  // check queue size every 1/10 of a second
  Simulator::Schedule (Seconds (0.1), &CheckQueueSize, queue, queue_disc_type);

  std::ofstream fPlotQueue (dir + queue_disc_type + "/queueTraces/queue.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

static void CwndTrace (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd / 1446.0 << std::endl;
}

static void TraceCwnd (std::string queue_disc_type)
{
  for (uint8_t i = 0; i < tcpConnectNum; i++)
    {
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (dir + queue_disc_type + "/cwndTraces/S1-" + std::to_string (i + 1) + ".plotme");
      Config::ConnectWithoutContext ("/NodeList/" + std::to_string (i) + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndTrace,stream));
    }
}


void experiment(std::string queue_disc_type)
{
    // Set the simulation stop time in seconds
    double stopTime = 201;
    int payloadSize = 1446;
    std::string phyRate = "HtMcs7"; /* Physical layer bitrate. */
    std::string queue_disc = std::string ("ns3::") + queue_disc_type;

    std::string bottleneckBandwidth = "10Mbps";
    std::string bottleneckDelay = "10ms";
    std::string accessBandwidth = "100Mbps";
    std::string accessDelay = "10ms";

    
    NS_LOG_INFO ("Create nodes.");
    NodeContainer tcpSender;
    tcpSender.Create(tcpConnectNum);
    NodeContainer udpSender;
    udpSender.Create(udpConnectNum);

    NS_LOG_INFO ("Create gateway.");
    // NodeContainer gateway;
    // gateway.Create(2);
    NodeContainer wifiNodes;
    wifiNodes.Create (2);
    Ptr<Node> apWifiNode = wifiNodes.Get (0);
    Ptr<Node> staWifiNode = wifiNodes.Get (1);


    NS_LOG_INFO ("Configure AP and STA.");
    NS_LOG_INFO ("Configure TCP Options.");
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
    WifiMacHelper wifiMac;
    WifiHelper wifiHelper;
    wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);


    NS_LOG_INFO ("Set up Legacy Channels.");
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));


    NS_LOG_INFO ("Set up Physical Channels.");
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());
    wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
    wifiHelper.SetRemoteStationManager ( "ns3::ConstantRateWifiManager",
                                        "DataMode", StringValue (phyRate),
                                        "ControlMode", StringValue ("HtMcs0"));

    Ssid ssid = Ssid ("network");
    wifiMac.SetType ( "ns3::ApWifiMac",
                      "Ssid", SsidValue (ssid));
    NetDeviceContainer apDevice;
    apDevice = wifiHelper.Install (wifiPhy, wifiMac, apWifiNode);

    wifiMac.SetType ( "ns3::StaWifiMac",
                      "Ssid", SsidValue (ssid));
    NetDeviceContainer staDevices;
    staDevices = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);



    NS_LOG_INFO ("Configure Mobility Model.");
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (1.0, 1.0, 0.0));

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (apWifiNode);
    mobility.Install (staWifiNode);

    NS_LOG_INFO ("Create sink.");
    NodeContainer sink;
    sink.Create(1);

    NS_LOG_INFO ("tcp parameter.");
    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
    Config::SetDefault ("ns3::TcpSocket::DelAckTimeout", TimeValue (Seconds (0)));
    Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (1));
    Config::SetDefault ("ns3::TcpSocketBase::LimitedTransmit", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
    Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (true));
    Config::SetDefault (queue_disc + "::MaxSize", QueueSizeValue (QueueSize ("200p")));

    InternetStackHelper internet;
    internet.InstallAll ();
    

    NS_LOG_INFO ("setup TrafficControlHelper.");
    TrafficControlHelper tchPfifo;
    uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
    tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));
    TrafficControlHelper tch;
    tch.SetRootQueueDisc (queue_disc);


    PointToPointHelper accessLink;
    accessLink.SetDeviceAttribute ("DataRate", StringValue (accessBandwidth));
    accessLink.SetChannelAttribute ("Delay", StringValue (accessDelay));

    PointToPointHelper bottleneckLink;
    bottleneckLink.SetDeviceAttribute ("DataRate", StringValue (bottleneckBandwidth));
    bottleneckLink.SetChannelAttribute ("Delay", StringValue (bottleneckDelay));


    NS_LOG_INFO ("setup netdevice.");
    // for the access link
    NetDeviceContainer devices [tcpConnectNum];
    NetDeviceContainer udpdevices [udpConnectNum];

    for (uint8_t i = 0; i<tcpConnectNum; i++)
    {
        devices[i] = accessLink.Install(tcpSender.Get(i), apWifiNode);
        tchPfifo.Install(devices[i]);
    }

    NetDeviceContainer devices_sink;
    devices_sink = accessLink.Install(staWifiNode, sink.Get(0));
    tchPfifo.Install(devices_sink);

    // // for the bottleneck link
    // NetDeviceContainer devices_gateway;
    // devices_gateway = bottleneckLink.Install(apWifiNode, staWifiNode);
    // // Install the AMQ
    // QueueDiscContainer queueDiscs = tch.Install(devices_gateway);
    // Install the AMQ
    QueueDiscContainer queueDiscs = tch.Install(apDevice);


    NS_LOG_INFO ("assign the ip.");
    Ipv4AddressHelper address;
    address.SetBase("0.0.0.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces [tcpConnectNum];
    Ipv4InterfaceContainer interfaces_sink;
    Ipv4InterfaceContainer interfaces_gateway;
    Ipv4InterfaceContainer udpinterfaces [udpConnectNum];

    for (uint8_t i = 0; i < tcpConnectNum; i++)
    {
      address.NewNetwork ();
      interfaces [i] = address.Assign (devices [i]);
    }

    for (uint8_t i = 0; i < udpConnectNum; i++)
    {
      udpdevices [i] = accessLink.Install (udpSender.Get (i), apWifiNode);
      address.NewNetwork ();
      udpinterfaces [i] = address.Assign (udpdevices [i]);
    }

    address.NewNetwork ();
    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign (apDevice);
    Ipv4InterfaceContainer staInterface;
    staInterface = address.Assign (staDevices);

    address.NewNetwork ();
    interfaces_sink = address.Assign (devices_sink);

    NS_LOG_INFO ("Populate Routing Protocols.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    uint16_t port = 50000;
    uint16_t port1 = 50001;
    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
    Address sinkLocalAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
    PacketSinkHelper sinkHelper1 ("ns3::UdpSocketFactory", sinkLocalAddress1);

    AddressValue remoteAddress (InetSocketAddress (interfaces_sink.GetAddress (1), port));
    AddressValue remoteAddress1 (InetSocketAddress (interfaces_sink.GetAddress (1), port1));

    BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
    ftp.SetAttribute ("Remote", remoteAddress);
    ftp.SetAttribute ("SendSize", UintegerValue (1000));

    ApplicationContainer sourceApp = ftp.Install (tcpSender);
    sourceApp.Start (Seconds (0));
    sourceApp.Stop (Seconds (stopTime - 1));

    sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
    ApplicationContainer sinkApp = sinkHelper.Install (sink);
    sinkApp.Start (Seconds (0));
    sinkApp.Stop (Seconds (stopTime));

    for(uint8_t i = 0; i < udpConnectNum; i++)
    {
        OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
        clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));
        clientHelper.SetAttribute ("PacketSize", UintegerValue (1000));

        ApplicationContainer clientApps6;
        clientHelper.SetAttribute ("Remote", remoteAddress1);
        clientApps6.Add (clientHelper.Install (udpSender.Get (i)));
        clientApps6.Start (Seconds (0));
        clientApps6.Stop (Seconds (stopTime - 1));
    }

    sinkHelper1.SetAttribute ("Protocol", TypeIdValue (UdpSocketFactory::GetTypeId ()));
    ApplicationContainer sinkApp1 = sinkHelper1.Install (sink);
    sinkApp1.Start (Seconds (0));
    sinkApp1.Stop (Seconds (stopTime));

    Ptr<QueueDisc> queue = queueDiscs.Get (0);
    Simulator::ScheduleNow (&CheckQueueSize, queue,queue_disc_type);
    
    std::string dirToSave = "mkdir -p " + dir + queue_disc_type;
    if (system ((dirToSave + "/cwndTraces/").c_str ()) == -1
        || system ((dirToSave + "/queueTraces/").c_str ()) == -1)
        {
        exit (1);
        }

    Simulator::Schedule (Seconds (0.1), &TraceCwnd,queue_disc_type);

    Simulator::Stop (Seconds (stopTime));
    Simulator::Run ();
    Simulator::Destroy ();

}

int main (int argc, char **argv)
{

  std::cout << "Simulation with COBALT QueueDisc: Start\n";
  experiment ("CobaltQueueDisc");
  std::cout << "Simulation with COBALT QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with CODEL QueueDisc: Start\n";
  experiment ("CoDelQueueDisc");
  std::cout << "Simulation with CODEL QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with FIFO QueueDisc: Start\n";
  experiment ("FifoQueueDisc");
  std::cout << "Simulation with FIFO QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with PIE QueueDisc: Start\n";
  experiment ("PieQueueDisc");
  std::cout << "Simulation with PIE QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with RED QueueDisc: Start\n";
  experiment ("RedQueueDisc");
  std::cout << "Simulation with RED QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with TBF QueueDisc: Start\n";
  experiment ("TbfQueueDisc");
  std::cout << "Simulation with TBF QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with FQCODEL QueueDisc: Start\n";
  experiment ("FqCoDelQueueDisc");
  std::cout << "Simulation with FQCODEL QueueDisc: End\n";
  std::cout << "------------------------------------------------\n";
  std::cout << "Simulation with PFIFO QueueDisc: Start\n";
  experiment ("PfifoFastQueueDisc");
  std::cout << "Simulation with PFIFO QueueDisc: End\n";

  return 0;
}
