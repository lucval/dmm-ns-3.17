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
#include "icmpv4.h"
#include "icmpv4-conntrack-l4-protocol.h"

NS_LOG_COMPONENT_DEFINE ("Icmpv4ConntrackL4Protocol");

namespace ns3 {

#define IPPROTO_ICMP 1

Icmpv4ConntrackL4Protocol::Icmpv4ConntrackL4Protocol ()
{
  SetL4Protocol (IPPROTO_ICMP);
}

bool 
Icmpv4ConntrackL4Protocol::PacketToTuple (Ptr<Packet> p, NetfilterConntrackTuple& tuple)
{
  NS_LOG_FUNCTION ( this << p );
  Icmpv4Header icmpHeader;
  bool found = p->PeekHeader (icmpHeader);

  if (!found)
    NS_LOG_DEBUG (":: Errrr, No ICMP Header :: ");

  //TODO: Add ICMP specific fields to the NetfilterConntrackTuple class, such as request ID

  return true;
}

bool 
Icmpv4ConntrackL4Protocol::InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig)
{
  return true;
}

}

