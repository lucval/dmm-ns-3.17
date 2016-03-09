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
#include "ns3/ipv4-header.h"
#include "ns3/callback.h"
//#include "ns3/conntrack-tag.h"
#include "ipv4-conntrack-l3-protocol.h"

NS_LOG_COMPONENT_DEFINE ("Ipv4ConntrackL3Protocol");

namespace ns3 {

Ipv4ConntrackL3Protocol::Ipv4ConntrackL3Protocol ()
{
}

uint32_t 
Ipv4ConntrackL3Protocol::Ipv4Confirm (Hooks_t hookNumber, Ptr<Packet> packet, Ptr<NetDevice> in,
                      Ptr<NetDevice> out, ContinueCallback& ccb)
{
  NS_LOG_DEBUG (":: Executing hook function Ipv4Confirm ::");
  /*ConntrackTag ctinfo;
  bool tagFound = packet->PeekPacketTag (ctinfo);

  if (!tagFound || ctinfo.GetConntrack () == IP_CT_RELATED + IP_CT_IS_REPLY)
  {
    NS_LOG_DEBUG ("Conntrack tag not found");
    return 0;
  }*/

  // Call conntrack helper here

  NS_ASSERT (!ccb.IsNull ());
  // NetfilterConntrackConfirm
  NS_LOG_DEBUG ("Invoking the ContinueCallback");
  ccb (packet);

  return 0;
}

uint32_t 
Ipv4ConntrackL3Protocol::Ipv4ConntrackPreRoutingHook (Hooks_t hookNumber, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  return 0;
}

uint32_t 
Ipv4ConntrackL3Protocol::Ipv4ConntrackInHook (Hooks_t hookNumber, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  return 0;
}

uint32_t 
Ipv4ConntrackL3Protocol::Ipv4ConntrackOutHook (Hooks_t hookNumber, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  return 0;
}

uint32_t 
Ipv4ConntrackL3Protocol::Ipv4ConntrackPostRoutingHook (Hooks_t hookNumber, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  return 0;
}

uint16_t 
Ipv4ConntrackL3Protocol::RegisterPreRoutingHook ()
{
  return 0;
}

uint16_t 
Ipv4ConntrackL3Protocol::RegisterInHook ()
{
  return 0;
}

uint16_t 
Ipv4ConntrackL3Protocol::RegisterOutHook ()
{
  return 0;
}

uint16_t 
Ipv4ConntrackL3Protocol::RegisterPostRoutingHook ()
{
  return 0;
}

bool 
Ipv4ConntrackL3Protocol::PacketToTuple (Ptr<Packet> packet, NetfilterConntrackTuple& tuple)
{
  Ipv4Header ipHeader;

  packet->PeekHeader (ipHeader);

  tuple.SetSource (ipHeader.GetSource ());
  tuple.SetDestination (ipHeader.GetDestination ());

  NS_LOG_DEBUG ("Ipv4 Packet To Tuple: " << "( " << tuple.GetSource () << "," << tuple.GetSourcePort () << "," << tuple.GetDestination () << "," << tuple.GetDestinationPort () << ")" );

  return true;
}

bool 
Ipv4ConntrackL3Protocol::InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig)
{
  inverse.SetSource (orig.GetDestination () );
  inverse.SetDestination (orig.GetSource () );
  return true;
}



}
