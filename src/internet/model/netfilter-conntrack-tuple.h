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
#ifndef NETFILTER_CONNTRACK_TUPLE_H
#define NETFILTER_CONNTRACK_TUPLE_H

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ref-count-base.h"
#include "ns3/object.h"
#include "ip-conntrack-info.h"
//#include "jhash.h"


namespace ns3 {

class Object;
class Ipv4Address;

class NetfilterConntrackTuple : public RefCountBase
{
public:
  NetfilterConntrackTuple ();
  NetfilterConntrackTuple (Ipv4Address src, uint16_t srcPort, Ipv4Address dst, uint16_t dstPort);
  bool operator== (const NetfilterConntrackTuple t) const;
  bool SourceEqual (NetfilterConntrackTuple t1, NetfilterConntrackTuple t2);
  bool DestinationEqual (NetfilterConntrackTuple t1, NetfilterConntrackTuple t2);
  NetfilterConntrackTuple Invert ();

  Ipv4Address GetSource () const;
  Ipv4Address GetDestination () const;
  uint16_t GetSourcePort () const;
  uint16_t GetDestinationPort () const;
  char * ToString () const;
  uint16_t GetDestinationProtocol () const;
  uint8_t GetDirection () const;
  uint16_t GetProtocol ();

  void SetSource (Ipv4Address source);
  void SetSourcePort (uint16_t source);
  void SetDestination (Ipv4Address destination);
  void SetDestinationPort (uint16_t destination);
  void SetProtocol (uint16_t protocol);
  void SetDirection (ConntrackDirection_t direction);

  void Print (std::ostream &os) const;

  NetfilterConntrackTuple& operator= (const NetfilterConntrackTuple&  tuple);
  friend std::ostream& operator << (std::ostream& os, NetfilterConntrackTuple const& tuple);

private:
  Ipv4Address m_l3Source;
  uint16_t m_l3Protocol;
  uint16_t m_l4Source;

  Ipv4Address m_l3Destination;
  uint16_t m_l4Destination;
  uint8_t m_protocolNumber;
  uint8_t m_direction;
};


class ConntrackTupleHash : public std::unary_function<NetfilterConntrackTuple, size_t>
{
public:
  size_t operator() (const NetfilterConntrackTuple &x) const;
};

}
#endif /* NETFILTER_CONNTRACK_TUPLE_H */
