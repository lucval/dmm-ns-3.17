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
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ipv4-netfilter.h"

#include "ip-conntrack-info.h"
#include "ipv4-conntrack-l3-protocol.h"
#include "tcp-conntrack-l4-protocol.h"
#include "udp-conntrack-l4-protocol.h"

#include "tcp-header.h"
#include "udp-header.h"
#include "ns3/node.h"
#include "ns3/net-device.h"

NS_LOG_COMPONENT_DEFINE ("Ipv4Netfilter");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Ipv4Netfilter);

TypeId
Ipv4Netfilter::GetTypeId (void)
{
  static TypeId tId = TypeId ("ns3::Ipv4Netfilter")
    .SetParent<Object> ()
#ifdef NOTYET
    .AddAttribute ("EnableNat", "0 disbales NAT and is the default, 1 enabled NAT",
                   UintegerValue (0),
                   MakeUintegerAccessor (&Ipv4Netfilter::m_enableNat),
                   MakeUintegerChecker <uint8_t> ())
#endif
  ;

  return tId;
}

Ipv4Netfilter::Ipv4Netfilter ()
 // : m_enableNat (0)
{
  NS_LOG_FUNCTION_NOARGS ();

  /* Create callback chains for all of the hooks */
  for (int i = 0; i < NF_INET_NUMHOOKS; i++)
    {
      m_netfilterHooks[i] = NetfilterCallbackChain ();
    }

  /* Create and register Ipv4 connection tracking module */
  Ptr<Ipv4ConntrackL3Protocol> ipv4 = Create<Ipv4ConntrackL3Protocol> ();
  this->RegisterL3Protocol (ipv4);

  /* Create and register TCP connection tracking module */
  Ptr<TcpConntrackL4Protocol> tcp = Create<TcpConntrackL4Protocol> ();
  this->RegisterL4Protocol (tcp);

  /* Create and register UDP connection tracking module */
  Ptr<UdpConntrackL4Protocol> udp = Create<UdpConntrackL4Protocol> ();
  this->RegisterL4Protocol (udp);

  //Ptr <NetworkAddressTranslation> networkAddressTranslation = Create<NetworkAddressTranslation> (this);
  // Create and register hook callbacks for conntrack
  NetfilterHookCallback preRouting = MakeCallback (&Ipv4Netfilter::NetfilterConntrackIn, this);
  NetfilterHookCallback localIn = MakeCallback (&Ipv4ConntrackL3Protocol::Ipv4Confirm, PeekPointer (ipv4));

  Ipv4NetfilterHook nfh = Ipv4NetfilterHook (1, NF_INET_PRE_ROUTING, NF_IP_PRI_CONNTRACK, preRouting);
  Ipv4NetfilterHook nfh1 = Ipv4NetfilterHook (1, NF_INET_LOCAL_OUT, NF_IP_PRI_CONNTRACK, preRouting);
  Ipv4NetfilterHook nfh2 = Ipv4NetfilterHook (1, NF_INET_POST_ROUTING, NF_IP_PRI_CONNTRACK_CONFIRM, localIn);
  Ipv4NetfilterHook nfh3 = Ipv4NetfilterHook (1, NF_INET_LOCAL_IN, NF_IP_PRI_CONNTRACK_CONFIRM, localIn);


  this->RegisterHook (nfh);
  this->RegisterHook (nfh1);
  this->RegisterHook (nfh2);
  this->RegisterHook (nfh3);

/*  if (m_enableNat)
    {
      EnableNat ();
    }

  nextAvailablePort = 1024;
*/
  }

void
Ipv4Netfilter::RegisterHook (const Ipv4NetfilterHook& hook)
{
  m_netfilterHooks[hook.GetHookNumber ()].Insert (hook);
}

void
Ipv4Netfilter::DeregisterHook (const Ipv4NetfilterHook& hook)
{
  m_netfilterHooks[hook.GetHookNumber ()].Remove (hook);
}

