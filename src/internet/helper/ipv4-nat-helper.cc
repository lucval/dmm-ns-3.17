/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/ipv4-nat.h"
#include "ns3/ipv4-nat-helper.h"

NS_LOG_COMPONENT_DEFINE ("Ipv4NatHelper");

namespace ns3 {

Ipv4NatHelper::Ipv4NatHelper ()
{
}

Ipv4NatHelper::Ipv4NatHelper (const Ipv4NatHelper &o)
{
}

Ptr<Ipv4Nat>
Ipv4NatHelper::Install (Ptr<Node> node) const
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4, "No IPv4 object found");
  Ptr<Ipv4Nat> nat = CreateObject<Ipv4Nat> ();
  node->AggregateObject (nat);
  return nat;
}


} // namespace ns3
