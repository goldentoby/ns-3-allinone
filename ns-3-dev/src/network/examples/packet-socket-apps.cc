/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Universita' di Firenze
 *
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
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

// Network topology
//
//       n0              n1
//       |               |
//       =================
//         SimpleChannel
//
// - Packets flows from n0 to n1
//
// This example shows how to use the PacketSocketServer and PacketSocketClient
// to send non-IP packets over a SimpleNetDevice

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

using namespace ns3;

/**
 * \ingroup network
 * A simple example of an Header implementation
 */
class MyHeader : public Header 
{
  public:

    MyHeader ();
    virtual ~MyHeader ();

    /**
     * Set the header data.
     * \param data The data.
     */
    void SetData (uint16_t data);
    /**
     * Get the header data.
     * \return The data.
     */
    uint16_t GetData (void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize (void) const;
  private:
    uint16_t m_data;  //!< Header data
};

MyHeader::MyHeader ()
{
  // we must provide a public default constructor, 
  // implicit or explicit, but never private.
}
MyHeader::~MyHeader ()
{
}

TypeId
MyHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyHeader")
    .SetParent<Header> ()
    .AddConstructor<MyHeader> ()
  ;
  return tid;
}
TypeId
MyHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
MyHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.
  //os << "data=" << m_data << std::endl;
  os << "data=" << m_data;
}
uint32_t
MyHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return 2;
}
void
MyHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteHtonU16 (m_data);
}
uint32_t
MyHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  m_data = start.ReadNtohU16 ();

  // we return the number of bytes effectively read.
  return 2;
}

void 
MyHeader::SetData (uint16_t data)
{
  m_data = data;
}
uint16_t 
MyHeader::GetData (void) const
{
  return m_data;
}

int main (int argc, char *argv[])
{
  bool verbose = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.Parse(argc, argv);
  LogComponentEnable ("PacketSocketServer", LOG_LEVEL_ALL);
  // LogComponentEnable ("PacketSocketClient", LOG_LEVEL_ALL);

  if (verbose)
    {
      LogComponentEnable ("PacketSocketServer", LOG_LEVEL_ALL);
      LogComponentEnable ("PacketSocketClient", LOG_LEVEL_ALL);
      LogComponentEnable ("SimpleNetDevice", LOG_LEVEL_ALL);
    }

  NodeContainer nodes;
  nodes.Create (2);

  ns3::PacketMetadata::Enable();

  PacketSocketHelper packetSocket;

  // give packet socket powers to nodes.
  packetSocket.Install (nodes);

  Ptr<SimpleNetDevice> txDev;
  txDev = CreateObject<SimpleNetDevice> ();
  nodes.Get (0)->AddDevice (txDev);

  Ptr<SimpleNetDevice> rxDev;
  rxDev = CreateObject<SimpleNetDevice> ();
  nodes.Get (1)->AddDevice (rxDev);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel> ();
  txDev->SetChannel (channel);
  rxDev->SetChannel (channel);
  txDev->SetNode (nodes.Get (0));
  rxDev->SetNode (nodes.Get (1));


  PacketSocketAddress socketAddr;
  socketAddr.SetSingleDevice (txDev->GetIfIndex ());
  socketAddr.SetPhysicalAddress (rxDev->GetAddress ());
  // Arbitrary protocol type.
  // Note: PacketSocket doesn't have any L4 multiplexing or demultiplexing
  //       The only mux/demux is based on the protocol field
  socketAddr.SetProtocol (1);

  Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient> ();
  client->SetRemote (socketAddr);
  Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer> ();
  server->SetLocal (socketAddr);

  // Packet::EnablePrinting ();
  // MyHeader sourceHeader;
  // sourceHeader.SetData (10000);
  // Ptr<Packet> p = Create<Packet> ();
  // p->AddHeader (sourceHeader);
  // p->Print (std::cout);
  // std::cout << std::endl;

  // nodes.Get (0)->AddApplication (client);
  // nodes.Get (1)->AddApplication (server);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
