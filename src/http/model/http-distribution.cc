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

#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/random-variable.h"
#include "ns3/rng-stream.h"
#include "ns3/object.h"
#include "ns3/nstime.h"

#include "http-distribution.h"
#include "http-random-variable.h"

NS_LOG_COMPONENT_DEFINE ("HttpDistribution");

//-------------------------------------------------------------
//-------------------------------------------------------------
//Http Distribution
namespace ns3 {
namespace http {

NS_OBJECT_ENSURE_REGISTERED (HttpDistribution);

HttpDistribution::HttpDistribution ()
  : m_rate (0),
    m_requestSize (NULL),
    m_responseSize (NULL),
    m_numberPages (NULL),
    m_singleObject (NULL),
    m_objectsPerPage (NULL),
    m_timeBtwnPages (NULL),
    m_timeBtwnObjs (NULL),
    m_serverDelay (NULL)
{
  Start ();
}

TypeId
HttpDistribution::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::http::HttpDistribution")
    .SetParent<Object> ()
    .AddConstructor<HttpDistribution> ()
  ;
  return tid;
}

HttpDistribution::~HttpDistribution ()
{
  NS_LOG_FUNCTION_NOARGS ();
  // delete RngStreams and Random Variables
  Cleanup ();
}

double HttpDistribution::GetRate ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_rate;
}

uint32_t HttpDistribution::GetRequestSize ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return (uint32_t) (m_requestSize->GetValue ());
}

uint32_t HttpDistribution::GetResponseSize ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return (uint32_t) (m_responseSize->GetValue ());
}

double HttpDistribution::GetServerDelay ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_serverDelay->GetValue ();
}

uint32_t HttpDistribution::GetNumberPages ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return (uint32_t) ceil (m_numberPages->GetValue ());
}

uint32_t HttpDistribution::GetNumObjects (uint32_t pages)
{
  NS_LOG_FUNCTION_NOARGS ();
  uint32_t p_singleobj = 0;
  uint32_t objs = 1;

  if (pages > 1)
    {
      // find the probability when there's only one object in this page
      p_singleobj = (uint32_t) m_singleObject->GetValue ();
    }
  if (p_singleobj == 0)
    {
      objs = (uint32_t) ceil (m_objectsPerPage->GetValue ());
      if (objs == 1)
        {
          // should be at least 2 objects at this point
          objs++;
        }
    }
  return objs;
}

double HttpDistribution::GetRequestGap (uint32_t page, uint32_t object)
{
  NS_LOG_FUNCTION_NOARGS ();
  double val;

  if (page == 0 && object == 0)
    {
      // first request
      val = 0;
    }
  else if (page != 0 && object == 0)
    {
      // main page (between-page requests)
      val = m_timeBtwnPages->GetValue ();
    }
  else
    {
      // embedded objects (within-page requests)
      val = m_timeBtwnObjs->GetValue ();
    }
  return val;
}

void HttpDistribution::Start ()
{
  NS_LOG_FUNCTION_NOARGS ();
  // Initialize HttpDistribution random variables
  if (m_requestSize == NULL)
    {

      m_requestSize = (HttpFileSizeRandomVariableImpl*) new
        HttpFileSizeRandomVariableImpl (m_rate, HTTP_REQUEST_SIZE);

      NS_LOG_DEBUG ("Created ReqSize RNG and RV");
    }
  if (m_responseSize == NULL)
    {
      m_responseSize = (HttpFileSizeRandomVariableImpl*) new
        HttpFileSizeRandomVariableImpl (m_rate,
                                        HTTP_RESPONSE_SIZE);

      NS_LOG_DEBUG ("Created RspSize RNG and Random Variable");
    }
  if (m_serverDelay == NULL)
    {
      m_serverDelay = (HttpServerDelayRandomVariableImpl*) new
        HttpServerDelayRandomVariableImpl
          (HttpServerDelayRandomVariableImpl::SERVER_DELAY_SHAPE,
          HttpServerDelayRandomVariableImpl::SERVER_DELAY_SCALE);

      NS_LOG_DEBUG ("Created ServerDelay RNG and Random Variable");
    }
  if (m_numberPages == NULL)
    {
      m_numberPages = (HttpNumPagesRandomVariableImpl*) new
        HttpNumPagesRandomVariableImpl
          (HttpNumPagesRandomVariableImpl::P_1PAGE,
          HttpNumPagesRandomVariableImpl::SHAPE_NPAGE,
          HttpNumPagesRandomVariableImpl::SCALE_NPAGE);

      NS_LOG_DEBUG ("Created Number of Pages RNG and Random Variable");
    }
  if (m_singleObject == NULL)
    {
      m_singleObject = (HttpSingleObjRandomVariableImpl*) new
        HttpSingleObjRandomVariableImpl
          (HttpSingleObjRandomVariableImpl::P_1TRANSFER);

      NS_LOG_DEBUG ("Created Single Objects RNG and Random Variable");
    }
  if (m_objectsPerPage == NULL)
    {
      m_objectsPerPage = (HttpObjsPerPageRandomVariableImpl*) new
        HttpObjsPerPageRandomVariableImpl
          (HttpObjsPerPageRandomVariableImpl::SHAPE_NTRANSFER,
          HttpObjsPerPageRandomVariableImpl::SCALE_NTRANSFER);

      NS_LOG_DEBUG ("Created Objects Per Page RNG and Random Variable");
    }
  if (m_timeBtwnPages == NULL)
    {
      m_timeBtwnPages = (HttpTimeBtwnPagesRandomVariableImpl*) new
        HttpTimeBtwnPagesRandomVariableImpl ();

      NS_LOG_DEBUG ("Created Time Btwn Pages RNG and Random Variable");
    }
  if (m_timeBtwnObjs == NULL)
    {
      m_timeBtwnObjs = (HttpTimeBtwnObjsRandomVariableImpl*) new
        HttpTimeBtwnObjsRandomVariableImpl ();

      NS_LOG_DEBUG ("Created Time Btwn Objs RNG and Random Variable");
    }
}

void HttpDistribution::Stop ()
{
  NS_LOG_FUNCTION_NOARGS ();
  /// delete all variables
  if (m_requestSize != NULL)
    {
      delete m_requestSize;
    }
  if (m_responseSize != NULL)
    {
      delete m_responseSize;
    }
  if (m_serverDelay != NULL)
    {
      delete m_serverDelay;
    }
  if (m_numberPages != NULL)
    {
      delete m_numberPages;
    }
  if (m_singleObject != NULL)
    {
      delete m_singleObject;
    }
  if (m_objectsPerPage != NULL)
    {
      delete m_objectsPerPage;
    }
  if (m_timeBtwnPages != NULL)
    {
      delete m_timeBtwnPages;
    }
  if (m_timeBtwnObjs != NULL)
    {
      delete m_timeBtwnObjs;
    }
}
} //namespace http
} //namespace ns3
