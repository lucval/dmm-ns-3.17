/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Yufei Cheng
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
 * Author: Yufei Cheng   <yfcheng@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "http-controller.h"
//#include "http-distribution.h"
#include "http-client.h"
#include "http-server.h"

#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/tcp-socket.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <deque>

using namespace std;

NS_LOG_COMPONENT_DEFINE ("HttpController");

namespace ns3 {
namespace http {

NS_OBJECT_ENSURE_REGISTERED (HttpController);

TypeId
HttpController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::http::HttpController")
    .SetParent<Object> ()
    .AddConstructor<HttpController> ()
    .AddAttribute ("SegmentSize", "The largest segment size",
                   UintegerValue (1460),
                   MakeUintegerAccessor (&HttpController::m_segmentSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

// \brief Constructor
HttpController::HttpController ()
{
}

// \brief Destructor
HttpController::~HttpController ()
{
}

void
HttpController::SetServerContainer (const AduContainer & container)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_serverContainer = container;
}

void
HttpController::SetClientContainer (const AduContainer & container)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_clientContainer = container;
}

AduContainer &
HttpController::GetServerContainer ()
{
  return m_serverContainer;
}

AduContainer &
HttpController::GetClientContainer ()
{
  return m_clientContainer;
}

void
HttpController::SetHttpVersion (bool pipelining, bool persistent)
{
  NS_LOG_FUNCTION (this << pipelining << persistent);
  m_persistent = persistent;
  if (m_persistent)
    {
      m_pipelining = pipelining;
    }
  else
    {
      m_pipelining = false;
    }
}

void
HttpController::Cleanup ()
{
  NS_LOG_FUNCTION (this);
  m_serverContainer.Clear ();
  m_clientContainer.Clear ();
}

void
HttpController::SetClientApplication (Ptr<Node> node, uint32_t id)
{
  NS_LOG_FUNCTION (this);
  m_clientNode = node;
  m_clientId = id;
}

void
HttpController::SetServerApplication (Ptr<Node> node, uint32_t id)
{
  NS_LOG_FUNCTION (this);
  m_serverNode = node;
  m_serverId = id;
}

void
HttpController::SetClientSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  m_clientSocket = socket;
}

void
HttpController::StartNewClientSocket ()
{
  NS_LOG_FUNCTION (this);

  Ptr<HttpClient> httpClient = DynamicCast<HttpClient>
      (m_clientNode->GetApplication (m_clientId));

  httpClient->StartNewSocket ();
}

void
HttpController::StartFirstWebSession ()
{
  Ptr<HttpClient> httpClient = DynamicCast<HttpClient>
      (m_clientNode->GetApplication (m_clientId));

  httpClient->StartFirstWebSession ();
}

void
HttpController::ScheduleNextClientSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  // It's expected that this won't be called if m_serverContainer is
  // empty, since the last ADU should have size==0, serve as the FIN message
  NS_ASSERT (!m_clientContainer.objects.empty ());

  if (m_pipelining)
    {
      // pipelining
      m_delayTime = m_clientContainer.objects.front ().requestObjectGapTime;

      if (m_delayTime > 0)
        {
          if (m_clientContainer.clientSentTime < Seconds (0))
            {
              NS_FATAL_ERROR ("client_send_wait: haven't sent anything yet. Adu container corrupted or wrong?");
            }
          else
            {
              // (m_lastSendTime + adu.requestGapTime) is the earliest
              // time at which the data shall be sent.
              double sendTime = m_clientContainer.clientSentTime + m_delayTime;
              NS_LOG_DEBUG ("The send time " << sendTime << " Now time " << Simulator::Now ().GetSeconds ());
              if (sendTime >= Simulator::Now ().GetSeconds ())
                {
                  NS_LOG_LOGIC ("client_send_wait: Scheduling ClientSend at " << sendTime << "s");
                  Simulator::Schedule (Seconds (sendTime - Simulator::Now ().GetSeconds ()),
                                       &HttpController::ClientSend, this, socket);
                }
              else
                {
                  NS_LOG_LOGIC ("client_send_wait: Scheduling ClientSend immediately (was overdue by " << Simulator::Now ().GetSeconds () - sendTime << "s");
                  Simulator::ScheduleNow (&HttpController::ClientSend, this, socket);
                }
            }
        }
      else
        {
          NS_LOG_LOGIC ("neither client_recv_wait nor client_send_wait: calling ClientSend");
          Simulator::ScheduleNow (&HttpController::ClientSend, this, socket);
        }
    }
  else
    {
      // Non-pipelining
      NS_LOG_LOGIC ("This is the non-pipelining option, need to determine if it is persistent connection or not");

      /// For the non-persistent option to work, we need to first have the pipelining as false
      if (!m_persistent)
        {
          NS_LOG_DEBUG ("Start a new connection for next client sent");
          StartNewClientSocket ();
        }
      else
        {
          NS_LOG_DEBUG ("This is the persistent connection without pipelining");
          Simulator::ScheduleNow (&HttpController::ClientSend, this, socket);
        }
    }
}

