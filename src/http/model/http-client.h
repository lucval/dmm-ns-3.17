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

#ifndef HTTP_CLIENT
#define HTTP_CLIENT

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include "http-distribution.h"
#include "ns3/nstime.h"
#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "http-controller.h"
#include "http-server.h"

#include "ns3/ipv4-interface.h"
#include "ns3/ipv4.h"

using namespace std;

namespace ns3 {

class HttpServer;

namespace http {

/**
 * \brief Implements the HttpClient client and server applications.
 *
 * It is difficult and probably unnecessary to use this class
 * directly; see TmixHelper or TmixTopology instead.
 *
 * \see HttpHelper
 */
class HttpClient : public Application
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * \brief Constructor.
   */
  HttpClient ();
  /**
   * \brief Destructor.
   */
  ~HttpClient ();

  void StartNewSocket ();

  void StartFirstWebSession ();

  void SetClientApplication ();
  /**
   * \brief Notify the completion of connection
   * \param socket the socket that get connected
   */
  void ConnectionComplete (Ptr<Socket> socket);
  /**
   * \brief Notify the failure of connection
   * \param socket the socket that failed the connected
   */
  void ConnectionFailed (Ptr<Socket> socket);
  /**
   * \brief Schedule a adu container for immediate execution.
   *
   * \param container The ADU container to start sending
   */
  void StartAduContainer (const AduContainer & container);
  /**
   * \brief Record the web page gap time.
   * \param pageGapTime the gap time among web pages
   */
  void RecordPageGapTime (double pageGapTime);
  /**
   * \brief Send out the first object for one web page
   * \param socket The socket for this connection
   * \param container The ADU container
   */
  void FirstObject (Ptr<Socket> socket, const AduContainer & container, bool firstPage);
  /**
   * \brief Call the time out function and time out the ongoing web page and record the results
   * \param socket The socket for the connection
   */
  void CallTimeOut (Ptr<Socket> socket);
//  void TestAduGeneration ();
/**
 * \brief Generate the Adu containers, either with Internet mode or transaction mode.
 *
 * with the Internet mode, based on the distribution function from "http-random-variable.cc/h",
 * it generates http traffic parameters that match Internet traffic characteristics.
 */
  void AduGeneration ();
  /**
   * \brief Schedule an adu container for immediate execution.
   *
   * You must specify the TCP port it will connect to listen on; make sure it won't be in use.
   * \param requestAdu request ADU for HTTP request
   * \param responseAdu response AUD for HTTP response
   * \param requestPageGap the gap time among web pages
   * \param container ADU container to execute immediately.
   */
  void ConstructAndStartAdu (ADU requestAdu, ADU responseADU, double requestPageGap, AduContainer & container);
  /**
   * \brief Set the file name for recording object deliver ratio and page delay
   * \param filename the file name for output
   */
  void SetFileName (string filename);
  /**
   * \brief Record the parameter for the Internet mode traffic
   * \param requestAdu The web request ADU
   * \param responseAdu The web response ADU
   */
  void RecordParameter (ADU requestAdu, ADU responseAdu);
  /**
   * \brief Record the results after finishing each web page
   * \param delay the delay to load each individual web page, one page may include multiple web objects
   * \param odr the object delivery ratio for the web page
   */
  void RecordResults (double delay, double odr);
  /**
   * \brief Record the results after finishing each web page
   * \param socket the socket that sending the web page
   */
  void FinishOnePage (Ptr<Socket> socket);
  /**
   * \brief It deals with web client receiving web pages
   * \param socket the socket that sending the web page
   */
  void ClientReceive (Ptr<Socket> socket);
  /**
   * \brief Notify the completion of web sessions
   */
  void SessionCompleted ();
  /**
   * \brief Set and get the http controller
   */
  void SetController (Ptr<HttpController> r);
  Ptr<HttpController> GetController () const;
  /**
   * Print the container adu sizes
   * \param container the adu container used in the simulation
   */
  void PrintContainer (const AduContainer & container);

private:
  /**
   * Call at time when starting application, inherited from application
   */
  virtual void StartApplication (void);
  /**
   * Called at time specified by Stop to terminate application, inherited from application
   */
  virtual void StopApplication (void);

  class PortAllocator : public SimpleRefCount<PortAllocator>
  {
public:
    PortAllocator ();

    uint16_t
    AllocatePort ();

    void
    DeallocatePort (uint16_t port);

private:
    vector<uint16_t> m_availablePorts;          ///< Min-heap of available ports. Starts out empty and grows as needed.
    int m_topPort;                              ///< Next port number to add if m_availablePorts is empty.
  };

  bool                  m_realFirstObject;              ///< Whether this is the first transport connection or not
  HttpDistribution      m_distribution;                 ///< The http distribution object
  Address               m_local;                        ///< Address where this HttpClient application is running.
  Address               m_peer;                         ///< Address where the corresponding HttpClient application is running.
  bool                  m_internet;                     ///< The working mode internet/transaction
  vector<uint32_t>      m_objectsPerPage;               ///< The objects per page
  bool                  m_lastObject;                   ///< The last object in one page
  bool                  m_lastItem;                     ///< The last page in one container
  Ptr<PortAllocator>    m_portAllocator;                ///< The port allocator
  Ptr<HttpController>   m_controller;                   ///< the pointer for http controller
  Ptr<Socket>           m_socket;                       ///< Associated socket
  TypeId                m_tid;                          ///< The transport protocol id to pass in (default. tcp)
  std::string           m_routing;                      ///< The string of routing protocol
  std::string           m_transport;                    ///< The string of transport protocol
  uint32_t              m_sessionNumber;                ///< Record the session number for this open socket
  uint32_t              m_maxSessions;                  ///< The maximum session number to run for the simulation
  bool                  m_pipelining;                   ///< The boolean value for pipelining or not
  bool                  m_persistent;                   ///< The boolean value for persistent connection or not
  EventId               m_timeoutEvent;                 ///< Event id to schedule timeout
  uint32_t              m_pageTimeout;                  ///< The page time out value
  uint32_t              m_nodeId;                       ///< The client node id
  uint32_t              m_run;                          ///< The number of simulation runs
  uint32_t              m_tempPageSize;                 ///< Temporary record the page size
  ostringstream         ClientFile1;                    ///< Record object delivery ratio
  ostringstream         ClientFile2;                    ///< Record the delay
  ostringstream         ClientFile3;                    ///< Record the distribution generated parameters
  ostringstream         ClientFile4;                    ///< Record the web page request gap
  /**
   * The model uses the request/response size for all the objects when using the transaction mode
   */
  uint32_t              m_userRequestSize;              ///< The user defined request size
  uint32_t              m_userResponseSize;             ///< The user defined response size
  double                m_userServerDelay;              ///< The user defined server delay
  double                m_userPageRequestGap;           ///< The user defined request gap between web page
  double                m_userObjectRequestGap;         ///< The user defined request gap between web object in one page
  uint32_t              m_userNumPages;                    ///< The user defined number of pages
  uint32_t              m_userNumObjects;                  ///< The user defined number of objects per page
};

} // namespace ns3
} // namespace http

#endif
