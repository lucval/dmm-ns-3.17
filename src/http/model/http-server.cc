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

#include <limits>
#include <deque>

#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/pointer.h"
#include "ns3/object.h"

#include "http-server.h"
#include "http-controller.h"

#include "ns3/tcp-socket.h"

NS_LOG_COMPONENT_DEFINE ("HttpServer");

namespace ns3 {
namespace http {

NS_OBJECT_ENSURE_REGISTERED (HttpServer);

TypeId
HttpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::http::HttpServer")
    .SetParent<Application> ()
    .AddConstructor<HttpServer> ()
    .AddAttribute ("HttpController", "The controller",
                   PointerValue (0),
                   MakePointerAccessor (&HttpServer::SetController,
                                        &HttpServer::GetController),
                   MakePointerChecker<HttpController> ())
    .AddAttribute ("PageTimeout", "Time out value for each one web page",
                   UintegerValue (1),
                   MakeUintegerAccessor (&HttpServer::m_pageTimeout),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Local", "The local address for server",
                   AddressValue (),
                   MakeAddressAccessor (&HttpServer::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("RunNo", "number of runs for simulation",
                   UintegerValue (0),
                   MakeUintegerAccessor (&HttpServer::m_run),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TransportId", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpServer::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Persistent", "Set if the connection is persistent connection or not.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&HttpServer::m_persistent),
                   MakeBooleanChecker ())
  ;
  return tid;
}

HttpServer::HttpServer ()
{
  m_firstConnection = true;
}

HttpServer::~HttpServer ()
{
  m_firstConnection = false;
}

void
HttpServer::SetController (Ptr<HttpController> controller)
{
  NS_LOG_FUNCTION (this);
  m_controller = controller;
}

Ptr<HttpController>
HttpServer::GetController () const
{
  NS_LOG_FUNCTION (this);
  return m_controller;
}

void HttpServer::StartApplication ()
{
  NS_LOG_FUNCTION (this);
  // Called at time specified by Start
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
    }
  NS_ASSERT (m_socket != 0);
  m_socket->SetAcceptCallback (MakeCallback (&HttpServer::ConnectionRequested, this),
                               MakeCallback (&HttpServer::ConnectionAccepted, this));

  /// By this time save the serve application information in the controller
  SetServerApplication ();
  NS_LOG_LOGIC (Simulator::Now () << " Server ready waiting for Client request");
}

void HttpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  // Called at time specified by Stop
  NS_LOG_DEBUG (Simulator::Now () << " Server: Stop Application");

  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
HttpServer::SetServerApplication ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = GetNode ();
  for (uint32_t i = 0; i < node->GetNApplications (); ++i)
    {
      if (node->GetApplication (i) == this)
        {
          m_controller->SetServerApplication (node, i);
        }
    }
}

bool
HttpServer::ConnectionRequested (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);
  NS_LOG_DEBUG (Simulator::Now () << " Socket = " << socket << " " << " Server: ConnectionRequested");
  return true;
}

void
HttpServer::ConnectionAccepted (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);
  /*
   * We have a new socket and need to set the receive callback and respond with the response
   * when the request arrives
   */
  NS_LOG_DEBUG (socket << " " << Simulator::Now () << " Server::Successful socket id : " << socket << " Connection Accepted");
  m_socket = socket;

  // Now we've blocked, so register the up-call for later
  m_socket->SetRecvCallback (MakeCallback (&HttpServer::ServerReceive, this));

  if (m_firstConnection)
    {
      m_firstConnection = false;
      /// This is the first connection accepted, call the first web session
      m_controller->StartFirstWebSession ();
    }
  else
    {
      NS_ASSERT (!m_persistent);
      /// After the new connection is established, we start the next client send immediately
      m_controller->CallClientSend ();
    }
}

void
HttpServer::ServerReceive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet = socket->Recv ();
  /// We should not have a null packet
  if (packet == NULL)
    {
      NS_LOG_WARN ("We should not receive this part");
      return;
    }

  AduContainer serverContainer = m_controller->GetServerContainer ();
  uint32_t bytesToRecv = packet->GetSize ();
  while (bytesToRecv)
    {
      uint32_t requestSize = FindFirstRequestSize (serverContainer);
      NS_LOG_DEBUG ("the request size " << requestSize);
      if (!requestSize)
        {
          NS_FATAL_ERROR ("This should not happen");
          return;
        }

      if (requestSize > bytesToRecv)
        {
          // Received less than a whole ADU. Reduce its size by the number of bytes received.
          requestSize -= bytesToRecv;
          UpdateFirstRequestSize (serverContainer, requestSize);
          NS_LOG_LOGIC ("Received " << bytesToRecv << " bytes of an ADU. " << serverContainer.objects.front ().size << " bytes to go.");
          break;
        }
      else
        {
          // Front ADU size <= bytesToRecv: remove the whole thing and continue receiving
          bytesToRecv -= requestSize;
          // Remove the next request adu
          RemoveFirstRequestSize (serverContainer);
          serverContainer.serverReceiveTime = Simulator::Now ().GetSeconds ();
          m_controller->SetServerContainer (serverContainer);
          if (serverContainer.objects.size ())
            {
              // After receiving the request we need to schedule the web response
              m_controller->ScheduleNextServerSend (socket);
            }
          continue;
        }
    }
}

uint32_t
HttpServer::FindFirstRequestSize (AduContainer & serverContainer)
{
  NS_LOG_FUNCTION (this);
  for (deque<ADU>::iterator i = serverContainer.objects.begin (); i != serverContainer.objects.end (); ++i)
    {
      if (i->messageType == ADU::REQUEST)
        {
          return i->size;
        }
    }
  return 0;
}

void
HttpServer::UpdateFirstRequestSize (AduContainer & serverContainer, uint32_t requestSize)
{
  NS_LOG_FUNCTION (this << requestSize);
  for (deque<ADU>::iterator i = serverContainer.objects.begin (); i != serverContainer.objects.end (); ++i)
    {
      if (i->messageType == ADU::REQUEST)
        {
          i->size = requestSize;
          break;
        }
    }
  m_controller->SetServerContainer (serverContainer);
}

void
HttpServer::RemoveFirstRequestSize (AduContainer & serverContainer)
{
  NS_LOG_FUNCTION (this);
  for (deque<ADU>::iterator i = serverContainer.objects.begin (); i != serverContainer.objects.end (); ++i)
    {
      if (i->messageType == ADU::REQUEST)
        {
          i = serverContainer.objects.erase (i);
          break;
        }
    }
  m_controller->SetServerContainer (serverContainer);
}

void
HttpServer::PrintContainer (const AduContainer & container)
{
  NS_LOG_FUNCTION (this);
  for (deque< deque <ADU> >::const_iterator i = container.pages.begin (); i != container.pages.end (); ++i)
    {
      deque<ADU> objects = *i;
      for (deque<ADU>::const_iterator j = objects.begin (); j != objects.end (); ++j)
        {
          NS_LOG_DEBUG ("The adu size " << j->size);
          if (j->messageType == ADU::REQUEST)
            {
              NS_LOG_DEBUG ("This is a request");
            }
          else
            {
              NS_LOG_DEBUG ("This is a response");
            }
        }
    }
}

} // namespace http
} // namespace ns3