void
HttpController::ScheduleNextServerSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  // It's expected that this won't be called if m_serverContainer is
  // empty, since the last ADU should have size==0, serve as the FIN message
  NS_ASSERT (!m_serverContainer.objects.empty ());

  m_delayTime = m_serverContainer.objects.front ().serverDelayTime;
  NS_LOG_DEBUG ("The server delay time " << m_delayTime);
  if (m_delayTime > 0)
    {
      // time at which the data may be sent.
      double sendTime = m_serverContainer.serverReceiveTime + m_delayTime;
      NS_LOG_DEBUG ("The sende time here " << sendTime << " The now " << Simulator::Now ().GetSeconds ());
      if (sendTime >= Simulator::Now ().GetSeconds ())
        {
          NS_LOG_LOGIC ("server_send_wait: Scheduling DoSend at " << sendTime << "s");
          Simulator::Schedule (Seconds (sendTime - Simulator::Now ().GetSeconds ()),
                               &HttpController::ServerSend, this, socket);
        }
      else
        {
          // past due because that we are doing pipelining
          NS_LOG_LOGIC ("server_send_wait: Scheduling ServerSend immediately (was overdue by " << Simulator::Now ().GetSeconds () - sendTime << "s");
          Simulator::ScheduleNow (&HttpController::ServerSend, this, socket);
        }
    }
  else
    {
      NS_LOG_LOGIC ("neither server_recv_wait nor server_send_wait: calling ServerSend");
      // When both the server_recv_wait and server_recv_wait times are zero, it
      // means to send those bytes immediately.
      Simulator::ScheduleNow (&HttpController::ServerSend, this, socket);
    }
}

void
HttpController::FirstObjectClientSend (Ptr<Socket> socket, double delay)
{
  NS_LOG_FUNCTION (this << socket << delay);
  NS_LOG_DEBUG ("The second now " << Simulator::Now ().GetSeconds ());
  if (m_persistent)
    {
      Simulator::Schedule (Seconds (delay - Simulator::Now ().GetSeconds ()), &HttpController::ClientSend, this, socket);
    }
  else
    {
      Simulator::Schedule (Seconds (delay - Simulator::Now ().GetSeconds ()), &HttpController::StartNewClientSocket, this);
    }
}

void
HttpController::CallClientSend ()
{
  NS_LOG_FUNCTION (this);
  ClientSend (m_clientSocket);
}