uint32_t
Ipv4Netfilter::ProcessHook (uint8_t protocolFamily, Hooks_t hookNumber, Ptr<Packet> p,Ptr<NetDevice> in, Ptr<NetDevice> out,ContinueCallback ccb)
{
  return m_netfilterHooks[(uint32_t)hookNumber].IterateAndCallHook (hookNumber, p, in, out, ccb);
  //return 1;
}

uint32_t
Ipv4Netfilter::RegisterL3Protocol (Ptr<NetfilterConntrackL3Protocol> l3Protocol)
{
  //m_netfilterConntrackL3Protocols.push_back(l3Protocol);
  m_netfilterConntrackL3Protocols = l3Protocol;
  return 0;
}

uint32_t
Ipv4Netfilter::RegisterL4Protocol (Ptr<NetfilterConntrackL4Protocol> l4Protocol)
{
  m_netfilterConntrackL4Protocols.push_back (l4Protocol);
  //m_netfilterConntrackL4Protocols = l4Protocol;
  return 0;
}

Ptr<NetfilterConntrackL3Protocol>
Ipv4Netfilter::FindL3ProtocolHelper (uint8_t protocolFamily)
{
  /*Ptr<NetfilterConntrackL3Protocol>::iterator it;

  for (; it != netfilerConntrackL3Protocols.end(); it++)
  {
    if (protocolFamily == it->protocol)
      return *it;
  }*/

  return m_netfilterConntrackL3Protocols;

}

Ptr<NetfilterConntrackL4Protocol>
Ipv4Netfilter::FindL4ProtocolHelper (uint8_t protocol)
{
  std::vector<Ptr<NetfilterConntrackL4Protocol> >::iterator it;

  for (it =  m_netfilterConntrackL4Protocols.begin (); it != m_netfilterConntrackL4Protocols.end (); it++)
    {
      NS_LOG_DEBUG ( "L4 protocol: " << (*it)->GetL4Protocol () << ", This protocol : " << (int)protocol );
      if ((*it)->GetL4Protocol () == protocol)
        {
          NS_LOG_DEBUG ( "Found protocol " << (int)protocol );
          return *it;
        }
    }

  return NULL;
}

int
Ipv4Netfilter::UpdateConntrackInfo (uint8_t info)
{
  m_hash[currentOriginalTuple].SetInfo (info);
  m_hash[currentReplyTuple].SetInfo (info);
  return 0;
}

bool
Ipv4Netfilter::NetfilterConntrackGetTuple (Ptr<Packet> packet, uint16_t l3Number, uint8_t protocolNumber,
                                           NetfilterConntrackTuple& tuple, Ptr<NetfilterConntrackL3Protocol> l3Protocol,
                                           Ptr<NetfilterConntrackL4Protocol> l4Protocol)
{
  tuple.SetProtocol (l3Number);

  if (l3Protocol->PacketToTuple (packet, tuple) == false)
    {
      return false;
    }

  //TODO: Do we really need the Protocol Family as well?
  //tuple->Set
  Ipv4Header ipHeader;
  NS_LOG_DEBUG (" :: Remove Ipv4 Header :: ");
  packet->RemoveHeader (ipHeader);

  tuple.SetDirection (IP_CT_DIR_ORIGINAL);

  if (l4Protocol->PacketToTuple (packet, tuple) == false)
    {
      return false;
    }

  NS_LOG_DEBUG (" :: Add Ipv4 Header :: ");
  packet->AddHeader (ipHeader);

  return true;
}

TupleHashI
Ipv4Netfilter::NewConnection (NetfilterConntrackTuple& tuple, Ptr<NetfilterConntrackL3Protocol> l3proto,
                              Ptr<NetfilterConntrackL4Protocol> l4proto, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION ( this << packet );

  NetfilterConntrackTuple replyTuple;

  if (!InvertTuple (replyTuple, tuple, l3proto, l4proto))
    {
      return m_hash.end ();
    }

  // Invoke l4proto->New

  // Find expectatons here

  NS_LOG_DEBUG (":: Creating an unconfirmed entry for this tuple ::");
  m_unconfirmed[tuple] = IpConntrackInfo ();

  TupleHashI it = m_unconfirmed.find (tuple);

  return it;
}

