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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/point-to-point-module.h"

#include "ns3/http-module.h"

NS_LOG_COMPONENT_DEFINE ("HttpTest");

using namespace ns3;

int main (int argc, char *argv[])
{
  //
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //
#if 0
  LogComponentEnable ("HttpClient", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpServer", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpDistribution", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpController", LOG_LEVEL_ALL);
#endif

  LogComponentEnable ("HttpTest", LOG_LEVEL_ALL);

  uint32_t totalTime = 1000;
  uint32_t dataStart = 30;
  uint32_t nNodes = 2;
  std::string delay = "2ms";
  std::string dataRate = "1Mbps";

  // Allow users to override the default parameters and set it to new ones from CommandLine.
  CommandLine cmd;
  /*
   * The parameter for the p2p link
   */
  cmd.AddValue ("DataRate", "The data rate for the link", dataRate);
  cmd.AddValue ("Delay", "The delay for the link", delay);
  cmd.Parse (argc, argv);

  SeedManager::SetSeed (99);
  SeedManager::SetRun (1);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (nNodes);
  NetDeviceContainer devices;

  NS_LOG_INFO ("Create channels.");
  // We create the channels first without any IP addressing information
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (dataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (delay));
  devices = p2p.Install (nodes);

  InternetStackHelper internet;
  internet.Install (nodes);

  NS_LOG_INFO ("Setting the address");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces;
  interfaces = address.Assign (devices);

  double randomStartTime = (1.0 / 4) / (nNodes);

  uint16_t port = 80;
  uint32_t count = 0;

  HttpHelper httpHelper;

  for (uint32_t i = 0; i < (nNodes - 1); ++i)
    {
      HttpServerHelper httpServer;
      NS_LOG_LOGIC ("Install app in server");
      httpServer.SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), port)));
      httpServer.SetAttribute ("HttpController", PointerValue (httpHelper.GetController ()));
      ApplicationContainer serverApps = httpServer.Install (nodes.Get (i));

      serverApps.Start (Seconds (0.0));
      serverApps.Stop (Seconds (totalTime));

      HttpClientHelper httpClient;
      httpClient.SetAttribute ("Peer", AddressValue (InetSocketAddress (interfaces.GetAddress (i), port)));
      httpClient.SetAttribute ("HttpController", PointerValue (httpHelper.GetController ()));
      ApplicationContainer clientApps = httpClient.Install (nodes.Get (nNodes - 1));

      UniformVariable var;
      clientApps.Start (Seconds (dataStart + (count * randomStartTime)));
      clientApps.Stop (Seconds (totalTime));
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}
