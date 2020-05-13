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

// Network topology
//
//       n0    n1
//       |     |
//       =======
//         LAN
//
// - UDP flows from n0 to n1

#include <fstream>
#include <chrono>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include <bits/stdc++.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpClientServerExample");

int
main (int argc, char *argv[])
{
//
// Enable logging for UdpClient and
//
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

  bool useV6 = false;
  Address serverAddress;

  CommandLine cmd;
  cmd.AddValue ("useIpv6", "Use Ipv6", useV6);
  cmd.Parse (argc, argv);

//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);

  NS_LOG_INFO ("Create channels.");
//
// Explicitly create the channels required by the topology (shown above).
//
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));
  NetDeviceContainer d = csma.Install (n);

//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  if (useV6 == false)
    {
      Ipv4AddressHelper ipv4;
      ipv4.SetBase ("10.1.1.0", "255.255.255.0");
      Ipv4InterfaceContainer i = ipv4.Assign (d);
      serverAddress = Address (i.GetAddress (1));
    }
  else
    {
      Ipv6AddressHelper ipv6;
      ipv6.SetBase ("2001:0000:f00d:cafe::", Ipv6Prefix (64));
      Ipv6InterfaceContainer i6 = ipv6.Assign (d);
      serverAddress = Address(i6.GetAddress (1,1));
    }

  
  NS_LOG_INFO ("Create Applications.");
//
// Create one udpServer applications on node one.
//
  uint16_t port = 4000;
  UdpServerHelper server (port);
  ApplicationContainer apps_iframe = server.Install (n.Get (1));
  ApplicationContainer apps_pframe = server.Install (n.Get (1));
  apps_iframe.Start (Seconds (1.0));
  apps_iframe.Stop (Seconds (5.0));
  apps_pframe.Start (Seconds (1.0));
  apps_pframe.Stop (Seconds (5.0));

//
// Create one UdpClient application to send UDP datagrams from node zero to
// node one.
//
  // uint32_t MaxPacketSize = 1024;
  uint32_t MaxPacketCount_iframe = 1;
  uint32_t MaxPacketCount_pframe = 1;
  uint32_t iframe = 10000;
  uint32_t pframe = 1000;

  Time interPacketInterval = MilliSeconds(30);

  UdpClientHelper client_iframe (serverAddress, port);
  client_iframe.SetAttribute ("MaxPackets", UintegerValue (MaxPacketCount_iframe));
  client_iframe.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client_iframe.SetAttribute ("PacketSize", UintegerValue (iframe));

  UdpClientHelper client_pframe (serverAddress, port);
  client_pframe.SetAttribute ("MaxPackets", UintegerValue (MaxPacketCount_pframe));
  client_pframe.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client_pframe.SetAttribute ("PacketSize", UintegerValue (pframe));
  
  apps_iframe = client_iframe.Install (n.Get (0));
  apps_pframe = client_pframe.Install (n.Get (0));

  int idx = 0;
  int pre_iflag = 0;
  int pre_pflag = 0;

  for (float time = 0.0; time<1.0; time+=0.03, idx++){

    if(idx%30 == 0){
      apps_iframe.Start(Seconds (time));
      if(pre_pflag == 1) {
        apps_pframe.Stop(Seconds (time));
        pre_pflag = 0;
      }
      pre_iflag = 1;
    }
    
    else{
      if(pre_pflag == 1) {
        apps_pframe.Stop(Seconds (time));
      }else apps_iframe.Stop(Seconds (time));
      
      apps_pframe.Start(Seconds (time));
      pre_pflag = 1;
      
    }
    
  }

//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