uint32_t
Ipv4Netfilter::ResolveNormalConntrack (Ptr<Packet> packet, uint32_t protocolFamily, uint8_t protocol,
                                       Ptr<NetfilterConntrackL3Protocol> l3Protocol, Ptr<NetfilterConntrackL4Protocol> l4Protocol,
                                       int& setReply, ConntrackInfo_t& ctInfo, Ipv4Header ipHeader)
{
  NS_LOG_FUNCTION (this << packet);
  //NetfilterConntrackTuple tuple (ipHeader.GetSource(), 0, ipHeader.GetDestination(), 0);
  NetfilterConntrackTuple tuple;
  uint8_t conntrackInfo = 0;

  /* Get a tuple from the information in the packet */
  if (!NetfilterConntrackGetTuple (packet, protocolFamily, protocol, tuple, l3Protocol, l4Protocol))
    {
      NS_LOG_DEBUG ("Cannot create a tuple from the packet");
      return -1;
    }

  TupleHashI it = m_hash.find (tuple);

  if (it == m_hash.end ())
    {
      NS_LOG_DEBUG ("No tuple found");
      //TupleHashI newIt = NewConnection(tuple, l3Protocol, l4Protocol, packet);
      it = NewConnection (tuple, l3Protocol, l4Protocol, packet);
    }

  NetfilterConntrackTuple replyTuple;

  if (!InvertTuple (replyTuple, tuple, l3Protocol, l4Protocol))
    {
      return -1;
    }

  currentOriginalTuple = tuple;
  currentReplyTuple = replyTuple;

  /* TODO: Add a pointer to the hashed tuple in IpConntrackInfo()
   * and store these tuples somehwere, when you destruct then you
   * you have to take care of the tuples stored in the vector as
   * well
   */
  if ((it->first).GetDirection () == (uint8_t)IP_CT_DIR_REPLY)
    {
      NS_LOG_DEBUG (":: **** This is a REPLY *** ::");
      conntrackInfo = IP_CT_ESTABLISHED + IP_CT_IS_REPLY;
      setReply = 1;
    }
  else
    {
      NS_LOG_DEBUG (":: Packet is in the original direction ::");
      if ( m_hash[tuple].GetStatus () & IPS_SEEN_REPLY)
        {
          NS_LOG_DEBUG (":: Connection ESTABLISHED! ::");
          conntrackInfo = IP_CT_ESTABLISHED;
        }
      else
        {
          NS_LOG_DEBUG (":: New connection :: ");
          conntrackInfo = IP_CT_NEW;
        }

    }

  m_unconfirmed[tuple].SetInfo (conntrackInfo);
  //UpdateConntrackInfo (conntrackInfo);
  /*NS_LOG_DEBUG ("Adding the conntrack packet tag" );
  ConntrackTag ctTag;
  bool tagFound = packet->PeekPacketTag (ctTag);

  if (!tagFound)
    packet->AddPacketTag (ConntrackTag (conntrackInfo));
  else
    NS_LOG_DEBUG ("Tag already present");
    */

  return NF_ACCEPT;

}

