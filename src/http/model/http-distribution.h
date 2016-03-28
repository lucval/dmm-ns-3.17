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
 *
 * Notification:
 * Statistics Research, Bell Labs, Lucent Technologies and The University of North Carolina at Chapel Hill
 */

#ifndef Http_DISTRIBUTION_H
#define Http_DISTRIBUTION_H

#include <string>
#include <stack>
#include <queue>
#include <map>

#include "ns3/boolean.h"
#include "ns3/application.h"
#include "ns3/node.h"
#include "ns3/rng-stream.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/random-variable.h"
#include "ns3/object.h"

#include "http-random-variable.h"


namespace ns3 {
namespace http {

class HttpDistribution : public Object
{
public:
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Constructor.
   */
  HttpDistribution ();
  /**
   * \brief Destructor.
   */
  virtual ~HttpDistribution ();

  ///\name Fields
  //\{
  double GetRate ();
  uint32_t GetRequestSize ();
  uint32_t GetResponseSize ();
  double GetServerDelay ();
  uint32_t GetNumberPages ();
  uint32_t GetNumObjects (uint32_t pages);
  //\}

  /**
   * \brief This function will determine the gap time type: e.g. first request, time between requests in
   *        one session, time between different sessions, and then get the request gap time accordingly
   * \param page The web page to work with
   * \param object The object within the web page
   * \return double the request gap time in double value
   */
  double GetRequestGap (uint32_t page, uint32_t object);

protected:
  /**
   * \brief Initialize the random variable functions
   */
  void Start ();
  /**
   * \brief stop the random variable functions and do cleanup
   */
  void Stop ();

private:
  double m_rate;
  /**
   * \brief statistics objects
   *
   * For detailed documentation, check the http-random-variable.cc/h files
   */
  HttpFileSizeRandomVariableImpl* m_requestSize;
  HttpFileSizeRandomVariableImpl* m_responseSize;
  HttpNumPagesRandomVariableImpl* m_numberPages;
  HttpSingleObjRandomVariableImpl* m_singleObject;
  HttpObjsPerPageRandomVariableImpl* m_objectsPerPage;
  HttpTimeBtwnPagesRandomVariableImpl* m_timeBtwnPages;
  HttpTimeBtwnObjsRandomVariableImpl* m_timeBtwnObjs;
  HttpServerDelayRandomVariableImpl* m_serverDelay;
};

} //namespace http
} //namespace ns3
#endif
