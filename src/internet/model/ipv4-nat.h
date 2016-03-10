/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Sindhuja Venkatesh
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
 * Authors: Sindhuja Venkatesh <intutivestriker88@gmail.com>
 */
#ifndef IPV4_NAT_H
#define IPV4_NAT_H

#include <stdint.h>
#include <limits.h>
#include <sys/socket.h>
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/object.h"
#include "ipv4-netfilter.h"
#include "ipv4-netfilter-hook.h"
#include "netfilter-callback-chain.h"

#include "netfilter-tuple-hash.h"
#include "netfilter-conntrack-tuple.h"
#include "netfilter-conntrack-l3-protocol.h"
#include "netfilter-conntrack-l4-protocol.h"
#include "ip-conntrack-info.h"
#include "ipv4.h"


namespace ns3 {

class Packet;
class NetDevice;
class OutputStreamWrapper;

/**
  * \brief Implementation of the Static NAT Rule.
  *
  * This implements the basic static NAT rule structure with some
  * methods to access their attributes.
  */

class Ipv4StaticNatRule
{
public:
/**
  *\brief Initialization of the static NAT rule with specific protcol port restricitions.
  *\param localip The ip address of the host on the local network
  *\param locprt The source port number of the host on the local network
  *\param globalip The translated ip address on the global network
  *\param gloprt The translated source port number of the host on the global network
  *\param protocol The protocol used in the connection
  */
  Ipv4StaticNatRule (Ipv4Address localip, uint16_t locprt, Ipv4Address globalip,uint16_t gloprt, uint16_t protocol);

/**
  *\brief This version is used for no port restrictions
  *\param localip The ip address of the host on the local network
  *\param globalip The translated ip address on the global network
  */
  Ipv4StaticNatRule (Ipv4Address localip, Ipv4Address globalip);

/**
  *\return The local host Ipv4Address
  */
  Ipv4Address GetLocalIp () const;

/**
  *\return The translated global Ipv4Address
  */
  Ipv4Address GetGlobalIp () const;

/**
  *\return The source port number of the host on the local network
  */
  uint16_t GetLocalPort () const;

/**
  *\return The translated source port number of the host on the global network
  */
  uint16_t GetGlobalPort () const;

/**
  *\return The Protocol the rule is specific to.
  */
  uint16_t GetProtocol () const;


private:
  Ipv4Address m_localaddr;
  Ipv4Address m_globaladdr;
  uint16_t m_localport;
  uint16_t m_globalport;
  uint16_t m_protocol;

  // private data member
};

/**
  * \brief Implementation of the Dynamic NAT Rule.
  *
  * This implements the basic Dynamic NAT rule structure with some
  * methods to access their attributes.
  */

class Ipv4DynamicNatRule
{
public:
/**
  *\brief Used to initialize the network on the inside that is going to be translated
  *\param localnet The local network ip to be translated
  *\param localmask The mask of the local network
  */

  Ipv4DynamicNatRule (Ipv4Address localnet, Ipv4Mask localmask);

/**
  *\return The local network address
  */
  Ipv4Address GetLocalNet () const;

/**
  *\return The local network mask
  */
  Ipv4Mask GetLocalMask () const;

private:
  Ipv4Address m_localnetwork;
  Ipv4Mask m_localmask;
  // private data members
};

/**
  * \brief Implementation of the Dynamic NAT Tuple.
  *
  * This implements the basic Dynamic NAT tuple entry structure with some
  * methods to access their attributes.
  */

class Ipv4DynamicNatTuple
{
public:
/**
  *\brief Used to initialize the Dynamic NAT translated tuple entry.
  *\param local The local host ip that is translated
  *\param global The global ip that the host has been translated to
  *\param port The source port that the local host has translated to
  */
  Ipv4DynamicNatTuple (Ipv4Address local, Ipv4Address global, uint16_t port);

/**
  *\return The local host Ipv4Address
  */
  Ipv4Address GetLocalAddress () const;

/**
  *\return The translated global Ipv4Address
  */
  Ipv4Address GetGlobalAddress () const;

/**
  *\return The translated source port number of the host on the global network
  */
  uint16_t GetTranslatedPort () const;

private:
  Ipv4Address m_localip;
  Ipv4Address m_globalip;
  uint16_t m_port;
};

/**
  * \brief Implementation of Nat
  *
  * This implements NAT functionality over a Netfilter framework.
  * The NAT is of two major types (static and dynamic).
  */

class Ipv4Nat : public Object
{
public:
  static TypeId GetTypeId (void);

  Ipv4Nat ();

  /**
   * \brief Add rules to the Dynamic NAT Table.
   *
   * \param rule Dynamic NAT rule reference reference to the NAT rule to be added
   *
   * Adds a NAT rule to the lists that have been dedicated for the specific types
   * of rules.
   */

  void AddDynamicRule (const Ipv4DynamicNatRule& rule);

  /**
   * \brief Add rules to the Static NAT Table.
   *
   * \param rule Static NAT rule reference reference to the NAT rule to be added
   *
   * Adds a Static NAT rule to the lists that have been dedicated for the specific types
   * of rules.
   */

  void AddStaticRule (const Ipv4StaticNatRule& rule);

  /**
   * \return number of Static NAT rules
   *
   * Returns the number of rules that are currently listed on the list.
   */
  uint32_t GetNStaticRules (void) const;