uint32_t
Ipv4Netfilter::NetfilterConntrackIn (Hooks_t hook, Ptr<Packet> packet, Ptr<NetDevice> in,
                                     Ptr<NetDevice> out, ContinueCallback& ccb)
{
  NS_LOG_DEBUG ("::: Executing Hook Function :::");
  int setReply = 0;
  ConntrackInfo_t ctInfo;
  /* If this packet has been seen previously, Ignore. */

  /* Find layer 3 helper for this packet */
  Ptr<NetfilterConntrackL3Protocol> l3proto = FindL3ProtocolHelper (1);

  Ipv4Header ipHeader;

  packet->PeekHeader (ipHeader);

  NS_LOG_DEBUG ( "IP header protocol: " << (int)ipHeader.GetProtocol ());

  Ptr<NetfilterConntrackL4Protocol> l4proto = FindL4ProtocolHelper (ipHeader.GetProtocol ());

  // HERE WE ARE JUST IGNORING THE PROTOCOLS WITHOUT A HELPER
  // todo: we need to return here afterwards and find a better solution and more
  // generic solution, this is just a hot fix so that the porting can be finished
  if (l4proto == 0)
    {
      NS_LOG_DEBUG ( "Netfilter: Letting packet pass without treatment, there is no helper for protocol: " << (int)ipHeader.GetProtocol ());
      return NF_ACCEPT;
    }

  ResolveNormalConntrack (packet, 1 /* PF */, ipHeader.GetProtocol (), l3proto, l4proto, setReply, ctInfo, ipHeader);

  // Call layer 4 Packet callback
  //uint32_t ret = l4proto->packet(packet, protocolFamily, hook);

  if (setReply)
    {
      NS_LOG_DEBUG ("Setting IPS_SEEN_REPLY");
      m_hash[currentOriginalTuple].SetStatus ( IPS_SEEN_REPLY );
      m_hash[currentReplyTuple].SetStatus ( IPS_SEEN_REPLY );
    }

  return NF_ACCEPT;

}

uint32_t
Ipv4Netfilter::NetfilterConntrackConfirm (Ptr<Packet> packet)
//, NetfilterConntrackTuple& orig,
//          NetfilterConntrackTuple& reply)
{
  NS_LOG_FUNCTION ( this << packet );
  /* If this packet has been seen previously, Ignore. */

  /* Find layer 3 helper for this packet */
  /*Ptr<NetfilterConntrackL3Protocol> l3proto = FindL3ProtocolHelper (1);

    Ipv4Header ipHeader;

  //packet->RemoveHeader(ipHeader);
  packet->PeekHeader (ipHeader);

  NS_LOG_DEBUG ( "IP header protocol: " << (int)ipHeader.GetProtocol ());

  Ptr<NetfilterConntrackL4Protocol> l4proto = FindL4ProtocolHelper (ipHeader.GetProtocol ());

  NetfilterConntrackTuple orig;*/

  /* Get a tuple from the information in the packet */
  /*if (!NetfilterConntrackGetTuple (packet, 1, ipHeader.GetProtocol (), orig, l3proto, l4proto))
    {
    NS_LOG_DEBUG ("Cannot create a tuple from the packet");
    return -1;
    }

    NetfilterConntrackTuple reply;


    InvertTuple (reply, orig, l3proto, l4proto);

    currentOriginalTuple = orig;
    currentReplyTuple = reply;

    NS_LOG_DEBUG ("Current Original Tuple: " << currentOriginalTuple.GetSource () << ", " << currentOriginalTuple.GetDestination ());
    NS_LOG_DEBUG ("Current Reply Tuple: " << currentReplyTuple.GetSource () << ", " << currentReplyTuple.GetDestination ());
  //currentTuple[IP_CT_DIR_REPLY] = reply;*/

  /**************************/

  /*ConntrackTag ctTag;

    if (!packet->PeekPacketTag (ctTag))
    {
    NS_LOG_DEBUG ("ConntrackTag not found");
    return 0;
    }
    else {
    if ( CTINFO2DIR (ctTag.GetConntrack ()) != IP_CT_DIR_ORIGINAL)
    return 0;

    if (m_hash.find (orig) != m_hash.end () && m_hash.find (reply) != m_hash.end () )
    {
    NS_LOG_DEBUG ("Entries already present!");
    return NF_DROP;
    }

    NS_LOG_DEBUG ("Creating confirmed hash entries");
  //m_hash[orig] = IpConntrackInfo().SetStatus(IPS_CONFIRMED);
  m_hash[orig] = IpConntrackInfo ();
  m_hash[reply] = IpConntrackInfo ();

  }*/

  if ( CTINFO2DIR (m_unconfirmed[currentOriginalTuple].GetInfo ()) != IP_CT_DIR_ORIGINAL)
    {
      NS_LOG_DEBUG ("Not a packet in the original direction");
      return NF_ACCEPT;
    }

  /*if (m_hash.find (currentOriginalTuple) != m_hash.end () && m_hash.find (currentReplyTuple) != m_hash.end () )
  {
    NS_LOG_DEBUG ("Entries already present!");
    return NF_DROP;
  }*/

  NS_LOG_DEBUG ("Creating confirmed hash entries");
  m_hash[currentOriginalTuple] = m_unconfirmed[currentOriginalTuple];
  m_hash[currentReplyTuple] = m_unconfirmed[currentOriginalTuple];

  return 0;
}

