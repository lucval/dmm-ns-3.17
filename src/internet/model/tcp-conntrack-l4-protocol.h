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
#ifndef TCP_CONNTRACK_L4_PROTOCOL
#define TCP_CONNTRACK_L4_PROTOCOL

#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "netfilter-conntrack-l4-protocol.h"

namespace ns3 {

//#define IPPROTO_TCP 6

class Packet;
class NetDevice;

  class TcpConntrackL4Protocol : public NetfilterConntrackL4Protocol {
    public:
      TcpConntrackL4Protocol ();
      bool PacketToTuple (Ptr<Packet> p, NetfilterConntrackTuple& tuple);
      bool InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig);

    private:
  };
}

#endif /* TCP_CONNTRACK_L4_PROTOCOL */
