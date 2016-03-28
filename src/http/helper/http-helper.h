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

#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include <stdint.h>
#include <vector>
#include <string>

#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/trace-helper.h"
#include "ns3/http-client.h"
#include "ns3/http-server.h"
#include "ns3/http-controller.h"


namespace ns3 {

/**
 * \brief Create a server application which waits for input http packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class HttpHelper
{
public:
  /**
   * Create HttpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  HttpHelper ();
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  Ptr<http::HttpController> GetController (void);
private:
  ObjectFactory m_factory;
  Ptr<http::HttpController> m_controller;
};

/**
 * \brief Create a server application which waits for input http packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class HttpServerHelper
{
public:
  /**
   * Create HttpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  HttpServerHelper ();
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  /**
   * Create one http server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param node The node on which to create the Applications.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (Ptr<Node> node);
  /**
   * Create one udp server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c);
private:
  ObjectFactory m_factory;
  HttpHelper httpHelper;
};

/**
 * \brief Create a client application which sends udp packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class HttpClientHelper
{

public:
  /**
   * Create HttpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   */
  HttpClientHelper ();
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  /**
   * Create one http client application on each of the Nodes in the
   * NodeContainer.
   *
   * \param node The node on which to create the Applications.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (Ptr<Node> node);
  /**
     * \param c the nodes
     *
     * Create one udp client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
  ApplicationContainer Install (NodeContainer c);
  Ptr<http::HttpClient> GetClient (void);

private:
  ObjectFactory m_factory;
  Ptr<http::HttpClient> m_client;
  HttpHelper httpHelper;
};

} // namespace ns3

#endif /* HTTP_HELPER_H */