bool
Ipv4Netfilter::InvertTuple (NetfilterConntrackTuple& inverse, NetfilterConntrackTuple& orig,
                            Ptr<NetfilterConntrackL3Protocol> l3Protocol,
                            Ptr<NetfilterConntrackL4Protocol> l4Protocol)
{
  inverse.SetProtocol (orig.GetProtocol ());

  if (!l3Protocol->InvertTuple (inverse, orig))
    {
      return false;
    }

  inverse.SetDirection (orig.GetDirection () == IP_CT_DIR_ORIGINAL ? IP_CT_DIR_REPLY : IP_CT_DIR_ORIGINAL);

  return l4Protocol->InvertTuple (inverse, orig);

}

TupleHash&
Ipv4Netfilter::GetHash ()
{
  return m_hash;
}

#ifdef NOTYET
uint32_t
Ipv4Netfilter::NetfilterDoNat (Hooks_t hookNumber, Ptr<Packet> p,
                               Ptr<NetDevice> in, Ptr<NetDevice> out, ContinueCallback& ccb)
{
  NS_LOG_FUNCTION ( this << p );
  /*ConntrackTag ctTag;

  bool found = p->PeekPacketTag (ctTag);

  if (!found)
  {
    NS_LOG_DEBUG ("Conntrack tag not found");
    return NF_ACCEPT;
  }*/

  /* Conntrack has a higher priority so currentOriginalTuple and
   * currentReply tuple should always be correct
   */

  //std::vector<NatRule>::iterator it = FindNatDevice (out);

  /*if (it == m_natRules.end ())
  {
    NS_LOG_DEBUG ("No NAT rule for device " << out->GetId ());
    return NF_ACCEPT;
  }*/

  NS_LOG_DEBUG ("Current Original Tuple: " << currentOriginalTuple.GetSource () << ", " << currentOriginalTuple.GetDestination ());
  NS_LOG_DEBUG ("Current Reply Tuple: " << currentReplyTuple.GetSource () << ", " << currentReplyTuple.GetDestination ());

  NS_LOG_DEBUG ("ConntrackInfo: " << (uint16_t)m_unconfirmed[currentOriginalTuple].GetInfo () );

  /* TODO: Why are you checking m_hash here when the info is in
   * m_unconfirmed. This could be a problem because NAT is sandwiched
   * between conntrack hook callbacks
   */
  switch (m_unconfirmed[currentOriginalTuple].GetInfo ())
    {
    case IP_CT_RELATED:
    case IP_CT_RELATED + IP_CT_IS_REPLY:
      /* This should be updated when "expectations" are added */
      break;

    case IP_CT_NEW:

      if ( hookNumber == NF_INET_POST_ROUTING )
        {
          NS_LOG_DEBUG ("SRC_NAT: New Connection encountered");

          TupleHashI it;


          if ( (it = m_hash.find (currentOriginalTuple)) != m_hash.end ())
            {
              if ( !((it->second).GetStatus () & IPS_SRC_NAT_DONE) )
                {
                  /* Get a unique tuple and create a mapping */

                  NS_LOG_DEBUG ("Doing rule lookup at device " << out->GetIfIndex ());
                  std::vector<NatRule>::iterator it =
                    FindNatRule ( Ipv4Address (currentOriginalTuple.GetSource ()), out );

                  if ( it == m_natRules.end () )
                    {
                      NS_LOG_DEBUG ("No rule matched!");
                      return NF_ACCEPT;
                    }

                  NS_LOG_DEBUG ("Creating a NAT mapping");

                  /* Create a NULL mapping, which does not contain port numbers */
                  NetfilterConntrackTuple mapping =
                    NetfilterConntrackTuple (it->GetMangledSource (), nextAvailablePort,
                                             currentOriginalTuple.GetDestination (), 9);
                  mapping.SetDirection (IP_CT_DIR_ORIGINAL);
                  nextAvailablePort++;

                  NS_LOG_DEBUG ("Creating a NAT mapping for the tuple " << currentOriginalTuple
                                                                        << ": " << mapping );
                  m_natMappings[currentOriginalTuple] = mapping;
                  m_natReplyLookup[mapping] = currentOriginalTuple;
                }


              NS_LOG_DEBUG (":: Translating addresses and fixing IP checksum ::");
              NetfilterNatPacket (hookNumber, p);
            }
          else
            {
              NS_LOG_DEBUG ("BUG: currentTuple non-existent in hash!");
            }
        }

      break;

    default:
      NS_LOG_DEBUG ("SRC_NAT: Connection is established!");
      NS_LOG_DEBUG (":: Translating addresses and fixing IP checksum ::");
      NetfilterNatPacket (hookNumber, p);

    }


  return NF_ACCEPT;
}


