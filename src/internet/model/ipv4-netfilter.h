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

#ifndef IPV4_NETFILTER_H
#define IPV4_NETFILTER_H

#include <stdint.h>
#include <limits.h>
#include <sys/socket.h>
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
//#include "ns3/conntrack-tag.h"
#include "ns3/ipv4-header.h"
#include "ns3/object.h"

#include "ipv4-netfilter-hook.h"
#include "netfilter-callback-chain.h"

#include "netfilter-tuple-hash.h"
#include "netfilter-conntrack-tuple.h"
#include "netfilter-conntrack-l3-protocol.h"
#include "netfilter-conntrack-l4-protocol.h"
#include "ip-conntrack-info.h"
//#include "network-address-translation.h"

#define CTINFO2DIR(ctinfo) ((ctinfo) >= IP_CT_IS_REPLY ? IP_CT_DIR_REPLY : IP_CT_DIR_ORIGINAL)

namespace ns3 {

class Packet;
class NetDevice;

typedef enum
{
  NF_IP_PRI_FIRST = INT_MIN,
  NF_IP_PRI_CONNTRACK_DEFRAG = -400,
  NF_IP_PRI_RAW = -300,
  NF_IP_PRI_SELINUX_FIRST = -225,
  NF_IP_PRI_CONNTRACK = -200,
  NF_IP_PRI_MANGLE = -150,
  NF_IP_PRI_NAT_DST = -100,
  NF_IP_PRI_FILTER = 0,
  NF_IP_PRI_SECURITY = 50,
  NF_IP_PRI_NAT_SRC = 100,
  NF_IP_PRI_SELINUX_LAST = 225,
  NF_IP_PRI_CONNTRACK_CONFIRM = INT_MAX,
  NF_IP_PRI_LAST = INT_MAX,
} NetfilterIpv4HookPriorities;


static Callback<uint32_t, Ptr<Packet> > defaultContinueCallback = MakeNullCallback<uint32_t, Ptr<Packet> > ();

/**
  * \brief Implementation of netfilter
  *
  * This implements functionality similar to netfilter
  * in the Linux Kernel. As of now, it supports limited
  * connection tracking (without expectations), and Network
  * Address Translation.
  */

class Ipv4Netfilter  : public Object
{
public:
  static TypeId GetTypeId (void);

  Ipv4Netfilter ();

  /**
    * \param hook The hook function to be registered
    *
    * Registers the hook function at the specified hook
    * using the priority given in the hook datastructure.
    * The hook function is added to the callback chain for
    * that hook and is called whenever a packet traverses
    * that hook.
    */
  void RegisterHook (const Ipv4NetfilterHook& hook);

  /**
    * \param hook The hook function to be registered
    *
    * Unregisters the hook function from the specified hook
    * The hook function is removed from the callback chain for
    * that hook.
    */
  void DeregisterHook (const Ipv4NetfilterHook& hook);

  /**
    * \param protocolFamily The protocol family e.g., PF_INET
    * \param hook The hook number e.g., NF_INET_PRE_ROUTING
    * \param p Packet that is handed over to the callback chain for this hook
#if 0
    * \param in NetDevice which received the packet
    * \param out The outgoing NetDevice
    * \param ccb If not NULL, this callback will be invoked once the hook
    * callback chain has finished processing
#endif
    * \returns Netfilter verdict for the Packet. e.g., NF_ACCEPT, NF_DROP etc.
    *
    * Various invocations of this method are used to implement hooks within the
    * ns-3 IP stack. When a packet "traverses" a hook, it is handed over to the
    * callback chain for that hook by this method.
    */
  uint32_t ProcessHook (uint8_t protocolFamily, Hooks_t hookNumber, Ptr<Packet> p,Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback cc = MakeNullCallback <uint32_t, Ptr<Packet> > ());       //ContinueCallback ccb = defaultContinueCallback);


  //Adding void methods for Hooking on specific nodes - sender,forwarder and receiver
  // uint32_t HookRegistered(Hooks_t hook, Ptr<Packet> packet, Ptr<NetDevice> in,
  //          Ptr<NetDevice> out, ContinueCallback& ccb);

  uint32_t HookPri1 (Hooks_t hook, Ptr<Packet> packet, Ptr<NetDevice> in,
                     Ptr<NetDevice> out, ContinueCallback& ccb);

  uint32_t HookPri2 (Hooks_t hook, Ptr<Packet> packet, Ptr<NetDevice> in,
                     Ptr<NetDevice> out, ContinueCallback& ccb);

  uint32_t HookPri3 (Hooks_t hook, Ptr<Packet> packet, Ptr<NetDevice> in,
                     Ptr<NetDevice> out, ContinueCallback& ccb);

  /**
    * \param l3Protocol Layer 3 protocol
    * \returns 0 on success
    *
    * Registers a layer 3 protocol with the netfilter framework. Packets
    * that conform to the registered protocols can be processed by netfilter.
    */
  uint32_t RegisterL3Protocol (Ptr<NetfilterConntrackL3Protocol> l3Protocol);

  /**
    * \param l4Protocol Layer 4 protocol
    * \returns 0 on success
    *
    * Registers a layer 4 protocol with the netfilter framework. Packets
    * that conform to the registered protocols can be processed by netfilter.
    */
  uint32_t RegisterL4Protocol (Ptr<NetfilterConntrackL4Protocol> l4Protocol);

  /**
    * \param protocolFamily Protocol Family e.g., PF_INET
    * \returns Pointer to the helper object
    *
    * Searches for a matching Layer 3 protocol helper among the ones that
    * have been registered using RegisterL3Protocol.
    */
  Ptr<NetfilterConntrackL3Protocol> FindL3ProtocolHelper (uint8_t protocolFamily);