  /**
   * \return number of Dynamic NAT rules
   *
   * Returns the number of rules that are currently listed on the list.
   */
  uint32_t GetNDynamicRules (void) const;

  /**
   * \return number of Dynamic NAT Tuples
   *
   * Returns the number of tuples that are currently listed on the list.
   */
  uint32_t GetNDynamicTuples (void) const;

  /**
   * \param index index in table specifying rule to return
   * \return rule at specified index
   *
   * Returns the specific Static NAT rule that is stored on the given index.
   */
  Ipv4StaticNatRule GetStaticRule (uint32_t index) const;

  /**
   * \param index index in table specifying rule to return
   * \return rule at specified index
   *
   * Returns the specific Dynamic NAT rule that is stored on the given index.
   */
  Ipv4DynamicNatRule GetDynamicRule (uint32_t index) const;

  /**
   * \param index index in table specifying rule to return
   * \return rule at specified index
   *
   * Returns the specific Dynamic NAT tuple that is stored on the given index.
   */
  Ipv4DynamicNatTuple GetDynamicTuple (uint32_t index) const;


  /**
   * \param index index in table specifying rule to remove
   *
   * Removes the Static NAT rule that is stored on the given index.
   */
  void RemoveStaticRule (uint32_t index);

  /**
   * \param index index in table specifying rule to remove
   *
   * Removes the Dynamic NAT rule that is stored on the given index.
   */
  void RemoveDynamicRule (uint32_t index);

  /**
   * \brief Print the NAT translation table
   *
   * \param stream the ostream the NAT table is printed to
   *
   * Prints out the NAT table.
   */
  void PrintTable (Ptr<OutputStreamWrapper> stream) const;

  /**
   * \brief Add the address pool for Dynamic NAT
   *
   * \param Ipv4address the addresses to be added in the Dynamic Nat pool
   * \param Ipv4Mask the mask of the pool of network address given
   */
  void AddAddressPool (Ipv4Address, Ipv4Mask);

  /**
   * \brief Add the port pool for Dynamic NAT
   *
   * \param numbers for the port pool
   * \param port
   */
  void AddPortPool (uint16_t, uint16_t); //port range

  /**
   * \brief Set the inside interface for the node
   *
   * \param interfaceIndex interface index number of the interface on the node
   */
  void SetInside (int32_t interfaceIndex);

  /**
   * \brief Set the outside interfaces for the node
   *
   * \param interfaceIndex interface index number of the interface on the node
   */
  void SetOutside (int32_t interfaceIndex);

  void ReceivePacketIn(Ptr<Socket> sock);
  void ReceivePacketEg(Ptr<Socket> sock);

  typedef std::list<Ipv4StaticNatRule> StaticNatRules;
  typedef std::list<Ipv4DynamicNatRule> DynamicNatRules;
  typedef std::list<Ipv4DynamicNatTuple> DynamicNatTuple;


protected:
  // from Object base class
  virtual void NotifyNewAggregate (void);

private:
  //bool m_isConnected;

  Ptr<Ipv4> m_ipv4;

  /**
    * \param hook The hook number e.g., NF_INET_PRE_ROUTING
    * \param p Packet that is handed over to the callback chain for this hook
    * \param in NetDevice which received the packet
    * \param out The outgoing NetDevice
    * \param ccb If not NULL, this callback will be invoked once the hook
    * callback chain has finished processing
    *
    * \returns Netfilter verdict for the Packet. e.g., NF_ACCEPT, NF_DROP etc.
    *
    *  This method is invoke to perform NAT of the packet at the NF_INET_PRE_ROUTING stage.
    */

  uint32_t DoNatPreRouting (Hooks_t hookNumber, Ptr<Packet> p,
                            Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb);

  /**
     * \param hook The hook number e.g., NF_INET_PRE_ROUTING
     * \param p Packet that is handed over to the callback chain for this hook
     * \param in NetDevice which received the packet
     * \param out The outgoing NetDevice
     * \param ccb If not NULL, this callback will be invoked once the hook
     * callback chain has finished processing
     *
     * \returns Netfilter verdict for the Packet. e.g., NF_ACCEPT, NF_DROP etc.
     *
     *  This method is invoke to perform NAT of the packet at the NF_INET_POST_ROUTING stage.
     */

  uint32_t DoNatPostRouting (Hooks_t hookNumber, Ptr<Packet> p,
                             Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb);
  /**
  *\return The Global Pool Ip address
  */
  Ipv4Address GetAddressPoolIp () const;

  /**
  *\return The Global Pool NetMask
  */
  Ipv4Mask GetAddressPoolMask () const;

  /**
  *\return The start port of the port pool
  */
  uint16_t GetStartPort () const;

  /**
  *\return The end port of the port pool
  */
  uint16_t GetEndPort () const;

  /**
  *\return The current port value for the dynamic nat translation
  */
  uint16_t GetCurrentPort () const;

  /**
  *\return The a new port to translate to from the port pool
  */
  uint16_t GetNewOutsidePort ();

  StaticNatRules m_statictable;
  DynamicNatRules m_dynamictable;
  DynamicNatTuple m_dynatuple;
  std::vector<int32_t> m_insideInterfaces;
  std::vector<int32_t> m_outsideInterfaces;
  Ipv4Address m_globalip;
  Ipv4Mask m_globalmask;
  uint16_t m_startport;
  uint16_t m_endport;
  uint16_t m_currentPort;

  std::map<Ipv4Address, Time> m_ts;

};

}
#endif