void
Ipv4Netfilter::AddNatRule (NatRule natRule)
{
  m_natRules.push_back (natRule);
}

std::vector<NatRule>::iterator
Ipv4Netfilter::FindNatRule (NatRule natRule)
{
  std::vector<NatRule>::iterator it = m_natRules.begin ();

  for (; it != m_natRules.end ();  it++)
    {
      if ( *it == natRule )
        {
          return it;
        }
    }

  return m_natRules.end ();
}

std::vector<NatRule>::iterator
Ipv4Netfilter::FindNatRule (Ipv4Address orig, Ptr<NetDevice> out)
{
  std::vector<NatRule>::iterator it = m_natRules.begin ();

  NS_LOG_DEBUG ("Number of rules: " << m_natRules.size () );
  NS_LOG_DEBUG ("Orig: " << orig );

  for (; it != m_natRules.end ();  it++)
    {
      NS_LOG_DEBUG ("Rule source: " << it->GetOriginalSource () << ", passed in source: " << orig);
      NS_LOG_DEBUG ("Rule device: " << it->GetDevice () << ", passed in dev: " << out);
      if ( it->GetOriginalSource () == orig && it->GetDevice () == out)
        {
          NS_LOG_DEBUG ("Rule match found!");
          return it;
        }
    }

  return m_natRules.end ();
}

void
Ipv4Netfilter::EnableNat ()
{
  m_enableNat = 1;

  NS_LOG_DEBUG (":: Enabling NAT ::");

  NetfilterHookCallback doNat = MakeCallback (&Ipv4Netfilter::NetfilterDoNat, this);

  Ipv4NetfilterHook natCallback1 = Ipv4NetfilterHook (1, NF_INET_POST_ROUTING, NF_IP_PRI_NAT_SRC, doNat);
  Ipv4NetfilterHook natCallback2 = Ipv4NetfilterHook (1, NF_INET_PRE_ROUTING, NF_IP_PRI_NAT_DST, doNat);


  this->RegisterHook (natCallback1);
  this->RegisterHook (natCallback2);
}

