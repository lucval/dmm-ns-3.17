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

#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include "ns3/nstime.h"
#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/socket.h"

#include "http-client.h"

using namespace std;

namespace ns3 {
namespace http {

/**
 * \brief Implements the HttpServer applications.
 *
 * This class deals with http server side operations, like receiving http request and
 * sending back http response.
 *
 * \see HttpHelper
 */
class HttpServer : public Application
{
public:
  static TypeId
  GetTypeId (void);

  /**
   * \brief Constructor.
   */
  HttpServer ();
  ~HttpServer ();

  void SetServerApplication ();
  /**
   * \brief Notify that the connection has requested
   * \param socket the socket we used for http connection
   * \param address the address
   */
  bool ConnectionRequested (Ptr<Socket> socket, const Address& address);
  /**
   * \brief Notify that the new connection has been created
   * \param socket the socket we used for http connection
   * \param address the address
   */
  void ConnectionAccepted (Ptr<Socket> socket, const Address& address);
  /**
   * \brief The server starts to receive packet from the client
   * \param socket the socket we used for http connection
   */
  void ServerReceive (Ptr<Socket> socket);
  /**
   * \brief Find the first in-line request ADU size
   * \param requestSize The request ADU size
   */
  uint32_t FindFirstRequestSize (AduContainer & serverContainer);
  /**
   * \brief Update the first in-line request ADU size
   * \param requestSize The request ADU size to be updated to
   */
  void UpdateFirstRequestSize (AduContainer & serverContainer, uint32_t requestSize);
  /**
   * \brief Remove the first in-line request ADU
   */
  void RemoveFirstRequestSize (AduContainer & serverContainer);
  /**
   * Set and get the http controller
   */
  void SetController (Ptr<HttpController> r);
  Ptr<HttpController> GetController () const;
  void PrintContainer (const AduContainer & container);

private:
  /**
   * \brief Called at time specified by Start to start application
   */
  virtual void StartApplication (void);
  /**
   * \brief Called at time specified by Stop to terminate application
   */
  virtual void StopApplication (void);

  bool                          m_firstConnection;              ///< The first connection or not
  bool                          m_persistent;                   ///< The persistent connection or not
  Address                       m_local;                        ///< Address where this HttpServer application is running.
  Ptr<HttpController>           m_controller;                   ///< The pointer to http controller
  Ptr<Socket>                   m_socket;                       ///< The socket to work with
  uint32_t                      m_pageTimeout;                  ///< The page time out value
  uint32_t                      m_run;                          ///< The number of simulation runs
  TypeId                        m_tid;                          ///< The transport protocol id to pass in (default. tcp)
};

} // namespace ns3
} // namespace http

#endif  /* HTTP_SERVER */
