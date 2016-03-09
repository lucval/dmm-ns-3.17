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
#ifndef NETFILTER_CONNTRACK_L4_PROTOCOL
#define NETFILTER_CONNTRACK_L4_PROTOCOL

#include "sgi-hashmap.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/ref-count-base.h"
#include "ns3/ptr.h"
#include "netfilter-conntrack-tuple.h"
#include "ipv4-netfilter-hook.h"
#include "ip-conntrack-info.h"

namespace ns3 {

class Packet;

class NetfilterConntrackL4Protocol : public RefCountBase
{
public:
  typedef Callback<uint32_t, Ptr<Packet>, uint32_t, IpConntrackInfo, uint8_t, Hooks_t> PacketVerdict_t;

  PacketVerdict_t PacketVerdict;

  /**
    * \param packet Packet that should be converted to a tuple
    * \param tuple The created tuple is stored here
    * \returns true if success, false otherwise
    *
    * Protocol specific method to convert a packet into a tuple
    * for connection tracking purposes.
    */
  virtual bool PacketToTuple (Ptr<Packet>, NetfilterConntrackTuple&)
  {
    return false;
  }

  /**
    * \param inverse The resulting inverse of the provided tuple
    * \param orig The original tuple that should be inverted
    * \returns true if success, false otherwise
    *
    * Protocol specific method to invert the passed tuple
    */
  virtual bool InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig)
  {
    return false;
  }

  /**
    * \returns Layer 4 protocol this helpers belongs to e.g., IPPROTO_TCP
    *
    * Returns the Layer 4 protocol number that this helper belongs to
    */
  uint16_t GetL4Protocol ()
  {
    return m_l4Protocol;
  }

  /**
    * \param protocol The layer 4 protocol
    *
    * Set the layer 4 protocol that this helper belongs to.
    */
  void SetL4Protocol (uint8_t protocol)
  {
    m_l4Protocol = protocol;
  }

private:
  uint16_t m_l3Protocol;
  uint8_t m_l4Protocol;
  std::string m_protocolName;
};
}

#endif /* NETFILTER_CONNTRACK_L4_PROTOCOL */
