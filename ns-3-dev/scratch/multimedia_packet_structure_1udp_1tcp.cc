#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <ctime>
#include <ostream>
#include <map>

#include "ns3/stats-module.h"
#include "examples/stats/wifi-example-apps.h"

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
void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t);
void WriteUntilBufferFull (Ptr<Socket>, uint32_t);
void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port, uint16_t mdata);
void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev);
void dstSocketRecv (Ptr<Socket> socket);

std::string dir = "Multimedia_packet/";
int segment_size = 1446;
int packet_sent = 0;
int packet_received = 0;
int16_t serial_no = 0;
std::vector< int > TTFB_arr;
std::vector< int > TTFB_ts;
std::map<uint16_t, uint16_t> send_map;
std::map<uint16_t, uint16_t> send_serialno_map;
std::map<uint16_t, uint16_t> receive_map;
std::map<uint16_t, uint16_t>::iterator iter;
// tcp
static const uint32_t totalTxBytes = 20000;
static uint32_t currentTxBytes = 0;
static const uint32_t writeSize = 1040;
uint8_t data[writeSize];
//----------------------------------------------------------------------
//-- TimestampTag
//------------------------------------------------------
TypeId
TimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("TimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<TimestampTag> ()
    .AddAttribute ("Timestamp",
                   "Some momentous point in time!",
                   EmptyAttributeValue (),
                   MakeTimeAccessor (&TimestampTag::GetTimestamp),
                   MakeTimeChecker ())
  ;
  return tid;
}
TypeId
TimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
TimestampTag::GetSerializedSize (void) const
{
  return 8;
}
void
TimestampTag::Serialize (TagBuffer i) const
{
  int64_t t = m_timestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&t, 8);
}
void
TimestampTag::Deserialize (TagBuffer i)
{
  int64_t t;
  i.Read ((uint8_t *)&t, 8);
  m_timestamp = NanoSeconds (t);
}
void
TimestampTag::SetTimestamp (Time time)
{
  m_timestamp = time;
}
Time
TimestampTag::GetTimestamp (void) const
{
  return m_timestamp;
}
void
TimestampTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp;
}

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
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd / float(segment_size) << std::endl;
}

static void TraceCwnd (std::string queue_disc_type)
{
  
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (dir + queue_disc_type + "/cwndTraces/S1-1.plotme");
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndTrace,stream));
}

