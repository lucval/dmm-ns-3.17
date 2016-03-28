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

#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/object.h"
#include "ns3/random-variable.h"
#include "http-client.h"
#include "http-controller.h"

NS_LOG_COMPONENT_DEFINE ("HttpClient");

namespace ns3 {
namespace http {

NS_OBJECT_ENSURE_REGISTERED (HttpClient);

TypeId
HttpClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::http::HttpClient")
    .SetParent<Application> ()
    .AddConstructor<HttpClient> ()
    .AddAttribute ("HttpController", "The controller",
                   PointerValue (0),
                   MakePointerAccessor (&HttpClient::SetController,
                                        &HttpClient::GetController),
                   MakePointerChecker<HttpController> ())
    .AddAttribute ("PageTimeout", "Time out value for each one web page",
                   UintegerValue (10),
                   MakeUintegerAccessor (&HttpClient::m_pageTimeout),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Peer", "The peer address for the client",
                   AddressValue (),
                   MakeAddressAccessor (&HttpClient::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("TransportId", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpClient::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("RoutingProtocol", "The type of routing protocol to use.",
                   StringValue ("Wired"),
                   MakeStringAccessor (&HttpClient::m_routing),
                   MakeStringChecker ())
    .AddAttribute ("TransportProtocol", "The type of transport protocol to use, this gives the name for tracing.",
                   StringValue ("TCP"),
                   MakeStringAccessor (&HttpClient::m_transport),
                   MakeStringChecker ())
    .AddAttribute ("RunNo", "number of runs for simulation",
                   UintegerValue (0),
                   MakeUintegerAccessor (&HttpClient::m_run),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxSessions", "number of total web sessions for simulation",
                   UintegerValue (1),
                   MakeUintegerAccessor (&HttpClient::m_maxSessions),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Internet", "Set if the working mode is Internet or user-defined.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&HttpClient::m_internet),
                   MakeBooleanChecker ())
    .AddAttribute ("Persistent", "Set if the connection is persistent connection or not.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&HttpClient::m_persistent),
                   MakeBooleanChecker ())
    .AddAttribute ("Pipelining", "Set if the connection is doing pipelining or not.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&HttpClient::m_pipelining),
                   MakeBooleanChecker ())
    .AddAttribute ("UserNumPages", "Number of pages in one session",
                   UintegerValue (20),
                   MakeUintegerAccessor (&HttpClient::m_userNumPages),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("UserNumObjects", "Number of objects per page",
                   UintegerValue (2),
                   MakeUintegerAccessor (&HttpClient::m_userNumObjects),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("UserServerDelay", "User defined server delay",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&HttpClient::m_userServerDelay),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UserPageRequestGap", "User defined request gap for each web page",
                   DoubleValue (0.2),
                   MakeDoubleAccessor (&HttpClient::m_userPageRequestGap),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UserObjectRequestGap", "User defined request gap among web objects in one page",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&HttpClient::m_userObjectRequestGap),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UserRequestSize", "User defined size of the request",
                   UintegerValue (100),
                   MakeUintegerAccessor (&HttpClient::m_userRequestSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("UserResponseSize", "User defined size of the response",
                   UintegerValue (2048),
                   MakeUintegerAccessor (&HttpClient::m_userResponseSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

HttpClient::HttpClient ()
{
  /// Only test for if this is the first connection or not
  m_realFirstObject = true;
}

HttpClient::~HttpClient ()
{
  m_realFirstObject = false;
}

void
HttpClient::SetController (Ptr<HttpController> controller)
{
  NS_LOG_FUNCTION (this);
  m_controller = controller;
}

Ptr<HttpController>
HttpClient::GetController () const
{
  NS_LOG_FUNCTION (this);
  return m_controller;
}

void
HttpClient::StartApplication ()     // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind ();
      m_socket->Connect (m_peer);
    }

  m_socket->SetConnectCallback (MakeCallback (&HttpClient::ConnectionComplete, this),
                                MakeCallback (&HttpClient::ConnectionFailed, this));

  SetClientApplication ();

  m_controller->SetHttpVersion (m_pipelining, m_persistent);
  /// If it is not a persistent connection, there should not be pipelining
  if (!m_persistent)
    {
      m_pipelining = false;
    }

  // Don't bother using SetAcceptCallback -- we don't listen for connections on this node.
}

void
HttpClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (m_socket << " " << Simulator::Now () << " Stop Application");
  if (m_socket != 0)
    {
      NS_LOG_INFO ("Client is closing the socket: " << m_socket);
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  Simulator::Cancel (m_timeoutEvent);
}

void
HttpClient::ConnectionComplete (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  // Get ready to receive.
  socket->SetRecvCallback (MakeCallback (&HttpClient::ClientReceive, this));
  m_controller->SetClientSocket (socket);
}

void
HttpClient::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  // Retry the connection I assume? This isn't really supposed to happen.
  NS_LOG_WARN ("Client failed to open connection. Retrying.");
  m_socket->Connect (m_peer);
}

void
HttpClient::SetClientApplication ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = GetNode ();
  for (uint32_t i = 0; i < node->GetNApplications (); ++i)
    {
      if (node->GetApplication (i) == this)
        {
          m_controller->SetClientApplication (node, i);
        }
    }
}

void
HttpClient::StartNewSocket ()
{
  NS_LOG_FUNCTION (this);

  m_socket = Socket::CreateSocket (GetNode (), m_tid);
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  NS_ASSERT (m_socket != 0);

  m_socket->SetConnectCallback (MakeCallback (&HttpClient::ConnectionComplete, this),
                                MakeCallback (&HttpClient::ConnectionFailed, this));
  m_controller->SetClientSocket (m_socket);
}

void
HttpClient::CallTimeOut (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("Time: " << Simulator::Now () << " Socket: " << socket << " We have a transaction timeout");

  AduContainer clientContainer = m_controller->GetClientContainer ();

  NS_LOG_DEBUG ("the file size " << clientContainer.remainingPageSize);

  double totalCurrentPage = clientContainer.totalCurrentPage;
  NS_LOG_DEBUG ("the total page size " << totalCurrentPage);

  double remainingSize = clientContainer.remainingPageSize;
  double odr = (totalCurrentPage - remainingSize) / totalCurrentPage;

  RecordResults (odr, m_pageTimeout);

  if (clientContainer.pages.size ())
    {
      FirstObject (socket, clientContainer, false);
    }
}

void
HttpClient::StartFirstWebSession ()
{
  SetFileName ("http");
  AduGeneration ();
}


void
HttpClient::FirstObject (Ptr<Socket> socket, const AduContainer & container, bool firstPage)
{
  NS_LOG_FUNCTION (this << socket << firstPage);

  AduContainer clientContainer = container;
  clientContainer.objects = clientContainer.pages.front ();
  clientContainer.pages.pop_front ();
  clientContainer.startTime = Simulator::Now ().GetSeconds ();
  // This part takes care of the next page size
  clientContainer.totalCurrentPage = clientContainer.totalPageSize.front ();
  clientContainer.remainingPageSize = clientContainer.totalPageSize.front ();
  clientContainer.totalPageSize.pop_front ();

  if (!firstPage)
    {
      /// This is the first object inside of one web page, schedule the next client send using the request page gap time
      double nextPageGap = clientContainer.requestPageGaps.front ();
      clientContainer.requestPageGaps.pop_front ();
      // (m_lastSendTime + adu.requestGapTime) is the earliest
      // time at which the next data shall be sent.
      double sendTime = clientContainer.clientSentTime + nextPageGap;
      NS_LOG_DEBUG ("The nextpagegap time is " << nextPageGap << " the send time " << sendTime);

      m_controller->SetClientContainer (clientContainer);
      m_controller->SetServerContainer (clientContainer);
      if (sendTime >= Simulator::Now ().GetSeconds ())
        {
          NS_LOG_LOGIC ("client_send_wait: Scheduling ClientSend at " << sendTime << "s");
          m_controller->FirstObjectClientSend (socket, sendTime);
        }
      else
        {
          NS_LOG_LOGIC ("client_send_wait: Scheduling ClientSend immediately (was overdue by " << Simulator::Now ().GetSeconds () - sendTime << "s");
          if (m_persistent)
            {
              m_controller->ClientSend (socket);
            }
          else
            {
              // After new connection, will call client send
              StartNewSocket ();
            }
        }
    }
  else
    {
      //// This only happens once
      if (m_realFirstObject)
        {
          m_realFirstObject = false;
          NS_LOG_LOGIC ("This is the first web page for the session, send the web page request immediately");
          clientContainer.requestPageGaps.pop_front ();
          m_controller->SetClientContainer (clientContainer);
          m_controller->SetServerContainer (clientContainer);
          m_controller->ClientSend (socket);
        }
      else if (!m_persistent)
        {
          /// if this is not the very first object of the simulation and not persistent connection, we need to work on a new connnection
          StartNewSocket ();
        }
      else
        {
          NS_LOG_LOGIC ("This is the persistent connection");
          clientContainer.requestPageGaps.pop_front ();
          m_controller->SetClientContainer (clientContainer);
          m_controller->SetServerContainer (clientContainer);
          m_controller->ClientSend (socket);
        }
    }
}

void
HttpClient::SessionCompleted ()
{
  NS_LOG_FUNCTION (this);
  /// increase session number and see if we finished all the sessions or not.  The max sessions is defined as
  /// m_maxSessions
  m_sessionNumber++;
  if (m_sessionNumber < m_maxSessions)
    {
      m_controller->Cleanup ();
      // Call the http application to generate ADUs
      AduGeneration ();
    }
  else
    {
      // Call stop application
      NS_LOG_LOGIC ("ALL sessions are done, stopping the TCP connection for this node pair");
      StopApplication ();
    }
}

void
HttpClient::SetFileName (string filename)
{
  NS_LOG_FUNCTION (this << filename);
  /*
   * Create three files to record results from simulation
   */
  // This is the node id to record the results for each client node
  m_nodeId = GetNode ()->GetId ();
  /*
   * Note that we cannot add RequestPageGapTime in the parameter file since it has less number of values than others
   */
  ClientFile1 << filename << "_" << m_nodeId << "_" << m_transport << "_" << m_run << "_delay.csv";
  ClientFile2 << filename << "_" << m_nodeId << "_" << m_transport << "_" << m_run << "_odr.csv";
  ClientFile3 << filename << "_" << m_nodeId << "_" << m_transport << "_" << m_run << "_parameter.csv";
  ClientFile4 << filename << "_" << m_nodeId << "_" << m_transport << "_" << m_run << "_pageGap.csv";

  ofstream out ((ClientFile3.str ()).c_str (), ios::app);
  /*
   * The RequestObjectGapTime is the gap time between different objects within one web page
   * The ServerDelayTime is the time for the server to process the web request
   * The RequestSize is the ADU size for web request
   * The ResponseSize is the ADU size for web response
   */
  out << "RequestObjectGapTime, ServerDelayTime, RequestSize, ResponseSize" << endl;

  ofstream out1 ((ClientFile4.str ()).c_str (), ios::app);
  /*
   * The RequestPageGapTime is the gap time between the last object of previous web page and the next web page
   */
  out1 << "RequestPageGapTime" << endl;
}

void
HttpClient::RecordParameter (ADU requestAdu, ADU responseAdu)
{
  ofstream out ((ClientFile3.str ()).c_str (), ios::app);

  // The time here recorded is milliseconds, we time 1000 to the time to get milliseconds
  out << requestAdu.requestObjectGapTime * 1000 << "," << responseAdu.serverDelayTime * 1000 << "," << requestAdu.size
      << "," << responseAdu.size << endl;
  out.close ();
}

void
HttpClient::RecordPageGapTime (double pageGapTime)
{
  NS_LOG_FUNCTION (this << pageGapTime);
  ofstream out ((ClientFile4.str ()).c_str (), ios::app);
  // The time here recorded is milliseconds
  out << pageGapTime << endl;
  out.close ();
}

void
HttpClient::RecordResults (double odr, double delay)
{
  NS_LOG_FUNCTION (this << odr << delay);
  ofstream out1 ((ClientFile1.str ()).c_str (), ios::app);
  ofstream out2 ((ClientFile2.str ()).c_str (), ios::app);

  NS_LOG_DEBUG ("ODR " << odr);
  NS_LOG_DEBUG ("delay " << delay);

  out1 << delay << "" << endl;
  out2 << odr << "" << endl;

  out1.close ();
  out2.close ();

  /// We need to cancel the timeout event here
  Simulator::Cancel (m_timeoutEvent);
}

void
HttpClient::FinishOnePage (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  AduContainer clientContainer = m_controller->GetClientContainer ();

  NS_LOG_DEBUG ("The client size " << clientContainer.objects.size ());
  double delay = Simulator::Now ().GetSeconds () - clientContainer.startTime;
  // After finishing one web page, the object deliver ratio would be 1
  RecordResults (1, delay);
  if (clientContainer.pages.empty () && clientContainer.objects.empty ())
    {
      NS_LOG_LOGIC ("We have finished one web session");
      SessionCompleted ();
    }
  else if (clientContainer.objects.empty ())
    {
      // We have finished one web page here
      NS_LOG_LOGIC ("We have finished one web page");
      FirstObject (socket, clientContainer, false);
    }
}

void
HttpClient::ClientReceive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  /*
   * Get the Adu container for client and server for the http controller
   * The controller is maintaining both the client and server side containers
   */
  AduContainer clientContainer = m_controller->GetClientContainer ();

  Ptr<Packet> packet = socket->Recv ();

  uint32_t bytesToRecv = packet->GetSize ();

  while (bytesToRecv)
    {
      NS_LOG_DEBUG ("The size " << clientContainer.objects.front ().size);
      if (clientContainer.objects.front ().size > bytesToRecv)
        {
          // Received less than a whole ADU. Reduce its size by the number of bytes received.
          clientContainer.remainingPageSize -= bytesToRecv;
          clientContainer.objects.front ().size -= bytesToRecv;
          m_controller->SetClientContainer (clientContainer);
          NS_LOG_LOGIC ("Received " << bytesToRecv << " bytes of an ADU. " << clientContainer.objects.front ().size << " bytes to go.");
          break;
        }
      else
        {
          // Front ADU size <= bytesToRecv: remove the whole thing and continue receiving
          bytesToRecv -= clientContainer.objects.front ().size;
          clientContainer.remainingPageSize -= clientContainer.objects.front ().size;
          clientContainer.objects.pop_front ();
          m_controller->SetClientContainer (clientContainer);

          NS_LOG_LOGIC ("Finished receiving an ADU. " << clientContainer.objects.size () << " to go.");

          if (clientContainer.objects.empty ())
            {
              // We finish one web page
              FinishOnePage (socket);
            }
          else if (!m_pipelining)
            {
              NS_LOG_LOGIC ("This is the non-pipelining connection, and working with the next web request");
              // The next adu is request or the terminator, schedule the next send
              // We would send the request immediately if past due of the request gap time
              // If this is not persistent connection, right after the next connection is established,
              // we will send the next web request
              m_controller->ScheduleNextClientSend (socket);
            }
          else
            {
              NS_ASSERT (m_pipelining);
            }
          continue;
        }
    }
}

void
HttpClient::AduGeneration ()
{
  NS_LOG_FUNCTION (this);
  uint32_t pages, objects;
  uint32_t requestSize, responseSize;

  // The traffic generator will generate http traffic with persistent transport connections
  if (m_internet)
    {
      /*
       * This is the internet mode in which the parameters needed in traffic generation is
       * generated based on previous defined distribution functions (refer http-random-variable.cc/h
       * for details)
       */
      pages = 15; //m_distribution.GetNumberPages ();
      NS_LOG_DEBUG ("Number of pages " << pages);

      for (uint32_t i = 0; i < pages; i++)
        {
    	  objects = 0;
          // Generate the number of objects on this page
    	  while ((objects < 2) || (objects > 53))
    	  {
    		  UniformVariable u (0,1);
    		  double value = u.GetValue();
    		  objects = (uint32_t)(pow((-value*pow(55,1.1)+value*pow(2,1.1)+pow(55,1.1))/pow(55*2,1.1),-1/1.1)-2);
    	  }
          m_objectsPerPage.push_back (objects);
          NS_LOG_DEBUG ("Number of objects contained in one page " << i << " " << m_objectsPerPage[i]);
        }

      double serverDelay = m_distribution.GetServerDelay ();
      double requestPageGap = 0;
      double requestObjectGap = 0;
      // Initialize the ADU container before generation ADUs
      AduContainer container;
      /// We start fill in the ADU container with request and response ADUs
      for (uint32_t i = 0; i < pages; i++)
        {
          for (uint32_t j = 0; j < m_objectsPerPage[i]; j++)
            {
              // Sample inter-request time
              requestSize = 100; //m_distribution.GetRequestSize ();

              LogNormalVariable objectSize (8.37, 1.37);
                uint32_t objectSizeValue = 0;
                while ((objectSizeValue>2000000) || (objectSizeValue<100))
                	objectSizeValue = objectSize.GetInteger();

                responseSize = objectSizeValue;

              NS_LOG_DEBUG ("The request size " << requestSize << " The response Size " << responseSize);
              /// When this is the first object in one web page, we generate the request gap time between
              /// different web pages
              if (!j)
                {
            	  UniformVariable z (0,1);
            	  requestPageGap = (-log(z.GetValue())/0.033);
                  NS_LOG_DEBUG ("The request page gap here " << requestPageGap);
                }
              else
                {
                  /// This is the request gap time between different web objects within one web page
                  requestObjectGap = m_distribution.GetRequestGap (i, j);
                  NS_LOG_DEBUG ("The request object gap here " << requestObjectGap);
                }
              /*
               * Remove the zero-sized web object sizes if generated from m_distribution function
               * This is a very low possibility occasion, we just put it to make sure we have
               * sensible request and response size
               */
              if (requestSize == 0)
                {
                  requestSize = 1;
                }
              if (responseSize == 0)
                {
                  responseSize = 1;
                }
              ADU requestAdu;
              requestAdu.size = requestSize;
              requestAdu.messageType = ADU::REQUEST;
              requestAdu.requestObjectGapTime = requestObjectGap;

              ADU responseAdu;
              responseAdu.size = responseSize;
              responseAdu.messageType = ADU::RESPONSE;
              responseAdu.serverDelayTime = serverDelay;

              NS_LOG_DEBUG ("i " << i << " pages " << pages << " j " << j << " m_objectsPerPage[i] " << m_objectsPerPage[i]);
              if (i == (pages - 1) && j == (m_objectsPerPage[i] - 1))
                {
                  /// Check if we are at the last object
                  m_lastItem = true;
                }
              if (j == (m_objectsPerPage[i] - 1))
                {
                  m_lastObject = true;
                }
              /// Construct and start the Adu container
              ConstructAndStartAdu (requestAdu, responseAdu, requestPageGap, container);
            }
        }
    }
  else
    {
      AduContainer container;
      for (uint32_t i = 0; i < m_userNumPages; i++)
        {
          m_objectsPerPage.push_back (m_userNumObjects);
          NS_LOG_DEBUG ("Number of objects on page " << i << " " << m_objectsPerPage[i]);
        }
      for (uint32_t i = 0; i < m_userNumPages; i++)
        {
          for (uint32_t j = 0; j < m_objectsPerPage[i]; j++)
            {
              /// This part is the request ADU
              ADU requestAdu;
              requestAdu.size = m_userRequestSize;
              requestAdu.messageType = ADU::REQUEST;
              requestAdu.requestObjectGapTime = m_userObjectRequestGap;

              /// This part is the response ADU
              ADU responseAdu;
              responseAdu.size = m_userResponseSize;
              responseAdu.messageType = ADU::RESPONSE;
              responseAdu.serverDelayTime = m_userServerDelay;

              NS_LOG_DEBUG ("i " << i << " pages " << m_userNumPages << " j " << j << " m_objectsPerPage[i] " << m_objectsPerPage[i]);
              if (i == (m_userNumPages - 1) && j == (m_objectsPerPage[i] - 1))
                {
                  /// Check if we are at the last object
                  m_lastItem = true;
                }
              if (j == (m_objectsPerPage[i] - 1))
                {
                  m_lastObject = true;
                }
              /// Construct and start the Adu container
              ConstructAndStartAdu (requestAdu, responseAdu, m_userPageRequestGap, container);
            }
        }
    }
}

void
HttpClient::ConstructAndStartAdu (ADU requestAdu, ADU responseAdu, double requestPageGap, AduContainer & container)
{
  NS_LOG_FUNCTION (this);
  /// This is the first page in a session
  container.objects.push_back (requestAdu);
  container.objects.push_back (responseAdu);
  /// We only care about the response adu size for result recording
  m_tempPageSize += responseAdu.size;

  // Here we record the ADU we generated for later validation of generation function
  RecordParameter (requestAdu, responseAdu);

  if (m_lastObject)
    {
      m_lastObject = false;
      container.pages.push_back (container.objects);
      container.objects.clear ();
      container.totalPageSize.push_back (m_tempPageSize);
      container.requestPageGaps.push_back (requestPageGap);
      RecordPageGapTime (requestPageGap);
      m_tempPageSize = 0;
    }

  if (m_lastItem)
    {
      m_lastItem = false;
      StartAduContainer (container);
    }
}

void
HttpClient::PrintContainer (const AduContainer & container)
{
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

void
HttpClient::StartAduContainer (const AduContainer & container)
{
  NS_LOG_FUNCTION_NOARGS ();

  /// Since we just start the adu container, so this is the first web page
  FirstObject (m_socket, container, true);

  // Schedule a timeout for each web page transmission
  m_timeoutEvent = Simulator::Schedule (Seconds (m_pageTimeout), &HttpClient::CallTimeOut, this, m_socket);
}

} // namespace http
} // namespace ns3
