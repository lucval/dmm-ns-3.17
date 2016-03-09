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
#ifndef NETFILTER_CONNTRACK_L3_IPV4
#define NETFILTER_CONNTRACK_L3_IPV4

#include "sgi-hashmap.h"
#include "netfilter-conntrack-tuple.h"
#include "netfilter-conntrack-l3-protocol.h"

namespace ns3 {
class NetfilterConntrackL3Ipv4
{
public:
  NetfilterConntrackL3Ipv4 ();
  ~NetfilterConntrackL3Ipv4 ();
  RegisterNetfilterL3Ipv4 ();
  UnRegisterNetfilterL3Ipv4 ();

private:
  NetfilterConntrackL3Protocol ipv4;
};
}

#endif /* NETFILTER_CONNTRACK_L3_IPV4 */
