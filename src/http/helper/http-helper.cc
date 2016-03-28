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

#include "http-helper.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/names.h"
#include <iostream>

namespace ns3 {

HttpHelper::HttpHelper ()
{
  m_factory.SetTypeId (http::HttpController::GetTypeId ());
  m_controller = m_factory.Create<http::HttpController> ();
}

void
HttpHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

Ptr<http::HttpController>
HttpHelper::GetController (void)
{
  return m_controller;
}

//------------------------------------------------------------------------------------------------------
HttpServerHelper::HttpServerHelper ()
{
  m_factory.SetTypeId (http::HttpServer::GetTypeId ());
}

void
HttpServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
HttpServerHelper::Install (Ptr<Node> node)
{
  ApplicationContainer apps;
  Ptr<http::HttpServer> server = m_factory.Create<http::HttpServer> ();
  node->AddApplication (server);
  apps.Add (server);
  return apps;
}

ApplicationContainer
HttpServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      Ptr<http::HttpServer> server = m_factory.Create<http::HttpServer> ();
      node->AddApplication (server);
      apps.Add (server);
    }
  return apps;
}

//------------------------------------------------------------------------------------------------------
HttpClientHelper::HttpClientHelper ()
{
  m_factory.SetTypeId (http::HttpClient::GetTypeId ());
}

void
HttpClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
HttpClientHelper::Install (Ptr<Node> node)
{
  ApplicationContainer apps;
  m_client = m_factory.Create<http::HttpClient> ();
  node->AddApplication (m_client);
  apps.Add (m_client);
  return apps;
}

ApplicationContainer
HttpClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      m_client = m_factory.Create<http::HttpClient> ();
      node->AddApplication (m_client);
      apps.Add (m_client);
    }
  return apps;
}

Ptr<http::HttpClient>
HttpClientHelper::GetClient (void)
{
  return m_client;
}

} // namespace ns3
