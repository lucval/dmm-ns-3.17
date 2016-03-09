/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of Washington
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

#ifndef IPV4_NAT_HELPER_H
#define IPV4_NAT_HELPER_H

#include "ns3/ptr.h"
#include "ns3/ipv4-nat.h"

namespace ns3 {

class Node;

/**
 * \brief Helper class that adds ns3::Ipv4Nat objects
 */
class Ipv4NatHelper
{
public:
  /**
   * \brief Constructor.
   */
  Ipv4NatHelper ();

  /**
   * \brief Construct an Ipv4NatHelper from another previously 
   * initialized instance (Copy Constructor).
   */
  Ipv4NatHelper (const Ipv4NatHelper &);

  /**
   * \param node the node on which the NAT will run
   * \returns a newly-created NAT, aggregated to the node
   *
   * This method installs a NAT object and hooks it to a node.  It
   * assumes that an Internet stack has already been aggregated to the node
   */
  virtual Ptr<Ipv4Nat> Install (Ptr<Node> node) const;

private:
  /**
   * \internal
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  Ipv4NatHelper &operator = (const Ipv4NatHelper &o);
};

} // namespace ns3

#endif /* IPV4_NAT_HELPER_H */

