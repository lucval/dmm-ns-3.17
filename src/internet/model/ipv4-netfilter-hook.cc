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
#include "ipv4-netfilter-hook.h"
#include "ns3/packet.h"

namespace ns3 {

Ipv4NetfilterHook::Ipv4NetfilterHook ()
{
  m_protocolFamily = 0;
  m_hookNumber = 0;
  m_priority = 10;
}

Ipv4NetfilterHook::Ipv4NetfilterHook (uint8_t protocolFamily, uint32_t hookNumber, uint32_t priority, NetfilterHookCallback hook)
{
  m_protocolFamily = protocolFamily;
  m_hookNumber = hookNumber;
  m_priority = priority;
  m_hook = hook;
}

Ipv4NetfilterHook::Ipv4NetfilterHook (uint8_t protocolFamily, Hooks_t hookNumber, uint32_t priority, NetfilterHookCallback hook)
{
  m_protocolFamily = protocolFamily;
  m_hookNumber = (uint32_t)hookNumber;
  m_priority = priority;
  m_hook = hook;
}

bool
Ipv4NetfilterHook::operator== (const Ipv4NetfilterHook& hook) const
{
  return (m_protocolFamily == hook.m_protocolFamily
          && m_hookNumber == hook.m_hookNumber
          && m_priority == hook.m_priority);
  //m_hook == hook.m_hook);
}

Ipv4NetfilterHook&
Ipv4NetfilterHook::operator= (const Ipv4NetfilterHook& hook)
{
  if (this != &hook)
    {
      m_hook = hook.m_hook;
      m_protocolFamily = hook.m_protocolFamily;
      m_hookNumber = hook.m_hookNumber;
      m_priority = hook.m_priority;
    }

  return *this;
}

int32_t
Ipv4NetfilterHook::GetPriority () const
{
  return m_priority;
}

int32_t
Ipv4NetfilterHook::GetHookNumber () const
{
  return m_hookNumber;
}

int32_t
Ipv4NetfilterHook::HookCallback (Hooks_t hookNumber, Ptr<Packet> p, Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback ccb)
{
  if (m_hook.IsNull ())
    {
      std::cout << "********* OOOPS! ***********" << std::endl;
    }
  return m_hook (hookNumber, p, in, out, ccb);
}

void
Ipv4NetfilterHook::Print (std::ostream &os) const
{
  os << "Hook: " << m_hookNumber << ","
     << "Priority: " << m_priority;
}

} // Namespace ns3
