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
#ifndef NETFILTER_CONNTRACK_L3_PROTOCOL
#define NETFILTER_CONNTRACK_L3_PROTOCOL

#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/ref-count-base.h"
#include "ns3/ptr.h"
#include "sgi-hashmap.h"
#include "netfilter-conntrack-tuple.h"

namespace ns3 {

class Packet;

/**
  * \brief Base class for Netfilter Layer 3 m_protocol helper
  *
  * Every Layer 3 m_protocol helper should inherit from this class
  * and implement methods PacketToTuple and InvertTuple
  */

class NetfilterConntrackL3Protocol : public RefCountBase
{
public:
  /**
    * \param packet Packet that should be converted to a tuple
    * \param tuple The created tuple is stored here
    * \returns true if success, false otherwise
    *
    * Protocol specific method to convert a packet into a tuple
    * for connection tracking purposes.
    */
  virtual bool PacketToTuple (Ptr<Packet> packet, NetfilterConntrackTuple& tuple)
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

private:
  std::string m_m_protocolName;
  uint16_t m_protocol;
};

}

#endif /* NETFILTER_CONNTRACK_L3_PROTOCOL */
