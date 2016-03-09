/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * Copyright (c) 2009 University of Texas at Dallas
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
 * Author: Qasim Javed <qasim@utdallas.edu>
 */
#include "udp-header.h"
#include "udp-conntrack-l4-protocol.h"

NS_LOG_COMPONENT_DEFINE ("UdpConntrackL4Protocol");

namespace ns3 {

#define IPPROTO_UDP 17

UdpConntrackL4Protocol::UdpConntrackL4Protocol ()
{
  SetL4Protocol (IPPROTO_UDP);
}

bool 
UdpConntrackL4Protocol::PacketToTuple (Ptr<Packet> p, NetfilterConntrackTuple& tuple)
{
  NS_LOG_FUNCTION ( this << p );
  UdpHeader udpHeader;
  bool found = p->PeekHeader (udpHeader);

  if (!found)
    NS_LOG_DEBUG (":: Errrr, No UDP Header :: ");

  tuple.SetSourcePort (udpHeader.GetSourcePort ());
  tuple.SetDestinationPort (udpHeader.GetDestinationPort ());
  
  NS_LOG_DEBUG ("UDP Packet To Tuple: " << "( " << tuple.GetSource () << "," << tuple.GetSourcePort () << "," << tuple.GetDestination () << "," << tuple.GetDestinationPort () << ")" );
  return true;
}

bool 
UdpConntrackL4Protocol::InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig)
{
  inverse.SetSourcePort (orig.GetDestinationPort () );
  inverse.SetDestinationPort (orig.GetSourcePort () );
  return true;
}

}

