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

#ifndef IPV4_NETFILTER_HOOK_H
#define IPV4_NETFILTER_HOOK_H

#include <stdint.h>
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"

namespace ns3 {

class Packet;
class NetDevice;

typedef enum
{
  NF_INET_PRE_ROUTING,
  NF_INET_LOCAL_IN,
  NF_INET_FORWARD,
  NF_INET_LOCAL_OUT,
  NF_INET_POST_ROUTING,
  NF_INET_NUMHOOKS,
} Hooks_t;

typedef enum
{
  NF_DROP,
  NF_ACCEPT,
  NF_STOLEN,
  NF_QUEUE,
  NF_REPEAT,
  NF_STOP,
} Verdicts_t;

typedef Callback<uint32_t, Ptr<Packet> > ContinueCallback;
typedef Callback<uint32_t, Hooks_t, Ptr<Packet>, Ptr<NetDevice>, Ptr<NetDevice>, ContinueCallback&> NetfilterHookCallback;

/**
  * \brief Implementation of the Hook datastructure
  *
  * This contains information such as the hook callback
  * function, the priority of the hook callback, the protocol
  * family this callback caters to and the hook number. The hook
  * number is needed to identify the hook and thus the callback chain
  * where the hook function should be inserted
  */

class Ipv4NetfilterHook
{
public:
  Ipv4NetfilterHook ();
  Ipv4NetfilterHook (uint8_t protocolFamily, uint32_t hookNumber, uint32_t priority, NetfilterHookCallback hook);
  Ipv4NetfilterHook (uint8_t protocolFamily, Hooks_t hookNumber, uint32_t priority, NetfilterHookCallback hook);
  Ipv4NetfilterHook & operator= (const Ipv4NetfilterHook& hook);
  bool operator== (const Ipv4NetfilterHook& hook) const;
  int32_t GetPriority () const;
  int32_t GetHookNumber () const;
  int32_t HookCallback (Hooks_t, Ptr<Packet>, Ptr<NetDevice>, Ptr<NetDevice>, ContinueCallback);
  void Print (std::ostream &os) const;

private:
  NetfilterHookCallback m_hook;
  uint8_t m_protocolFamily;
  uint32_t m_hookNumber;
  int32_t m_priority;
};

} // Namespace ns3
#endif /* IPV4_NETFILTER_HOOK_H */
