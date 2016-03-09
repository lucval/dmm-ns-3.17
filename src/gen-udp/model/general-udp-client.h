/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 *
 */

#ifndef GENERAL_UDP_CLIENT_H
#define GENERAL_UDP_CLIENT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

using namespace std;

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup udpclientserver
 * \class GeneralUdpClient
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class GeneralUdpClient : public Application
{
public:
  static TypeId
  GetTypeId (void);

  GeneralUdpClient ();

  GeneralUdpClient (int type);

  virtual ~GeneralUdpClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Ipv4Address ip, uint16_t port);
  void SetRemote (Address ip, uint16_t port);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTransmit (Time dt);
  void Send (void);

  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;
  uint32_t m_type;
  uint32_t m_sliceCount;
  bool m_voipActive;
  uint32_t m_talkFrame;
  uint32_t m_silenceFrame;
  double m_nextFrameTime;
  uint32_t m_sent;
  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;
  EventId m_sendFrameEvent;
  ostringstream ClientFile1;

};

} // namespace ns3

#endif /* GENERAL_UDP_CLIENT_H */
