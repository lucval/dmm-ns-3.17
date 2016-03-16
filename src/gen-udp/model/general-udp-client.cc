/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
#include "general-udp-client.h"
#include "seq-ts-header.h"
#include <stdlib.h>
#include <stdio.h>
#include "ns3/random-variable.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GeneralUdpClient");
NS_OBJECT_ENSURE_REGISTERED (GeneralUdpClient);

TypeId
GeneralUdpClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GeneralUdpClient")
    .SetParent<Application> ()
    .AddConstructor<GeneralUdpClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&GeneralUdpClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets", TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&GeneralUdpClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute (
      "RemoteAddress",
      "The destination Address of the outbound packets",
      AddressValue (),
      MakeAddressAccessor (&GeneralUdpClient::m_peerAddress),
      MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&GeneralUdpClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&GeneralUdpClient::m_size),
                   MakeUintegerChecker<uint32_t> (12,1500))
   .AddAttribute ("Type",
		   "Type of the UDP Service (Video=0, Voice=1, Gaming=2).",
		   UintegerValue (0),
		   MakeUintegerAccessor (&GeneralUdpClient::m_type),
		   MakeUintegerChecker<uint32_t> (0,3))
  ;
  return tid;
}

GeneralUdpClient::GeneralUdpClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_type = 0;
  m_sliceCount = 0;
  m_voipActive = true;
  m_talkFrame = 0;
  m_silenceFrame = 0;
}

GeneralUdpClient::~GeneralUdpClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GeneralUdpClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  m_peerAddress = Address(ip);
  m_peerPort = port;
}

void
GeneralUdpClient::SetRemote (Address ip, uint16_t port)
{
  m_peerAddress = ip;
  m_peerPort = port;
}

void
GeneralUdpClient::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
GeneralUdpClient::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_type == 0) //Video
  {
	  int m_nodeId = GetNode ()->GetId ();
	  ClientFile1 << "traces/Video" << "_" << m_nodeId << "_" <<  "_TX_time.csv";
  }
  if (m_type == 1) //Game Uplink
    {
  	  int m_nodeId = GetNode ()->GetId ();
  	  ClientFile1 << "traces/Gaming_UL" << "_" << m_nodeId << "_" <<  "_TX_time.csv";
    }
  if (m_type == 2) //Game Downlink
    {
  	  int m_nodeId = GetNode ()->GetId ();
  	  ClientFile1 << "traces/Gaming_DL" << "_" << m_nodeId << "_" <<  "_TX_time.csv";
    }
  if (m_type == 3) //Game Downlink
      {
    	  int m_nodeId = GetNode ()->GetId ();
    	  ClientFile1 << "traces/Voice" << "_" << m_nodeId << "_" <<  "_TX_time.csv";
      }

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }

    }

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &GeneralUdpClient::Send, this);
}

void
GeneralUdpClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_sendFrameEvent);
}

void
GeneralUdpClient::Send (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (m_sendEvent.IsExpired ());
  SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);

  Ptr<Packet> p = NULL;
  if (m_type == 0)
  {
	  // Asume 64Kbps video stream

	  if (m_sliceCount == 0) // Schedule next frame time
		  m_sendFrameEvent = Simulator::Schedule (MilliSeconds(100), &GeneralUdpClient::Send, this);
	  ParetoVariable pSize (326,1.2,250);
	  m_size = pSize.GetInteger();
	  p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
	  p->AddHeader (seqTs);
	  std::stringstream peerAddressStringStream;
		  if (Ipv4Address::IsMatchingType (m_peerAddress))
		  {
			  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
		  }

		  if ((m_socket->Send (p)) >= 0)
		  {
			  ++m_sent;
			  ++m_sliceCount;
			  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
					  << peerAddressStringStream.str () << " Uid: "
					  << p->GetUid () << " Time: "
					  << (Simulator::Now ()).GetSeconds ());
			  ofstream out ((ClientFile1.str ()).c_str (), ios::app);
			  out << "TraceDelay TX " << m_size << " bytes to "
					  << peerAddressStringStream.str () << " Uid: "
					  << p->GetUid () << " Time: "
					  << (Simulator::Now ()).GetSeconds () << endl;

		  }
	  if (m_sliceCount < 8)
	  {
		  ParetoVariable timeBwSlices (20.3 , 1.2 , 12.5);
		  m_sendEvent = Simulator::Schedule (MicroSeconds(1000*timeBwSlices.GetValue()), &GeneralUdpClient::Send, this);
	  }
	  else m_sliceCount=0;

	  //Assume 256Kbps video stream
