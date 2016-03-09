/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "packet-loss-counter.h"

#include "seq-ts-header.h"
#include "general-udp-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GeneralUdpServer");
NS_OBJECT_ENSURE_REGISTERED (GeneralUdpServer);


TypeId
GeneralUdpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GeneralUdpServer")
    .SetParent<Application> ()
    .AddConstructor<GeneralUdpServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&GeneralUdpServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&GeneralUdpServer::GetPacketWindowSize,
                                         &GeneralUdpServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
   .AddAttribute ("Type",
		   "Type of the UDP Service (Video=0, GamingUL=1, GamingDL=2, Voice = 3).",
		   UintegerValue (0),
		   MakeUintegerAccessor (&GeneralUdpServer::m_type),
		   MakeUintegerChecker<uint32_t> (0,3))
  ;
  return tid;
}

GeneralUdpServer::GeneralUdpServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received=0;
}

GeneralUdpServer::~GeneralUdpServer ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t
GeneralUdpServer::GetPacketWindowSize () const
{
  return m_lossCounter.GetBitMapSize ();
}

void
GeneralUdpServer::SetPacketWindowSize (uint16_t size)
{
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
GeneralUdpServer::GetLost (void) const
{
  return m_lossCounter.GetLost ();
}

uint32_t
GeneralUdpServer::GetReceived (void) const
{

  return m_received;

}

void
GeneralUdpServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
GeneralUdpServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_type == 0) //Video
  {
	  int m_nodeId = GetNode ()->GetId ();
	  ClientFile1 << "Video" << "_" << m_nodeId << "_" <<  "_RX_time.csv";
  }
  if (m_type == 1) //Game Uplink
    {
  	  int m_nodeId = GetNode ()->GetId ();
  	  ClientFile1 << "Gaming_UL" << "_" << m_nodeId << "_" <<  "_RX_time.csv";
    }
  if (m_type == 2) //Game Downlink
    {
  	  int m_nodeId = GetNode ()->GetId ();
  	  ClientFile1 << "Gaming_DL" << "_" << m_nodeId << "_" <<  "_RX_time.csv";
    }
  if (m_type == 3) //Game Downlink
      {
    	  int m_nodeId = GetNode ()->GetId ();
    	  ClientFile1 << "Voice" << "_" << m_nodeId << "_" <<  "_RX_time.csv";
      }

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      m_socket->Bind (local);
    }

  m_socket->SetRecvCallback (MakeCallback (&GeneralUdpServer::HandleRead, this));


}

void
GeneralUdpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
GeneralUdpServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();
          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("RX " << packet->GetSize () <<
                           " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now ().GetMilliSeconds() <<
                           " Delay " << Simulator::Now ().GetMilliSeconds() - seqTs.GetTs ());
              ofstream out ((ClientFile1.str ()).c_str (), ios::app);
              out << "TraceDelay: RX " << packet->GetSize () <<
                      " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                      " Sequence Number: " << currentSequenceNumber <<
                      " Uid: " << packet->GetUid () <<
                      " TXtime: " << seqTs.GetTs ().GetMilliSeconds() <<
                      " RXtime: " << Simulator::Now ().GetMilliSeconds() <<
                      " Delay " << Simulator::Now ().GetMilliSeconds() - seqTs.GetTs ().GetMilliSeconds() << endl;
            }
          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;
        }
    }
}

} // Namespace ns3
