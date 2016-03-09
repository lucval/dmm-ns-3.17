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

#ifndef NETFILTER_CALLBACK_CHAIN_H
#define NETFILTER_CALLBACK_CHAIN_H

#include <list>
#include "ipv4-netfilter-hook.h"

namespace ns3 {

/**
 * \brief container class for holding netfilter callbacks
 *
 * This class manages a list of callbacks for the netfilter system.
 * The callback objects are copied upon insertion into a list.
 * The IP netfilter code can call IterateAndCallHook () to traverse the
 * callback chain.
 */
class NetfilterCallbackChain
{
public:
  NetfilterCallbackChain ();
  void Insert (const Ipv4NetfilterHook& hook);
  std::list<Ipv4NetfilterHook>::iterator Find (const Ipv4NetfilterHook& hook);
  void Remove (const Ipv4NetfilterHook& hook);
  Ipv4NetfilterHook Front ();
  uint32_t Size () const;
  bool IsEmpty () const;
  void Clear ();
  int32_t IterateAndCallHook (Hooks_t, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback ccb);

private:
  std::list<Ipv4NetfilterHook> m_netfilterHooks;
};

} // namespace ns3

#endif /* NETFILTER_CALLBACK_CHAIN */
