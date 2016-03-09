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

#include "netfilter-callback-chain.h"
#include "ipv4-netfilter-hook.h"

namespace ns3 {

NetfilterCallbackChain::NetfilterCallbackChain ()
{
}

void
NetfilterCallbackChain::Insert (const Ipv4NetfilterHook& hook)
{
  if (m_netfilterHooks.empty () )
    {
      m_netfilterHooks.push_front (hook);
    }
  else
    {
      std::list<Ipv4NetfilterHook>::iterator it = m_netfilterHooks.begin ();
      for (; it != m_netfilterHooks.end (); it++)
        {
          if (hook.GetPriority () < it->GetPriority ())
            {
              m_netfilterHooks.insert (it,hook);
              return;
            }
        }
      m_netfilterHooks.push_back (hook);
    }
}

std::list<Ipv4NetfilterHook>::iterator
NetfilterCallbackChain::Find (const Ipv4NetfilterHook& hook)
{
  std::list<Ipv4NetfilterHook>::iterator it = m_netfilterHooks.begin ();
  std::list<Ipv4NetfilterHook>::iterator it2;

  for (; it != m_netfilterHooks.end (); it++)
    {
      if (*it == hook)
        {
          return it;
        }
    }
  return it2;
}

void
NetfilterCallbackChain::Remove (const Ipv4NetfilterHook& hook)
{
  m_netfilterHooks.remove (hook);
}

Ipv4NetfilterHook
NetfilterCallbackChain::Front ()
{
  return m_netfilterHooks.front ();
}

uint32_t
NetfilterCallbackChain::Size () const
{
  return m_netfilterHooks.size ();
}

bool
NetfilterCallbackChain::IsEmpty () const
{
  if (m_netfilterHooks.empty ())
    {
      return true;
    }
  else
    {
      return false;
    }
}

void
NetfilterCallbackChain::Clear ()
{
  m_netfilterHooks.clear ();
}

int32_t
NetfilterCallbackChain::IterateAndCallHook (Hooks_t hookNumber, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback ccb)
{
  std::list<Ipv4NetfilterHook>::iterator it = m_netfilterHooks.begin ();

  for (; it != m_netfilterHooks.end (); it++)
    {
      it->HookCallback (hookNumber, p, in, out, ccb);
    }

  return NF_ACCEPT; // TODO: Check
}

} // namespace ns3

