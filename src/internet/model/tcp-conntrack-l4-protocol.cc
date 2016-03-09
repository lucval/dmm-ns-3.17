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
#include "ns3/log.h"
#include "tcp-header.h"
#include "tcp-conntrack-l4-protocol.h"

NS_LOG_COMPONENT_DEFINE ("TcpConntrackL4Protocol");

namespace ns3 {

#define IPPROTO_TCP 6

TcpConntrackL4Protocol::TcpConntrackL4Protocol ()
{
  SetL4Protocol (IPPROTO_TCP);
}

bool 
TcpConntrackL4Protocol::PacketToTuple (Ptr<Packet> p, NetfilterConntrackTuple& tuple)
{
  TcpHeader tcpHeader;
  p->PeekHeader (tcpHeader);

  tuple.SetSourcePort (tcpHeader.GetSourcePort ());
  tuple.SetDestinationPort (tcpHeader.GetDestinationPort ());
  
  NS_LOG_DEBUG ("TCP Packet To Tuple: " << "( " << tuple.GetSource () << "," << tuple.GetSourcePort () << "," << tuple.GetDestination () << "," << tuple.GetDestinationPort () << ")" );
  return true;
}

bool 
TcpConntrackL4Protocol::InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig)
{
  inverse.SetSourcePort (orig.GetDestinationPort () );
  inverse.SetDestinationPort (orig.GetSourcePort () );
  return true;
}

}