void
HttpController::ClientSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (CheckNextRequest (m_clientContainer))
    {
      ADU adu = FindRemoveNextRequest (m_clientContainer);

      // When we call Client Send function, the first ADU should be request
      NS_ASSERT (adu.messageType == ADU::REQUEST);

      // Stuff the whole ADU into the send buffer, one packet at a time.
      uint32_t bytesToSend = adu.size;
      uint32_t bytesQueued = 0;
      while (bytesToSend)
        {
          int bytesAccepted;
          if (bytesToSend > m_segmentSize)
            {
              bytesAccepted = socket->Send (Create<Packet> (m_segmentSize));
            }
          else
            {
              bytesAccepted = socket->Send (Create<Packet> (bytesToSend));
            }
          if (bytesAccepted > 0)
            {
              bytesToSend -= bytesAccepted;
              bytesQueued += bytesAccepted;
            }
          else if (socket->GetErrno () == TcpSocket::ERROR_MSGSIZE)
            {
              // This part might have better way
              NS_FATAL_ERROR ("Send buffer filled while sending ADU bytes already queued). Increase TcpSocket's SndBufSize.");
            }
          else if (socket->GetErrno () == TcpSocket::ERROR_NOTCONN)
            {
              // This means that the socket was already closed by us for sending, somehow.
              NS_FATAL_ERROR ("ERROR_NOTCONN while sending - this is a bug");
            }
          else
            {
              NS_FATAL_ERROR ("Error sending " << bytesToSend << " bytes of data (" << bytesQueued << " bytes already queued). TcpSocket errno == " << socket->GetErrno ());
            }
        }
      /// This last send time record the time to send out the previous ADU, this is the case for both client and server send
      m_clientContainer.clientSentTime = Simulator::Now ().GetSeconds ();
      NS_LOG_DEBUG ("The clientContaienr send time " << m_clientContainer.clientSentTime);

      if (m_pipelining)
        {
          NS_LOG_LOGIC ("This is the pipelining request option");
          Simulator::ScheduleNow (&HttpController::ScheduleNextClientSend, this, socket);
        }
      /// When it is not pipelining, we wait for the previous response ADU to arrive before sending next request
    }
  else
    {
      NS_LOG_LOGIC ("No request to work on now, waiting for the next web page");
    }
}

void
HttpController::ServerSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (!m_serverContainer.objects.empty ());

  ADU adu = m_serverContainer.objects.front ();
  // When we call Server Send function, the first ADU should be response or the terminator
  NS_ASSERT (adu.messageType == ADU::RESPONSE);
  // remove the ADU we are about to send
  m_serverContainer.objects.pop_front ();

  // Stuff the whole ADU into the send buffer, one packet at a time.
  uint32_t bytesToSend = adu.size;
  NS_LOG_DEBUG ("The byte to send size " << bytesToSend);
  uint32_t bytesQueued = 0;
  while (bytesToSend)
    {
      int bytesAccepted;
      if (bytesToSend > m_segmentSize)
        {
          bytesAccepted = socket->Send (Create<Packet> (m_segmentSize));
        }
      else
        {
          bytesAccepted = socket->Send (Create<Packet> (bytesToSend));
        }
      if (bytesAccepted > 0)
        {
          bytesToSend -= bytesAccepted;
          bytesQueued += bytesAccepted;
        }
      else if (socket->GetErrno () == TcpSocket::ERROR_MSGSIZE)
        {
          // This part might have better way
          NS_FATAL_ERROR ("Send buffer filled while sending ADU  bytes already queued). Increase TcpSocket's SndBufSize.");
        }
      else if (socket->GetErrno () == TcpSocket::ERROR_NOTCONN)
        {
          // This means that the socket was already closed by us for sending, somehow.
          NS_FATAL_ERROR ("ERROR_NOTCONN while sending - this is a bug");
        }
      else
        {
          NS_FATAL_ERROR ("Error sending " << bytesToSend << " bytes of data (" << bytesQueued << " bytes already queued). TcpSocket errno == " << socket->GetErrno ());
        }
    }
}

ADU
HttpController::FindRemoveNextRequest (AduContainer & container)
{
  NS_LOG_FUNCTION (this);

  for (deque<ADU>::iterator i = container.objects.begin (); i != container.objects.end (); ++i)
    {
      if (i->messageType == ADU::REQUEST)
        {
          ADU adu = *i;
          i = container.objects.erase (i);
          return adu;
        }
    }
  NS_FATAL_ERROR ("Not finding the next request, error");
  ADU adu;
  return adu;
}

bool
HttpController::CheckNextRequest (AduContainer & container)
{
  NS_LOG_FUNCTION (this);

  for (deque<ADU>::iterator i = container.objects.begin (); i != container.objects.end (); ++i)
    {
      if (i->messageType == ADU::REQUEST)
        {
          return true;
        }
    }
  return false;
}

void
AduContainer::Clear ()
{
  NS_LOG_FUNCTION (this);
  startTime = 0;
  serverReceiveTime = 0;
  clientSentTime = 0;
  requestPageGaps.clear ();
  objects.clear ();
  pages.clear ();
  remainingPageSize = 0;
  totalPageSize.clear ();
  totalCurrentPage = 0;
}

} // namespace http
} // namespace ns3