uint32_t
Ipv4Netfilter::NetfilterNatPacket (Hooks_t hookNumber, Ptr<Packet> p)
{
  NS_LOG_FUNCTION ( this << p );
  Ipv4Header ipHeader;
  uint16_t dstPort; //, srcPort;
  Ipv4Address dstAddress, srcAddress;

  p->RemoveHeader (ipHeader);

  uint16_t protocol = ipHeader.GetProtocol ();

  if (hookNumber == NF_INET_POST_ROUTING)
    {
      NetfilterConntrackTuple mapped = m_natMappings[currentOriginalTuple];

      ipHeader.SetSource (mapped.GetSource ());

      if (protocol == IPPROTO_TCP)
        {
          TcpHeader tcpHeader;

          p->RemoveHeader (tcpHeader);

          tcpHeader.SetSourcePort (mapped.GetSourcePort ());

          p->AddHeader (tcpHeader);

        }
      else
        {
          UdpHeader udpHeader;

          p->RemoveHeader (udpHeader);

          udpHeader.SetSourcePort (mapped.GetSourcePort ());

          p->AddHeader (udpHeader);
        }

      p->AddHeader (ipHeader);

      NetfilterConntrackTuple oldOriginalTuple = currentOriginalTuple;


      currentOriginalTuple = NetfilterConntrackTuple (mapped.GetSource (), mapped.GetSourcePort (),
                                                      currentOriginalTuple.GetDestination (),
                                                      currentOriginalTuple.GetDestinationPort ());

      currentOriginalTuple.SetDirection (IP_CT_DIR_ORIGINAL);

      currentReplyTuple = NetfilterConntrackTuple (currentOriginalTuple.GetDestination (),
                                                   currentOriginalTuple.GetDestinationPort (),
                                                   mapped.GetSource (), mapped.GetSourcePort ());

      currentReplyTuple.SetDirection (IP_CT_DIR_REPLY);

      std::cout << " New Original Tuple: " << currentOriginalTuple << std::endl;
      std::cout << " New Reply Tuple: " << currentReplyTuple << std::endl;

      m_unconfirmed[currentOriginalTuple] = m_unconfirmed[oldOriginalTuple];

    }
  else if (hookNumber == NF_INET_PRE_ROUTING)
    {
      NS_LOG_DEBUG ("Mapping back to LAN address");
      //NetfilterConntrackTuple mapped = m_natMappings[currentReplyTuple];
      NetfilterConntrackTuple temp = currentReplyTuple;
      temp.SetDirection (IP_CT_DIR_ORIGINAL);
      TranslationMapI transIt = m_natReplyLookup.find (temp);

      if (transIt == m_natMappings.end ())
        {
          NS_LOG_DEBUG ("No such mapping found!");
          return NF_ACCEPT;
        }

      dstPort = (transIt->second).GetSourcePort ();
      dstAddress = (transIt->second).GetSource ();

      currentOriginalTuple = NetfilterConntrackTuple ((transIt->second).GetSource (), (transIt->second).GetSourcePort (), dstAddress, dstPort);
      currentReplyTuple = NetfilterConntrackTuple (dstAddress, dstPort, (transIt->second).GetSource (), (transIt->second).GetSourcePort ());

      NS_LOG_DEBUG ("Setting Destination IP address to : " << dstAddress);
      ipHeader.SetDestination (dstAddress);

      if (protocol == IPPROTO_TCP)
        {
          TcpHeader tcpHeader;

          p->RemoveHeader (tcpHeader);

          tcpHeader.SetDestinationPort (dstPort);

          p->AddHeader (tcpHeader);

        }
      else
        {
          UdpHeader udpHeader;

          p->RemoveHeader (udpHeader);

          udpHeader.SetDestinationPort (dstPort);
          NS_LOG_DEBUG ("Setting destination port to: " << dstPort);

          p->AddHeader (udpHeader);
        }

      p->AddHeader (ipHeader);
    }



  return NF_ACCEPT;

}
#endif 

} // Namespace ns3
