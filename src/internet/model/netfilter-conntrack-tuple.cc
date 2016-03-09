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

#include "netfilter-conntrack-tuple.h"

NS_LOG_COMPONENT_DEFINE ("ConntrackTupleHash");

namespace ns3 {

NetfilterConntrackTuple::NetfilterConntrackTuple ()
{
  m_l4Source = 0;
  m_l4Destination = 0;
}

NetfilterConntrackTuple::NetfilterConntrackTuple (Ipv4Address src, uint16_t srcPort, Ipv4Address dst, uint16_t dstPort)
{
  m_l3Source = src;
  m_l4Source = srcPort;
  m_l3Destination = dst;
  m_l4Destination = dstPort;
}

bool
NetfilterConntrackTuple::operator== (const NetfilterConntrackTuple t) const
{
  return (m_l3Source == t.m_l3Source)
         && (m_l4Source == t.m_l4Source)
         && (m_l3Destination == t.m_l3Destination)
         && (m_l4Destination == t.m_l4Destination);
}

bool
NetfilterConntrackTuple::SourceEqual (NetfilterConntrackTuple t1, NetfilterConntrackTuple t2)
{
  return (t1.m_l3Source == t2.m_l3Source)
         && (t1.m_l4Source == t2.m_l4Source);
}

bool
NetfilterConntrackTuple::DestinationEqual (NetfilterConntrackTuple t1, NetfilterConntrackTuple t2)
{
  return (t1.m_l3Destination == t2.m_l3Destination)
         && (t1.m_l4Destination == t2.m_l4Destination);
}


Ipv4Address
NetfilterConntrackTuple::GetSource () const
{
  return m_l3Source;
}

void
NetfilterConntrackTuple::SetSource (Ipv4Address source)
{
  m_l3Source = source;
}

void
NetfilterConntrackTuple::SetSourcePort (uint16_t port)
{
  m_l4Source = port;
}

void
NetfilterConntrackTuple::SetDestination (Ipv4Address destination)
{
  m_l3Destination = destination;
}

void
NetfilterConntrackTuple::SetDestinationPort (uint16_t destination)
{
  m_l4Destination = destination;
}

Ipv4Address
NetfilterConntrackTuple::GetDestination () const
{
  return m_l3Destination;
}

uint16_t
NetfilterConntrackTuple::GetSourcePort () const
{
  return m_l4Source;
}

uint16_t
NetfilterConntrackTuple::GetDestinationPort () const
{
  return m_l4Destination;
}

uint16_t
NetfilterConntrackTuple::GetDestinationProtocol () const
{
  return m_protocolNumber;
}

void
NetfilterConntrackTuple::SetProtocol (uint16_t protocol)
{
  m_l3Protocol = protocol;
}

uint16_t NetfilterConntrackTuple::GetProtocol ()
{
  return m_l3Protocol;
}

void
NetfilterConntrackTuple::SetDirection (ConntrackDirection_t direction)
{
  m_direction = (uint8_t)direction;
}

uint8_t
NetfilterConntrackTuple::GetDirection () const
{
  return m_direction;
}

char*
NetfilterConntrackTuple::ToString () const
{
  return (char *)this;
}

NetfilterConntrackTuple
NetfilterConntrackTuple::Invert ()
{
  NetfilterConntrackTuple inverse (GetDestination (), GetDestinationPort (), GetSource (), GetSourcePort ());
  inverse.SetDirection (this->GetDirection () == IP_CT_DIR_ORIGINAL ? IP_CT_DIR_REPLY : IP_CT_DIR_ORIGINAL);
  return inverse;
}

void
NetfilterConntrackTuple::Print (std::ostream &os) const
{
  os << "( " << GetSource () << "," << GetSourcePort () << "," << GetDestination () << "," << GetDestinationPort () << (int)GetDirection () << ")";
}

NetfilterConntrackTuple&
NetfilterConntrackTuple::operator= (const NetfilterConntrackTuple&  tuple)
{
  if (this != &tuple)
    {
      m_l3Source = tuple.m_l3Source;
      m_l4Source = tuple.m_l4Source;
      m_l3Destination = tuple.m_l3Destination;
      m_l4Destination = tuple.m_l4Destination;

      m_l3Protocol = tuple.m_l3Protocol;
      m_protocolNumber = tuple.m_protocolNumber;
      m_direction = tuple.m_direction;
    }

  return *this;
}

std::ostream& operator << (std::ostream& os, NetfilterConntrackTuple const& tuple)
{
  os << "( " << tuple.GetSource () << "," << tuple.GetSourcePort () << "," << tuple.GetDestination () << "," << tuple.GetDestinationPort () << ", " << (int)tuple.GetDirection () << ")";
  return os;
}

#define JHASH_GOLDEN_RATIO  0x9e3779b9

void
JHashMix (uint32_t a, uint32_t b, uint32_t c)
{
  a -= b;
  a -= c;
  a ^= (c >> 13);
  b -= c;
  b -= a;
  b ^= (a << 8);
  c -= a;
  c -= b;
  c ^= (b >> 13);
  a -= b;
  a -= c;
  a ^= (c >> 12);
  b -= c;
  b -= a;
  b ^= (a << 16);
  c -= a;
  c -= b;
  c ^= (b >> 5);
  a -= b;
  a -= c;
  a ^= (c >> 3);
  b -= c;
  b -= a;
  b ^= (a << 10);
  c -= a;
  c -= b;
  c ^= (b >> 15);
}

uint32_t
JHash2 (const uint32_t *k, uint32_t length, uint32_t initval)
{
  uint32_t a, b, c, len;

  a = b = JHASH_GOLDEN_RATIO;
  c = initval;
  len = length;

  while (len >= 3)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      JHashMix (a, b, c);
      k += 3;
      len -= 3;
    }

  c += length * 4;

  switch (len)
    {
    case 2:
      b += k[1];
    case 1:
      a += k[0];
    }

  JHashMix (a,b,c);
  return c;

}

size_t
ConntrackTupleHash::operator() (const NetfilterConntrackTuple &x) const
{
  uint32_t n;
  uint32_t h;
  uint16_t rnd = 2;
  uint32_t size = 12;

  n = (sizeof (x.GetSource ()) + sizeof (x.GetSourcePort ()) +
       sizeof (x.GetDestination ()) + sizeof (x.GetDestinationPort ())) / sizeof (uint32_t);

  h = JHash2 ((uint32_t*)(x.ToString ()), n, rnd ^ (((x.GetDestinationPort ()) << 16) | x.GetDestinationProtocol ()));

  NS_LOG_DEBUG ("Hashing ==> Tuple " <<  "( " << x.GetSource () << "," << x.GetSourcePort () << "," << x.GetDestination () << "," << x.GetDestinationPort () << "," << (int)x.GetDirection () << ")" << " Hash: " << (((uint64_t)h * size) >> 32));

  return ((uint64_t)h * size) >> 32;
}

}