//	  if (m_sliceCount == 0) // Schedule next frame time
//		  m_sendFrameEvent = Simulator::Schedule (MilliSeconds(50), &GeneralUdpClient::Send, this);
//	  ParetoVariable pSize (326,1.2,250);
//	  m_size = pSize.GetInteger();
//	  p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
//	  p->AddHeader (seqTs);
//	  std::stringstream peerAddressStringStream;
//		  if (Ipv4Address::IsMatchingType (m_peerAddress))
//		  {
//			  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
//		  }
//
//		  if ((m_socket->Send (p)) >= 0)
//		  {
//			  ++m_sliceCount;
//			  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
//					  << peerAddressStringStream.str () << " Uid: "
//					  << p->GetUid () << " Time: "
//					  << (Simulator::Now ()).GetSeconds ());
//
//		  }
//	  if (m_sliceCount < 16)
//	  {
//		  ParetoVariable timeBwSlices (5.2 , 1.2 , 3.125);
//		  m_sendEvent = Simulator::Schedule (MicroSeconds(1000*timeBwSlices.GetValue()), &GeneralUdpClient::Send, this);
//	  }
//	  else m_sliceCount=0;

  }
  else if (m_type == 1) //Gaming Uplink
  {
	  if (m_sliceCount == 0) //First packet, delay uniformly [0, 40ms]
	  {
		  UniformVariable initialTime (0,40);
		  m_sendEvent = Simulator::Schedule (MilliSeconds((uint32_t)initialTime.GetValue()), &GeneralUdpClient::Send, this);
		  m_sliceCount++;
	  }
	  else
	  {
		  //Largest Extreme Value Distribution

		  UniformVariable y (0,1);
		  m_size = (uint32_t)(45 - 5.7 * log(-log(y.GetValue())));

		  p = Create<Packet> (m_size+2-(8+4)); // 8+4 : the size of the seqTs header, 2 = added UDP header for gaming uplink
	      p->AddHeader (seqTs);
	  std::stringstream peerAddressStringStream;
		  if (Ipv4Address::IsMatchingType (m_peerAddress))
		  {
			  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
		  }

		  if ((m_socket->Send (p)) >= 0)
		  {
			  ++m_sent;
			  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
					  << peerAddressStringStream.str () << " Uid: "
					  << p->GetUid () << " Time: "
					  << (Simulator::Now ()).GetSeconds ());
			  ofstream out ((ClientFile1.str ()).c_str (), ios::app);
			  			  out << "TraceDelay TX " << m_size << " bytes to "
			  					  << peerAddressStringStream.str () << " Seq: " << seqTs.GetSeq() << " Uid: "
			  					  << p->GetUid () << " Time: "
			  					  << (Simulator::Now ()).GetSeconds () << endl;

		  }
		  else
		  {
			  NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
					  << peerAddressStringStream.str ());
		  }
		  m_sendEvent = Simulator::Schedule (MilliSeconds(40), &GeneralUdpClient::Send, this);
	  }
  }
  else if (m_type == 2) // 2 = Gaming Downlink
  {
	  if (m_sliceCount == 0) //First packet, delay uniformly [0, 40ms]
	  	  {
	  		  UniformVariable initialTime (0,40);
	  		  m_sendEvent = Simulator::Schedule (MilliSeconds((uint32_t)initialTime.GetValue()), &GeneralUdpClient::Send, this);
	  		  m_sliceCount++;
	  	  }
	  	  else
	  	  {
	  		  //Largest Extreme Value Distribution

	  		  UniformVariable y (0,1);
	  		  uint32_t interval = (uint32_t)(55 - 6 * log(-log(y.GetValue())));
	  		  m_size = (uint32_t)(120 - 36 * log(-log(y.GetValue())));
	  		  p = Create<Packet> (m_size+2-(8+4)); // 8+4 : the size of the seqTs header, 2 = added UDP header for gaming uplink
	  	      p->AddHeader (seqTs);
	  	  std::stringstream peerAddressStringStream;
	  		  if (Ipv4Address::IsMatchingType (m_peerAddress))
	  		  {
	  			  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
	  		  }

	  		  if ((m_socket->Send (p)) >= 0)
	  		  {
	  			  ++m_sent;
	  			  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
	  					  << peerAddressStringStream.str () << " Uid: "
	  					  << p->GetUid () << " Time: "
	  					  << (Simulator::Now ()).GetSeconds ());
	  			ofstream out ((ClientFile1.str ()).c_str (), ios::app);
	  						  out << "TraceDelay TX " << m_size << " bytes to "
	  								  << peerAddressStringStream.str () << " Uid: "
	  								  << p->GetUid () << " Time: "
	  								  << (Simulator::Now ()).GetSeconds () << endl;
	  		  }
	  		  else
	  		  {
	  			  NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
	  					  << peerAddressStringStream.str ());
	  		  }
	  		  m_sendEvent = Simulator::Schedule (MilliSeconds(interval), &GeneralUdpClient::Send, this);
	  	  }
  }
  else if (m_type == 3) // 3 = VOIP
  {
	  if (m_voipActive) // VOIP active and talk frame initialized
	  {
		  //Generate talk spurt frames, Exponentially distributed, mean = 100 frames
		  UniformVariable z (0,1);
		  if (m_voipActive && (m_talkFrame == 0)) // VOIP active, but talk frame is not initialized
		  {
			  while (m_talkFrame == 0)
				  m_talkFrame = (-log(z.GetValue())/0.01);
			  NS_LOG_INFO("m_talkFrame=" << m_talkFrame);
		  }
		  m_size = 38; //VOIP packet
		  p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
		  p->AddHeader (seqTs);
		  std::stringstream peerAddressStringStream;
		  if (Ipv4Address::IsMatchingType (m_peerAddress))
		  {
			  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
		  }

		  if ((m_socket->Send (p)) >= 0)
		  {
			  ++m_sent;
			  ++m_sliceCount;
			  m_sendEvent = Simulator::Schedule (MilliSeconds(20), &GeneralUdpClient::Send, this);
			  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
					  << peerAddressStringStream.str () << " Uid: "
					  << p->GetUid () << " Time: "
					  << (Simulator::Now ()).GetSeconds ());
			  ofstream out ((ClientFile1.str ()).c_str (), ios::app);
			  			  out << "TraceDelay TX " << m_size << " bytes to "
			  					  << peerAddressStringStream.str () << " Uid: "
			  					  << p->GetUid () << " Time: "
			  					  << (Simulator::Now ()).GetSeconds () << endl;
		  }
		  else
		  {
			  NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
					  << peerAddressStringStream.str ());
		  }
		  if (m_sliceCount >= m_talkFrame)
		  {
			  m_sliceCount = 0;
			  m_talkFrame = 0;
			  m_voipActive = false;
		  }
	  }
	  else //Silence duration
	  {
		  //Generate talk spurt frames, Exponentially distributed, mean = 100 frames
		  UniformVariable z (0,1);
		  if (!m_voipActive && (m_silenceFrame == 0)) // VOIP active, but talk frame is not initialized
		  {
			  while (m_silenceFrame == 0)
				  m_silenceFrame = (-log(z.GetValue())/0.01);
			  NS_LOG_INFO("m_silenceFrame=" << m_silenceFrame);
		  }

		  m_size = 14; //SID packet
		  p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
		  p->AddHeader (seqTs);
		  std::stringstream peerAddressStringStream;
		  if (Ipv4Address::IsMatchingType (m_peerAddress))
		  {
			  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
		  }

		  if ((m_socket->Send (p)) >= 0)
		  {
			  ++m_sent;
			  m_sliceCount+=8;
			  m_sendEvent = Simulator::Schedule (MilliSeconds(160), &GeneralUdpClient::Send, this);
			  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
					  << peerAddressStringStream.str () << " Uid: "
					  << p->GetUid () << " Time: "
					  << (Simulator::Now ()).GetSeconds ());
			  ofstream out ((ClientFile1.str ()).c_str (), ios::app);
			  			  out << "TraceDelay TX " << m_size << " bytes to "
			  					  << peerAddressStringStream.str () << " Uid: "
			  					  << p->GetUid () << " Time: "
			  					  << (Simulator::Now ()).GetSeconds () << endl ;

		  }
		  else
		  {
			  NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
					  << peerAddressStringStream.str ());
		  }
		  if (m_sliceCount >= m_silenceFrame)
		  {
			  m_sliceCount = 0;
			  m_silenceFrame = 0;
			  m_voipActive = true;
		  }
	  }
  }
  else //normal UDP
  {
	  p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
      p->AddHeader (seqTs);
  std::stringstream peerAddressStringStream;
	  if (Ipv4Address::IsMatchingType (m_peerAddress))
	  {
		  peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
	  }

	  if ((m_socket->Send (p)) >= 0)
	  {
		  ++m_sent;
		  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
				  << peerAddressStringStream.str () << " Uid: "
				  << p->GetUid () << " Time: "
				  << (Simulator::Now ()).GetSeconds ());
		  ofstream out ((ClientFile1.str ()).c_str (), ios::app);
		  			  out << "TraceDelay TX " << m_size << " bytes to "
		  					  << peerAddressStringStream.str () << " Uid: "
		  					  << p->GetUid () << " Time: "
		  					  << (Simulator::Now ()).GetSeconds ()<< endl ;

	  }
	  else
	  {
		  NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
				  << peerAddressStringStream.str ());
	  }

	  if (m_sent < m_count)
	  {
		  m_sendEvent = Simulator::Schedule (m_interval, &GeneralUdpClient::Send, this);
	  }
  }
}
} // Namespace ns3