  /**
    * \param protocol Layer 4 protocol e.g., IPPROTO_TCP
    * \returns Pointer to the helper object
    *
    * Searches for a matching Layer 4 protocol helper among the ones that
    * have been registered using RegisterL4Protocol.
    */
  Ptr<NetfilterConntrackL4Protocol> FindL4ProtocolHelper (uint8_t protocol);

  /**
    * \param packet The packet being processed by a hook
    * \param protocolFamily Protocol Family e.g., PF_INET
    * \param protocol The value in the protocol field of the IP header
    * \param l3Protocol Layer 3 protocol helper
    * \param l4Protocol Layer 4 protocol helper
    * \param setReply Set to 1 if this is a reply
    * \param ctInfo Connection tracking information e.g., IP_CT_ESTABLISHED
    * \param ipHeader IP header of the packet
    * \returns 0 on success
    *
    * This method checks whether this is a new connection and if so creates an
    * entry for it in the hash table. If this connection already exists in the
    * hash table then the state of the connection is updated depending on the
    * information inside the packet
    */
  uint32_t ResolveNormalConntrack (Ptr<Packet> packet, uint32_t protocolFamily, uint8_t protocol,
                                   Ptr<NetfilterConntrackL3Protocol> l3Protocol, Ptr<NetfilterConntrackL4Protocol> l4Protocol,
                                   int& setReply, ConntrackInfo_t& ctInfo, Ipv4Header ipHeader);

  /**
    * \param packet Packet that should be converted to a tuple
    * \param l3Number Layer 3 protocol
    * \param protocolNumber Layer 4 protocol
    * \param tuple Stores the created tuple
    * \param Layer 3 protocol helper
    * \param Layer 4 protocol helper
    * \returns true if a tuple was created successfully, false otherwise
    *
    * Extracts information from the packet to create a corresponding tuple
    */

  bool NetfilterConntrackGetTuple (Ptr<Packet> packet, uint16_t l3Number, uint8_t protocolNumber,
                                   NetfilterConntrackTuple& tuple, Ptr<NetfilterConntrackL3Protocol> l3Protocol,
                                   Ptr<NetfilterConntrackL4Protocol> l4Protocol);

  /**
    * \param tuple Tuple representing the new connection
    * \param l3proto Layer 3 protocol helper
    * \param l4proto Layer 4 protocol helper
    * \param packet Packet
    * \returns TupleHash Iterator
    *
    * Updates the hash table to create an entry for the new connection
    */

  TupleHashI NewConnection (NetfilterConntrackTuple& tuple, Ptr<NetfilterConntrackL3Protocol> l3proto,
                            Ptr<NetfilterConntrackL4Protocol> l4proto, Ptr<Packet> packet);


  int UpdateConntrackInfo (uint8_t info);

  uint32_t NetfilterConntrackIn (Hooks_t hook, Ptr <Packet> packet, Ptr<NetDevice> in,
                                 Ptr<NetDevice> out, ContinueCallback& ccb);

  uint32_t NetfilterConntrackConfirm (Ptr<Packet> p);



  /**
    * \param inverse The inverse of the tuple should be stored here
    * \param orig The tuple that should be inverted
    * \param l3Protocol Layer 3 protocol helper
    * \param l4Protocol Layer 4 protocol helper
    *
    * Inverses the passed tuple. This is needed to track reply packets.
    */

  bool InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig,
                    Ptr<NetfilterConntrackL3Protocol> l3Protocol,
                    Ptr<NetfilterConntrackL4Protocol> l4Protocol);

  TupleHash& GetHash ();

#ifdef NOTYET
  void AddNatRule (NatRule natRule);

  uint32_t NetfilterDoNat (Hooks_t hookNumber, Ptr<Packet> p,
                           Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb);


  std::vector<NatRule>::iterator FindNatRule (NatRule natRule);

  std::vector<NatRule>::iterator FindNatRule (Ipv4Address orig, Ptr<NetDevice> out);
  //static NetfilterConntrackTuple currentTuple[IP_CT_DIR_MAX];

  void EnableNat ();

  uint32_t NetfilterNatPacket (Hooks_t hookNumber, Ptr<Packet> p);

#endif 

private:
  NetfilterCallbackChain m_netfilterHooks[NF_INET_NUMHOOKS];
  //std::vector<Ptr<NetfilterConntrackL3Protocol> > m_netfilterConntrackL3Protocols;
  TupleHash m_netfilterTupleHash[IP_CT_DIR_MAX];
  TupleHash m_unconfirmed;
  TupleHash m_hash;

  /* TODO: Should be a table once we have more L3/L4 Protocols */
  Ptr<NetfilterConntrackL3Protocol> m_netfilterConntrackL3Protocols;
  std::vector<Ptr<NetfilterConntrackL4Protocol> > m_netfilterConntrackL4Protocols;

  //TranslationMap m_natMappings;

  NetfilterConntrackTuple currentOriginalTuple;
  NetfilterConntrackTuple currentReplyTuple;

/*
  uint8_t m_enableNat;
  std::vector <NatRule> m_natRules;


  uint16_t nextAvailablePort;

  TranslationMap m_natReplyLookup;
*/
};

//uint16_t Ipv4Netfilter::nextAvailablePort = 1024;

//NetfilterConntrackTuple Ipv4Netfilter::currentTuple = NetfilterConntrackTuple ();
//NetfilterConntrackTuple Ipv4Netfilter::currentTuple[IP_CT_DIR_ORIGINAL] = NetfilterConntrackTuple ();

} // Namespace ns3
#endif /* IPV4_NETFILTER_H */
