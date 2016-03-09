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
#ifndef IP_CONNTRACK_INFO
#define IP_CONNTRACK_INFO

#include <stdint.h>


namespace ns3 {
typedef enum
{
  /*Established connection on either direction*/
  IP_CT_ESTABLISHED,
  /*Like a NEW connection but is related to an existing connection,ICMP error or FTP data sessions*/
  IP_CT_RELATED,
  /*Started a new connection*/
  IP_CT_NEW,
  /*Indicates the reply direction*/
  IP_CT_IS_REPLY,
  /*Number of distinct IP_CT types*/
  IP_CT_NUMBER = IP_CT_IS_REPLY * 2 - 1
} ConntrackInfo_t;

typedef enum
{
  IP_CT_DIR_ORIGINAL,
  IP_CT_DIR_REPLY,
  IP_CT_DIR_MAX,
} ConntrackDirection_t;

typedef enum
{
  /*Its an expected connection,bit 0 set*/
  IPS_EXPECTED_BIT = 0,
  IPS_EXPECTED = (1 << IPS_EXPECTED_BIT),

  /*Packets seen both directions,bit 1 set*/
  IPS_SEEN_REPLY_BIT = 1,
  IPS_SEEN_REPLY = (1 << IPS_SEEN_REPLY_BIT),

  /*Conntrack never early-expired*/
  IPS_ASSURED_BIT = 2,
  IPS_ASSURED = (1 << IPS_ASSURED_BIT),

  /*Connection is confirmed,originating packet has left device*/
  IPS_CONFIRMED_BIT = 3,
  IPS_CONFIRMED = (1 << IPS_CONFIRMED_BIT),

  IPS_SRC_NAT_BIT = 4,
  IPS_SRC_NAT = (1 << IPS_SRC_NAT_BIT),

  IPS_DST_NAT_BIT = 5,
  IPS_DST_NAT = (1 << IPS_DST_NAT_BIT),

  IPS_NAT_MASK = (IPS_DST_NAT | IPS_SRC_NAT),

  IPS_SEQ_ADJUST_BIT = 6,
  IPS_SEQ_ADJUST = (1 << IPS_SEQ_ADJUST_BIT),

  IPS_SRC_NAT_DONE_BIT = 7,
  IPS_SRC_NAT_DONE = (1 << IPS_SRC_NAT_DONE_BIT),

  IPS_DST_NAT_DONE_BIT = 8,
  IPS_DST_NAT_DONE = (1 << IPS_DST_NAT_DONE_BIT),

  IPS_NAT_DONE_MASK = (IPS_DST_NAT_DONE | IPS_SRC_NAT_DONE),

  /* Connection is dying (removed from lists), can not be unset. */
  IPS_DYING_BIT = 9,
  IPS_DYING = (1 << IPS_DYING_BIT),

} ConntrackStatus_t;

class IpConntrackInfo
{
public:
  /*Sets the conntrack status to 0*/
  IpConntrackInfo ();
  /*Sets the conntrack status to the value passed*/
  IpConntrackInfo (uint32_t);
  /*Method to set the status*/
  void SetStatus (uint32_t status);
  /*Returns Conntrack status*/
  uint32_t GetStatus ();
  /*Confirming if the packet has left the device*/
  bool IsConfirmed ();
  /*Setting the Confirmed bit*/
  void SetConfirmed ();
  /*checking to see if connection is dying*/
  bool IsDying ();
  /*Setting the connection bit to dead*/
  void SetDying ();
  /*Setting the info field of Conntrack*/
  void SetInfo (uint8_t info);
  /*Get the info field of Conntrack*/
  uint8_t GetInfo ();

  ConntrackDirection_t ConntrackInfoToDirection (ConntrackInfo_t ctinfo);

private:
  /*Status of the Connection*/
  uint32_t m_status;
  /*Information on connection */
  uint8_t m_info;
};

}

#endif /* IP_CONNTRACK_INFO */