void experiment(std::string queue_disc_type)
{ 
  int segment_size = 1446;
  float stoptime = 27.0;
  int simulation_start_ts = 0;
  std::string bottleneckBandwidth = "10Mbps";
  std::string bottleneckDelay = "1ms";
  std::string accessBandwidth = "100Mbps";
  std::string accessDelay = "1ms";

  Ptr<Node> nSrc = CreateObject<Node> ();
  Ptr<Node> nSrc_tcp = CreateObject<Node> ();
  Ptr<Node> nDst = CreateObject<Node> ();
  Ptr<Node> gw0 = CreateObject<Node> ();
  Ptr<Node> gw1 = CreateObject<Node> ();

  NodeContainer c = NodeContainer (nSrc, nSrc_tcp, gw0, gw1, nDst);

  std::string queue_disc = std::string ("ns3::") + queue_disc_type;
  Config::SetDefault (queue_disc + "::MaxSize", QueueSizeValue (QueueSize ("200p")));
  
  NS_LOG_INFO ("tcp parameter.");
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::DelAckTimeout", TimeValue (Seconds (0)));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocketBase::LimitedTransmit", BooleanValue (false));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (segment_size));
  Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (true));

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));
  TrafficControlHelper tch;
  tch.SetRootQueueDisc (queue_disc);

  InternetStackHelper internet;
  internet.Install (c);

  // Point-to-point links
  NodeContainer nSrcGw0 = NodeContainer (nSrc, gw0);
  NodeContainer nSrcGw0_tcp = NodeContainer (nSrc_tcp, gw0);
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
  NetDeviceContainer dSrcGw0_tcp = accessLink.Install (nSrcGw0_tcp);
  NetDeviceContainer dGw1Dst = accessLink.Install (nGw1Dst);
  NetDeviceContainer dGw0Gw1 = bottleneckLink.Install (nGw0Gw1);

  tchPfifo.Install(dSrcGw0);
  tchPfifo.Install(dSrcGw0_tcp);
  QueueDiscContainer queueDiscs = tch.Install(dGw0Gw1);
  tchPfifo.Install(dGw1Dst);

  // Later, we add IP addresses.
  Ipv4AddressHelper address;
  address.SetBase("10.10.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_src = address.Assign(dSrcGw0);
  address.SetBase("10.15.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_src_tcp  = address.Assign(dSrcGw0_tcp);
  address.SetBase("10.20.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_gateway = address.Assign (dGw0Gw1);
  address.SetBase("10.30.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_dst = address.Assign (dGw1Dst);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();



  Ptr<Socket> srcSocket = Socket::CreateSocket (nSrc, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  uint16_t srcport = 12344;
  Ipv4Address srcaddr ("10.10.1.2");
  InetSocketAddress src = InetSocketAddress (srcaddr, srcport);
  srcSocket->Bind (src);

  Ptr<Socket> srcSocket_tcp = Socket::CreateSocket (nSrc_tcp, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  uint16_t srcport_tcp = 12345;
  Ipv4Address srcaddr_tcp ("10.15.1.2");
  InetSocketAddress src_tcp = InetSocketAddress (srcaddr_tcp, srcport_tcp);
  srcSocket_tcp->Bind (src_tcp);

  Ptr<Socket> dstSocket = Socket::CreateSocket (nDst, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  uint16_t dstport = 12346;
  Ipv4Address dstaddr ("10.30.1.2");
  InetSocketAddress dst = InetSocketAddress (dstaddr, dstport);
  dstSocket->Bind (dst);
  dstSocket->SetRecvCallback (MakeCallback (&dstSocketRecv));

  uint16_t dstport_tcp = 12347;
  Ipv4Address dstaddr_tcp ("10.30.1.2");
  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dstport_tcp));
  ApplicationContainer apps = sink.Install (nDst);
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (10.0));
  
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
  int udp_limit = 65000;
  for (float time = 0.0 ; time<stoptime; time+=0.03, idx++){
      if (v[idx] > udp_limit){
        int current_size = v[idx];
        do{
          Simulator::Schedule (Seconds (time), &SendStuff, srcSocket, dstaddr, dstport, udp_limit);
          current_size -= udp_limit;
        }while(current_size > udp_limit);
        Simulator::Schedule (Seconds (time), &SendStuff, srcSocket, dstaddr, dstport, current_size);

      }else {
        Simulator::Schedule (Seconds (time),&SendStuff, srcSocket, dstaddr, dstport, v[idx]);
      }
  }

  // for the tcp flow 
  // // PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(interfaces_src_tcp.GetAddress(0), dstport));
  // PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dstport));
  
  // AddressValue remoteAddr_tcp (InetSocketAddress(interfaces_src_tcp.GetAddress(1), dstport));
  // BulkSendHelper ftp ("ns3::TcpSocketFactory", Address());
  // ftp.SetAttribute ("Remote", remoteAddr_tcp);
  // ftp.SetAttribute ("SendSize", UintegerValue(1000));

  // ApplicationContainer src_tcp_App = ftp.Install(nSrc_tcp);
  // src_tcp_App.Start(Seconds(0));
  // src_tcp_App.Stop(Seconds(stoptime-1));

  // sinkHelper.SetAttribute("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId()));
  // ApplicationContainer sinkApp = sinkHelper.Install(nDst);
  // src_tcp_App.Start(Seconds(0));
  // src_tcp_App.Stop(Seconds(stoptime));
  
  
  for (float time = 0.0 ; time<stoptime; time+=.27){ 
    // for the .27 here, I look at the pcap of (3-2) and know the last packet time 
    // if I shorten the value, it pop up the error
    Simulator::Schedule (Seconds (time),&StartFlow, srcSocket_tcp, dstaddr_tcp, dstport_tcp);
  }

  Ptr<QueueDisc> queue = queueDiscs.Get (0);
  Simulator::ScheduleNow (&CheckQueueSize, queue, queue_disc_type);
  
  std::string dirToSave = "mkdir -p " + dir + queue_disc_type;
  if (system ((dirToSave + "/cwndTraces/").c_str ()) == -1
      || system ((dirToSave + "/queueTraces/").c_str ()) == -1)
      {
        exit (1);
      }
  
  Simulator::Schedule (Seconds (0), &TraceCwnd, queue_disc_type);
  Simulator::Stop (Seconds (stoptime));
  simulation_start_ts = Simulator::Now().GetMilliSeconds();
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Loss/Total: "<<packet_sent-packet_received<<"/"<<packet_sent);
  float tmp_sum = 0.0;
  
  std::string str1 = "";
  str1 = queue_disc_type;
  std::ofstream data_trace;
  data_trace.open(str1.append(".csv"));
  for (int i=0; i<TTFB_arr.size(); i++){
    data_trace << ((float)TTFB_ts[i] - (float)simulation_start_ts)/1000<<","<<TTFB_arr[i]<<"\n";
    tmp_sum += TTFB_arr[i];
  } data_trace.close();
  NS_LOG_INFO ("TTBL Latency: "<<(float)tmp_sum/(float)TTFB_arr.size());

  std::string str2 = "";
  str2 = queue_disc_type;
  std::ofstream sender_packet_stream;
  sender_packet_stream.open(str2.append("_sender_packet_stream.csv"));
  for (iter = send_map.begin(); iter != send_map.end(); iter++){
    sender_packet_stream<< send_serialno_map[iter->first] <<","<< iter->second<<"\n";
  }sender_packet_stream.close();

  std::string str3 = "";
  str3 = queue_disc_type;
  std::ofstream receiver_packet_stream;
  receiver_packet_stream.open(str3.append("_receiver_packet_stream.csv"));
  for (iter = receive_map.begin(); iter != receive_map.end(); iter++){
    receiver_packet_stream<< send_serialno_map[iter->first] <<","<< iter->second<<"\n";
  }receiver_packet_stream.close();

  TTFB_arr.clear();
  TTFB_ts.clear();
  send_serialno_map.clear();
  send_map.clear();
  receive_map.clear();
  packet_received = 0;
  packet_sent = 0;
  serial_no = 0;


}

void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port, uint16_t mdata)
{
  
  TimestampTag timestamp;
  timestamp.SetTimestamp (Simulator::Now());
  Ptr<Packet> p = Create<Packet> (mdata);
  p->AddByteTag (timestamp);
  sock->SendTo (p, 0, InetSocketAddress (dstaddr,port));
  send_map[(int16_t)p->GetUid()] = mdata;
  send_serialno_map[(int16_t)p->GetUid()] = serial_no++;
  packet_sent++;
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
  TimestampTag timestamp;
  if (packet->FindFirstMatchingByteTag(timestamp)) {
    TTFB_arr.push_back(Simulator::Now().GetMilliSeconds() - timestamp.GetTimestamp().GetMilliSeconds());
    TTFB_ts.push_back(timestamp.GetTimestamp().GetMilliSeconds());
  }

  receive_map[(int16_t)packet->GetUid()] = send_map[(int16_t)packet->GetUid()];
  packet->RemoveAllPacketTags ();
  packet->RemoveAllByteTags ();
  // std::cout<<"packet UID: " << packet->GetUid ()<<std::endl;

  // NS_LOG_INFO ("packet UID: " << packet->GetUid ());
  // InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
  packet_received++;
  // NS_LOG_INFO ("Destination Received " << packet->GetSize () << " bytes from " << address.GetIpv4 ());
}

void StartFlow (Ptr<Socket> localSocket,
                Ipv4Address servAddress,
                uint16_t servPort)
{
  NS_LOG_INFO ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
  currentTxBytes = 0;
  localSocket->Bind ();
  localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect

  // tell the tcp implementation to call WriteUntilBufferFull again
  // if we blocked and new tx buffer space becomes available
  localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
  WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
  while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0)
    {
      uint32_t left = totalTxBytes - currentTxBytes;
      uint32_t dataOffset = currentTxBytes % writeSize;
      uint32_t toWrite = writeSize - dataOffset;
      toWrite = std::min (toWrite, left);
      toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
      int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
      if(amountSent < 0)
        {
          // we will be called again when new tx space becomes available.
          return;
        }
      currentTxBytes += amountSent;
    }
  localSocket->Close ();
}
int 
main (int argc, char **argv)
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  std::cout << "Simulation with FIFO QueueDisc: Start\n";
  experiment ("FifoQueueDisc");
  std::cout << "Simulation with FIFO QueueDisc: End\n";
  // std::cout << "------------------------------------------------\n";
  // std::cout << "Simulation with COBALT QueueDisc: Start\n";
  // experiment ("CobaltQueueDisc");
  // std::cout << "Simulation with COBALT QueueDisc: End\n";
  // std::cout << "------------------------------------------------\n";
  // std::cout << "Simulation with CODEL QueueDisc: Start\n";
  // experiment ("CoDelQueueDisc");
  // std::cout << "Simulation with CODEL QueueDisc: End\n";
  // std::cout << "------------------------------------------------\n";
  // std::cout << "Simulation with PIE QueueDisc: Start\n";
  // experiment ("PieQueueDisc");
  // std::cout << "Simulation with PIE QueueDisc: End\n";
  // std::cout << "------------------------------------------------\n";
  // std::cout << "Simulation with RED QueueDisc: Start\n";
  // experiment ("RedQueueDisc");
  // std::cout << "Simulation with RED QueueDisc: End\n";
  // std::cout << "------------------------------------------------\n";
  // std::cout << "Simulation with FQCODEL QueueDisc: Start\n";
  // experiment ("FqCoDelQueueDisc");
  // std::cout << "Simulation with FQCODEL QueueDisc: End\n";
  // std::cout << "------------------------------------------------\n";
  // std::cout << "Simulation with PFIFO QueueDisc: Start\n";
  // experiment ("PfifoFastQueueDisc");
  // std::cout << "Simulation with PFIFO QueueDisc: End\n";
  
  system("cd ./Multimedia_packet; source exec_overall.sh");
  return 0;
}