#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/bridge-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/topology-read-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/general-udp-client-server-helper.h"
#include "ns3/http-module.h"
#include "ns3/openflow-module.h"

using namespace ns3;

void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

double
TriggerHO (uint32_t radius, double speed){
    double time = (9*radius)/(2*sqrt(3)*speed);
    //time = time-1600;
    //while(time>550){
     //   time = time-550;
    //}
    //time = time/100;
    //if(time<1) time = time+1;
    return time;
}

double
VpDelay (uint16_t choice)
{
    switch(choice){
        case 128:
        {
            return 4.46;
        }
        case 256:
       	{
            return 5.65;
       	}
        case 512:
        {
            return 9.03;
        }
        case 1024:
       	{
            return 19.71;
        }
        case 2048:
        {
            return 56.8;
        }
       	case 4096:
        {
            return 193.9;
        }
        case 8192:
        {
            return 719.7;
       	}
        default:
        {
            return 19.7;
        }
    }
}

void
InstallBgApplication(Ptr<Node> left, Ptr<Node> right, uint16_t rate, bool bgActive)
{
    if(!bgActive) return;
    switch(rate){
        case 1:
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstArrivals", RandomVariableValue (ConstantVariable (2000)));
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstTimeLength", RandomVariableValue (ConstantVariable (0.02)));
            Config::SetDefault ("ns3::PPBPApplication::BurstIntensity", DataRateValue (DataRate ("20Mb/s")));
            break;
        case 10:
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstArrivals", RandomVariableValue (ConstantVariable (2000)));
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstTimeLength", RandomVariableValue (ConstantVariable (0.02)));
            Config::SetDefault ("ns3::PPBPApplication::BurstIntensity", DataRateValue (DataRate ("200Mb/s")));
            break;
        case 100:
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstArrivals", RandomVariableValue (ConstantVariable (2000)));
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstTimeLength", RandomVariableValue (ConstantVariable (0.02)));
            Config::SetDefault ("ns3::PPBPApplication::BurstIntensity", DataRateValue (DataRate ("2Gb/s")));
            break;
        default:
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstArrivals", RandomVariableValue (ConstantVariable (2000)));
            Config::SetDefault ("ns3::PPBPApplication::MeanBurstTimeLength", RandomVariableValue (ConstantVariable (0.02)));
            Config::SetDefault ("ns3::PPBPApplication::BurstIntensity", DataRateValue (DataRate ("20Mb/s")));
    }

    Ipv4Address la = left->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

    PPBPHelper clientHelper ("ns3::UdpSocketFactory", Address ());
    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), 1001));
    PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApps;
    sinkApps.Add (packetSinkHelper.Install (left));
    sinkApps.Start (Seconds (0.0));

    // Create an on/off app sending packets to the left side
    ApplicationContainer clientApps;
    AddressValue remoteAddress (InetSocketAddress (la, 1001));
    clientHelper.SetAttribute ("Remote", remoteAddress);
    clientApps.Add (clientHelper.Install (right));
    clientApps.Start (Seconds (0.1)); // Start 0.1 second after sink
}

void
SetupRouting(Ptr<Node> src, Ptr<Node> dst, int32_t iface)
{
    Ipv4StaticRoutingHelper srh;
    Ptr<Ipv4StaticRouting> staticR = srh.GetStaticRouting(src->GetObject<Ipv4>());
    staticR->AddHostRouteTo(dst->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), iface);
}

/**
 * Sample simulation script for an handover with P-GW relocation
 * It instantiates two sub-EPC entities, each one with a P-GW/S-GW and one eNBs.
 * It attaches one UE to the 'source' sub-EPC and triggers a handover of the UE towards the 'target' sub-EPC.
 */
NS_LOG_COMPONENT_DEFINE ("Ṕartial OpenFlow");

int
main (int argc, char *argv[])
{
  /** ***************** COMMAND LINE ARGUEMENT ***************** **/
  double simTime = 20;
  double astart = 10.4;
  double astop = 19.5;
  double distance = 2000;
  uint16_t numberOfUesEpc1 = 1;
  uint16_t numberOfUesEpc2 = 0;
  uint32_t radius = 50;
  int hopsInEg = 1;
  uint16_t controllerPos = 0;
  bool singleMme = true;
  uint16_t migration = 128;
  uint16_t numberOfCNs = 3;
  bool bgActive = false;
  uint16_t timer = 10;
  std::string ascii = "traces/ascii.tr";
  std::string xml = "traces/result.xml";
  std::string version = "traces/00";
  uint32_t seed = 1;

  CommandLine cmd;
  cmd.AddValue("simTime", "Total duration of the simulation (in sec) [default=6] ",simTime);
  cmd.AddValue("distance", "Radius of eNodeB (in km). 2 times the radius equals the distance between source and target eNB [default=1000] ",distance);
  cmd.AddValue("numberOfUesEpc1", "UEs attached to the source eNodeB (note: 20% of them will handover) [default=70] ",numberOfUesEpc1);
  cmd.AddValue("numberOfUesEpc2", "UEs attached to the target eNodeB (note: the upcoming UEs should also be counted here) [default=70] ",numberOfUesEpc2);
  cmd.AddValue("radius", "S-PGW serving area radius (in meters) [default=50000] ",radius);
  cmd.AddValue("hopsInEg", "Avg distance between ingress and egress NAT routers (in number of hops) [default=1] ",hopsInEg);
  cmd.AddValue("controllerPos", "Position of the OF controller: 0 = middle of operator's OF backbone network; 1 = close to sourceMME; 2 = close to targetMME  [default=0] ",controllerPos);
  cmd.AddValue("singleMme", "If true (1) one MME shared between source and target S/PGWs, handover with MME relocation otherwise (0) [default=1] ",singleMme);
  cmd.AddValue("migration", "Size of the migrated component (in MB). 0 = no migration [default=0] ",migration);
  cmd.AddValue("numberOfCNs", "Number of CNs attached to the Internet network [default=10] ",numberOfCNs);
  cmd.AddValue("bgActive", "70% background traffic in the network; 0=false, 1=true [default=0] ",bgActive);
  cmd.AddValue("timer", "MME timer to delete session at the source S-/P-GW (in ms)  [default=10] ",timer);
  cmd.AddValue("ascii", "Name of the ASCII tracer output file [default=traces/ascii.tr] ",ascii);
  cmd.AddValue("xml", "Name of the flow monitor XML output file [default=traces/result.xml] ",xml);
  cmd.AddValue("version", "Prefix of CSV output file [default=traces/00] ",version);
  cmd.AddValue("seed", "RNG seed [default=1] ",seed);
  cmd.Parse(argc, argv);

  /** ************ SETUP SIMULATION PARAMETERS ************ **/
  // seed
  RngSeedManager::SetSeed (seed);

  // eNodeB physical
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (49.0));
  Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (5.0));

  // eNodeB transmission mode
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue(4)); //MIMO multi-user

  // eNodeB bandwidth (UL and DL)
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (25)); // 25 RBs = 5MHz
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (25)); // 25 RBs = 5MHz
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (3100)); // 2655MHz
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (21100)); // 2535MHz

  // UE physical
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (26.0));
  Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (5.0));

  // lteHelper
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  Config::SetDefault ("ns3::PfFfMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));

  // X2 handover
  Config::SetDefault("ns3::LteEnbRrc::Timer", UintegerValue(timer));

  // Queue size
  Config::SetDefault ("ns3::DropTailQueue::Mode", StringValue ("QUEUE_MODE_BYTES"));

  // Link prop. delay.
  Config::SetDefault ("ns3::CsmaChannel::Delay", TimeValue (MilliSeconds (1)));

  // Global routing
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));

  // TCP application
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(90000000));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue(90000000));

  // Tracing
  Config::SetDefault("ns3::LteEnbRrc::Version", StringValue(version));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename", StringValue (version+"_Ul.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename", StringValue (version+"_Dl.txt"));

  NS_LOG_UNCOND("");
  NS_LOG_UNCOND("*** NETWORK SETUP ***");

  /** ************ CREATE BACKBONE NETWORK TOPOLOGY ************ **/
  std::string format ("Rocketfuel");
  std::string input;
  input = "src/topology-read/examples/Ebone_small_unweighted_toposample.txt";

  Ptr<TopologyReader> inFile = 0;
  TopologyReaderHelper topoHelp;

  NodeContainer backboneNodes;
  NodeContainer remoteHosts;
  NodeContainer n;

  topoHelp.SetFileName (input);
  topoHelp.SetFileType (format);
  inFile = topoHelp.GetTopologyReader ();

  if (inFile != 0)
    {
      backboneNodes = inFile->Read ();
    }

  if (inFile->LinksSize () == 0)
    {
      NS_LOG_ERROR ("Problems reading the topology file. Failing.");
      return -1;
    }

  NS_LOG_INFO ("creating internet stacks");
  InternetStackHelper stack;

  stack.Install (backboneNodes);

  NS_LOG_INFO ("creating ip4 addresses");
  Ipv4AddressHelper opNetAddress;
  opNetAddress.SetBase ("10.0.0.0", "255.255.0.0");
  Ipv4AddressHelper inetAddress;
  inetAddress.SetBase ("90.0.0.0", "255.255.255.0");

  int totlinks = inFile->LinksSize ();

  NS_LOG_INFO ("creating node containers");
  NodeContainer* nc = new NodeContainer[totlinks];
  TopologyReader::ConstLinksIterator iter;
  int i = 0;
  for ( iter = inFile->LinksBegin (); iter != inFile->LinksEnd (); iter++, i++ )
    {
      nc[i] = NodeContainer (iter->GetFromNode (), iter->GetToNode ());
    }

  NS_LOG_INFO ("creating net device containers");
  NetDeviceContainer* ndc = new NetDeviceContainer[totlinks];
  CsmaHelper csmah;
  csmah.SetDeviceAttribute("Mtu", UintegerValue (1500));
  for (int i = 0; i < totlinks; i++)
    {
        if(i<10){
            csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
            Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (31250000));
        }
        else if(i>40){
            csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
            Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (31250000));
        }
        else{
            csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
            Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (3125000));
        }

        ndc[i] = csmah.Install (nc[i]);
    }

  // it crates little subnets, one for each couple of nodes and the backgroud traffic.
  NS_LOG_INFO ("creating ipv4 interfaces");
  Ipv4InterfaceContainer* ipic = new Ipv4InterfaceContainer[totlinks];
  for (int i = 0; i < totlinks; i++)
    {
      if(i<27){
        ipic[i] = opNetAddress.Assign (ndc[i]);
        NS_LOG_UNCOND("addresses link " << i << " :" << ipic[i].GetAddress(0) << " ," << ipic[i].GetAddress(1));
        opNetAddress.NewNetwork ();
      }
      else {
        ipic[i] = inetAddress.Assign (ndc[i]);
        NS_LOG_UNCOND("addresses link " << i << " :" << ipic[i].GetAddress(0) << " ," << ipic[i].GetAddress(1));
        inetAddress.NewNetwork ();
      }
    }

  n.Add(backboneNodes);

  /** setup the S/P-GWs **/
  csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (3125000));
  Ptr<Node> sgiAR1 = backboneNodes.Get(1);
  Ptr<Node> sgiAR2 = backboneNodes.Get(8);
  Ptr<Node> sourceSPgw = CreateObject<Node>();
  Ptr<Node> targetSPgw = CreateObject<Node>();
  stack.Install(sourceSPgw);
  stack.Install(targetSPgw);
  NetDeviceContainer sourcePgwToSgi = csmah.Install (NodeContainer(sgiAR1, sourceSPgw));
  opNetAddress.Assign (sourcePgwToSgi); opNetAddress.NewNetwork();
  NetDeviceContainer targetPgwToSgi = csmah.Install (NodeContainer(sgiAR2, targetSPgw));
  opNetAddress.Assign (targetPgwToSgi); opNetAddress.NewNetwork();
  NS_LOG_UNCOND("source S/PGW id: "<<sourceSPgw->GetId ());
  NS_LOG_UNCOND("destination S/PGW id: "<<targetSPgw->GetId ());
  n.Add(sourceSPgw); n.Add(targetSPgw);

  /** setup the MMEs **/
  csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (3125000));
  Ptr<Node> sourceMme = CreateObject<Node>();
  Ptr<Node> targetMme = CreateObject<Node>();
  stack.Install(sourceMme);
  stack.Install(targetMme);
  //if(migration > 0)
    //csmah.SetChannelAttribute ("Delay", TimeValue (Seconds (VpDelay(migration))));
  NetDeviceContainer sourceMmeToPgw1 = csmah.Install (NodeContainer(sgiAR1, sourceMme));
  opNetAddress.Assign (sourceMmeToPgw1); opNetAddress.NewNetwork();
  csmah.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (1)));
  if(singleMme){
    NetDeviceContainer sourceMmeToPgw2 = csmah.Install (NodeContainer(sgiAR2, sourceMme));
    opNetAddress.Assign (sourceMmeToPgw2); opNetAddress.NewNetwork();
  }
  NetDeviceContainer targetMmeToPgw2 = csmah.Install (NodeContainer(backboneNodes.Get(20), targetMme)); // or SGi2??
  opNetAddress.Assign (targetMmeToPgw2); opNetAddress.NewNetwork();
  NS_LOG_UNCOND("source MME id: "<<sourceMme->GetId ());
  NS_LOG_UNCOND("target MME id: "<<targetMme->GetId ());
  n.Add(sourceMme); n.Add(targetMme);

  /** setup connection to the E-UTRAN **/
  csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (3125000));
  Ptr<Node> sourceEnb = CreateObject<Node>();
  Ptr<Node> targetEnb = CreateObject<Node>();
  stack.Install(sourceEnb);
  stack.Install(targetEnb);
  NetDeviceContainer sourceEnbToBb = csmah.Install (NodeContainer(backboneNodes.Get(13), sourceEnb));
  opNetAddress.Assign (sourceEnbToBb); opNetAddress.NewNetwork();
  NetDeviceContainer targetEnbToBb = csmah.Install (NodeContainer(backboneNodes.Get(19), targetEnb));
  opNetAddress.Assign (targetEnbToBb); opNetAddress.NewNetwork();
  if(migration > 0)
  {
    csmah.SetChannelAttribute ("Delay", TimeValue (Seconds (VpDelay(migration)/2)));
    NetDeviceContainer x2Link = csmah.Install (NodeContainer(sourceEnb, targetEnb));
    opNetAddress.Assign (x2Link); opNetAddress.NewNetwork();
    csmah.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (1)));
  }
  NS_LOG_UNCOND("source eNB id: "<<sourceEnb->GetId ());
  NS_LOG_UNCOND("target eNB id: "<<targetEnb->GetId ());
  n.Add(sourceEnb); n.Add(targetEnb);

  /** setup the OF controller **/
  csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (3125000));
  Ptr<Node> pos = CreateObject<Node>();
  Ptr<Node> controller = CreateObject<Node>();
  switch(controllerPos){
    case 1: {
        controller = sourceMme;
    }
    break;
    case 2: {
        controller = targetMme;
    }
    break;
    case 3: {
        pos = backboneNodes.Get(5);
        stack.Install(controller);
        NetDeviceContainer posToContr = csmah.Install (NodeContainer(pos, controller));
        inetAddress.Assign (posToContr); inetAddress.NewNetwork();
    }
    break;
    case 4: {
        pos = backboneNodes.Get(27);
        stack.Install(controller);
        NetDeviceContainer posToContr = csmah.Install (NodeContainer(pos, controller));
        inetAddress.Assign (posToContr); inetAddress.NewNetwork();
    }
    break;
    default: {
        pos = backboneNodes.Get(0);
        stack.Install(controller);
        NetDeviceContainer posToContr = csmah.Install (NodeContainer(pos, controller));
        inetAddress.Assign (posToContr); inetAddress.NewNetwork();
    }
    break;
  }
  NS_LOG_UNCOND("OF controller id: "<<controller->GetId());
  n.Add(controller);

  /** setup the remote hosts **/
  csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue (3125000));
  remoteHosts.Create(numberOfCNs);
  stack.Install(remoteHosts);
  Ptr<Node> pop = CreateObject<Node>();
  uint16_t j=0;
  for(uint16_t i=0; i<numberOfCNs; i++) {
    if(j==0){
        pop = backboneNodes.Get(24);
        j++;
    }
    else if(j==1){
        pop = backboneNodes.Get(32);
        j++;
    }
    else if(j==2){
        pop = backboneNodes.Get(33);
        j=0;
    }
    Ptr<Node> remoteHost = remoteHosts.Get(i);
    NetDeviceContainer internetDevices = csmah.Install (NodeContainer(pop, remoteHost));
    inetAddress.Assign (internetDevices); inetAddress.NewNetwork();
    NS_LOG_UNCOND("remote host "<<i<<" id: "<<remoteHost->GetId ());
    n.Add(remoteHost);
  }

  /** populate routing tables **/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /** Setup OpenFlow Switches **/
  OpenFlowSwitchHelper swtch;
  NetDeviceContainer ofDevs;

  Ptr<ns3::ofi::LearningController> ofController = CreateObject<ns3::ofi::LearningController> ();
  ofController->SetAttribute ("ExpirationTime", TimeValue (Seconds(0)));

  NodeContainer ofIngress, ofEgress, of;
  switch(hopsInEg){
    case 0: {
        ofIngress.Add(backboneNodes.Get(0));
        ofIngress.Add(backboneNodes.Get(4));
    }
    break;
    case 2: {
        ofIngress.Add(backboneNodes.Get(28));
        ofIngress.Add(backboneNodes.Get(34));
    }
    break;
    case 3: {
        ofIngress.Add(backboneNodes.Get(25));
        ofIngress.Add(backboneNodes.Get(21));
    }
    break;
    case 4:
    case 5:
    case 6: {
        ofIngress.Add(backboneNodes.Get(36));
        ofIngress.Add(backboneNodes.Get(40));
    }
    break;
    default: {
        ofIngress.Add(backboneNodes.Get(3));
        ofIngress.Add(backboneNodes.Get(6));
    }
    break;
  }

  ofEgress.Add(sgiAR1);
  ofEgress.Add(sgiAR2);

  of.Add(ofIngress);
  of.Add(ofEgress);

  for(uint16_t i=0; i<of.GetN(); i++){
    Ptr<Node> switchNode = of.Get(i);
    NetDeviceContainer switchDevices;
    for(uint16_t j=0; j<switchNode->GetNDevices(); j++){
        if(!(switchNode->GetDevice(j)->IsPointToPoint()))
            switchDevices.Add(switchNode->GetDevice(j));
    }
    ofDevs.Add(swtch.Install (switchNode, switchDevices, ofController));
  }

  /** ******************* SETUP ARP TABLE ****************** **/
  Ptr<ArpCache> arp = CreateObject<ArpCache>();
  arp->SetAliveTimeout(Seconds(3600 * 24 * 365));
  for(uint16_t i=0; i<n.GetN(); i++){
    Ptr<Ipv4L3Protocol> ip = n.Get(i)->GetObject<Ipv4L3Protocol>();
    NS_ASSERT(ip!=0);
    int ninter = (int)ip->GetNInterfaces();
    for(int j = 0; j < ninter; j++) {
      Ptr<Ipv4Interface> ipIface = ip->GetInterface(j);
      NS_ASSERT(ipIface != 0);
      Ptr<NetDevice> device = ipIface->GetDevice();
      NS_ASSERT(device != 0);
      Mac48Address addr = Mac48Address::ConvertFrom(device->GetAddress ());
      for(uint32_t k = 0; k < ipIface->GetNAddresses (); k ++) {
        Ipv4Address ipAddr = ipIface->GetAddress (k).GetLocal();
        if(ipAddr == Ipv4Address::GetLoopback())
           continue;
        ArpCache::Entry * entry = arp->Add(ipAddr);
        entry->MarkWaitReply(0);
        entry->MarkAlive(addr);
      }
    }
  }

  for(uint16_t i=0; i<n.GetN(); i++){
    Ptr<Ipv4L3Protocol> ip = n.Get(i)->GetObject<Ipv4L3Protocol>();
    NS_ASSERT(ip!=0);
    int ninter = (int)ip->GetNInterfaces();
    for(int j = 0; j < ninter; j++) {
      Ptr<Ipv4Interface> ipIface = ip->GetInterface(j);
      ipIface->SetArpCache(arp);
    }
  }

  /** ************ SETUP THE E-UTRAN ************ **/
  // Container of eNBs for EPC1 and EPC2
  NodeContainer enbNodesEpc1, enbNodesEpc2;
  enbNodesEpc1.Add (sourceEnb);
  enbNodesEpc2.Add (targetEnb);

  // Install Mobility
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector(0.0, 0.0, 0.0));
  positionAlloc->Add (Vector(distance * 2, 0, 0));
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodesEpc1);
  mobility.Install(enbNodesEpc2);

  /** ********************** FIRST EPC *********************** **/
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<EpcHelper> epcHelper1 = CreateObject<EpcHelper> (sourceSPgw);
  lteHelper->SetEpcHelper (epcHelper1);

  lteHelper->InitArp(arp, n);

  epcHelper1->InitOpenFlow(ofDevs, ofController);

  // Install LTE Devices in eNB
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodesEpc1);

  /* **************** TRAFFIC in RAN ************** */
  // Create Voice UEs (30% of the nodes)
  NodeContainer lteVoiceUeContainer, lteVoiceUeContainerHO;
  lteVoiceUeContainer.Create((float)0.1*numberOfUesEpc1);
  //lteVoiceUeContainerHO.Create((float)0.2*numberOfUesEpc1);
  lteVoiceUeContainerHO.Create(1);

  // Create Video UEs (20% of the nodes)
  NodeContainer lteVideoUeContainer;
  lteVideoUeContainer.Create((float)0.2*numberOfUesEpc1);

  // Create Gaming UEs (20% of the nodes)
  NodeContainer lteGamingUeContainer;
  lteGamingUeContainer.Create((float)0.2*numberOfUesEpc1);

  // Create HTTP UEs (20% of the nodes)
  NodeContainer lteHttpUeContainer;
  lteHttpUeContainer.Create((float)0.2*numberOfUesEpc1);

  // Create FTP UEs (the rest)
  NodeContainer lteFtpUeContainer;
  lteFtpUeContainer.Create((float)0.1*numberOfUesEpc1);

  // Position of background UEs attached to eNodeB
  MobilityHelper uemobility;
  uemobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
			"X", DoubleValue (0.0),
			"Y", DoubleValue (0.0),
			"rho", DoubleValue (distance));
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.Install(lteVoiceUeContainer);
  uemobility.Install(lteVideoUeContainer);
  uemobility.Install(lteGamingUeContainer);
  uemobility.Install(lteHttpUeContainer);
  uemobility.Install(lteFtpUeContainer);

  // Position of moving UEs attached to eNodeB
  MobilityHelper homobility;
  homobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> pAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < lteVoiceUeContainerHO.GetN(); i++)
    {
      pAlloc->Add (Vector(distance, 0, 0));
    }
  homobility.SetPositionAllocator(pAlloc);
  homobility.Install(lteVoiceUeContainerHO);

  // Install LTE Devices to the nodes
  NetDeviceContainer lteVoiceUeDevice = lteHelper->InstallUeDevice (lteVoiceUeContainer);
  NetDeviceContainer lteVoiceUeDeviceHO = lteHelper->InstallUeDevice (lteVoiceUeContainerHO);
  NetDeviceContainer lteVideoUeDevice = lteHelper->InstallUeDevice (lteVideoUeContainer);
  NetDeviceContainer lteGamingUeDevice = lteHelper->InstallUeDevice (lteGamingUeContainer);
  NetDeviceContainer lteHttpUeDevice = lteHelper->InstallUeDevice (lteHttpUeContainer);
  NetDeviceContainer lteFtpUeDevice = lteHelper->InstallUeDevice (lteFtpUeContainer);

  // Install the IP stack on the UEs
  stack.Install(lteVoiceUeContainer);
  stack.Install(lteVoiceUeContainerHO);
  stack.Install(lteVideoUeContainer);
  stack.Install(lteGamingUeContainer);
  stack.Install(lteHttpUeContainer);
  stack.Install(lteFtpUeContainer);

  for (uint16_t i = 0; i < lteVoiceUeContainer.GetN(); i++)
  {
		lteHelper->Attach (lteVoiceUeDevice.Get(i), enbLteDevs.Get(0));
  }

  for (uint16_t i = 0; i < lteVoiceUeContainerHO.GetN(); i++)
  {
		lteHelper->Attach (lteVoiceUeDeviceHO.Get(i), enbLteDevs.Get(0));
  }

  for (uint16_t i = 0; i < lteVideoUeContainer.GetN(); i++)
  {
		lteHelper->Attach (lteVideoUeDevice.Get(i), enbLteDevs.Get(0));
  }

  for (uint16_t i = 0; i < lteGamingUeContainer.GetN(); i++)
  {
		lteHelper->Attach (lteGamingUeDevice.Get(i), enbLteDevs.Get(0));
  }

  for (uint16_t i = 0; i < lteHttpUeContainer.GetN(); i++)
  {
		lteHelper->Attach (lteHttpUeDevice.Get(i), enbLteDevs.Get(0));
  }

  for (uint16_t i = 0; i < lteFtpUeContainer.GetN(); i++)
  {
		lteHelper->Attach (lteFtpUeDevice.Get(i), enbLteDevs.Get(0));
  }

    // VoIP Application
  uint16_t lteVoiceRemotePort = 4000;
  // Create one VoIPClient application to send UDP datagrams from UE nodes
  GeneralUdpClientHelper clientVoip ("3.0.0.0", lteVoiceRemotePort, 3);
  ApplicationContainer voipClient = clientVoip.Install (lteVoiceUeContainer);
  voipClient.Start (Seconds (0.1));

  // VoIP Application, both UL and DL for handovering UEs

  // UPLINK (from UEs). Create one udpServer applications per CN.
  GeneralUdpServerHelper lteVoiceRemoteServer (lteVoiceRemotePort, 3);
  ApplicationContainer lteVoiceRemoteApp = lteVoiceRemoteServer.Install (remoteHosts);
  lteVoiceRemoteApp.Start (Seconds (astart));
  lteVoiceRemoteApp.Stop (Seconds (astop));

  // Create one VoIPClient application to send UDP datagrams from UE nodes to Remote VoIP host.
  j=0;
  for(uint16_t i=0; i<lteVoiceUeContainerHO.GetN(); i++){
    if(j==numberOfCNs) j=0;
    GeneralUdpClientHelper clientVoipHO (remoteHosts.Get(j)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), lteVoiceRemotePort, 3);
    lteVoiceRemoteApp = clientVoipHO.Install (lteVoiceUeContainerHO.Get(i));
    lteVoiceRemoteApp.Start (Seconds (astart+0.1));
    lteVoiceRemoteApp.Stop (Seconds (astop));
    j++;
  }

  // DOWNLINK (to UEs). Create Voice applications on UE nodes.
  uint16_t lteVoiceUePort = 4000;
  GeneralUdpServerHelper lteVoiceUeServer (lteVoiceUePort, 3);
  ApplicationContainer lteVoiceUeApp = lteVoiceUeServer.Install (lteVoiceUeContainerHO);
  lteVoiceUeApp.Start (Seconds (astart));
  lteVoiceRemoteApp.Stop (Seconds (astop));

  // Create one VoIPClient application to send UDP datagrams from Remote Host to VoIP UEs.
  j=0;
  for (uint16_t i=0; i < lteVoiceUeContainerHO.GetN(); i++)
  {
    if(j==numberOfCNs) j=0;
    GeneralUdpClientHelper remoteClient (lteVoiceUeContainerHO.Get(i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), lteVoiceUePort, 3);
	lteVoiceRemoteApp = remoteClient.Install (remoteHosts.Get(j));
	lteVoiceRemoteApp.Start (Seconds (astart+0.1));
    lteVoiceRemoteApp.Stop (Seconds (astop));
	j++;
  }

  // Gaming Application
  uint16_t lteGamingRemotePort = 5000;
  // Create one Gaming application to send UDP datagrams from UE nodes
  GeneralUdpClientHelper gamingClientUe ("3.0.0.1", lteGamingRemotePort, 1);
  ApplicationContainer gamingClient = gamingClientUe.Install (lteGamingUeContainer);
  gamingClient.Start (Seconds (0.1));

  // Video Application
  uint16_t lteVideoRemotePort = 5000;
  // Create one Video application to send UDP datagrams from UE nodes
  GeneralUdpClientHelper VideoClientUe ("3.0.0.2", lteVideoRemotePort, 0);
  ApplicationContainer videoClient = VideoClientUe.Install (lteVideoUeContainer);
  videoClient.Start (Seconds (0.1));

  // FTP
  uint16_t portFTP = 9;  // well-known echo port number
  // Create one FTP application to send TCP packets from UE nodes
  BulkSendHelper sourceUe ("ns3::TcpSocketFactory", InetSocketAddress ("3.0.0.3", portFTP));
  ApplicationContainer sourceAppsUe = sourceUe.Install (lteFtpUeContainer);
  sourceAppsUe.Start (Seconds (0.1));

  // HTTP Client
  uint16_t portHttp = 80;   // well-known http port
  HttpHelper httpHelper;
  HttpClientHelper httpClient;
  // Create one HTTP application to send TCP packets from UE nodes
  httpClient.SetAttribute ("Peer", AddressValue (InetSocketAddress ("3.0.0.4", portHttp)));
  httpClient.SetAttribute ("HttpController", PointerValue (httpHelper.GetController ()));
  ApplicationContainer clientHttp = httpClient.Install (lteHttpUeContainer);
  clientHttp.Start (Seconds (0.2));

  /** ********************** SECOND EPC *********************** **/
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (3150)); // 2660MHz
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (21150)); // 2540MHz
  Ptr<EpcHelper> epcHelper2 = CreateObject<EpcHelper> (targetSPgw);
  lteHelper->SetEpcHelper (epcHelper2);

  epcHelper2->InitOpenFlow(ofDevs, ofController);

  // Install LTE Devices in eNB and UEs
  NetDeviceContainer enbLteDevs2 = lteHelper->InstallEnbDevice (enbNodesEpc2);

  /* **************** TRAFFIC in RAN ************** */
  // Create Voice UEs (30% of the nodes)
  NodeContainer lteVoiceUeContainer2;
  lteVoiceUeContainer2.Create((float)0.1*numberOfUesEpc2);

  // Create Video UEs (20% of the nodes)
  NodeContainer lteVideoUeContainer2;
  lteVideoUeContainer2.Create((float)0.2*numberOfUesEpc2);

  // Create Gaming UEs (20% of the nodes)
  NodeContainer lteGamingUeContainer2;
  lteGamingUeContainer2.Create((float)0.2*numberOfUesEpc2);

  // Create HTTP UEs (20% of the nodes)
  NodeContainer lteHttpUeContainer2;
  lteHttpUeContainer2.Create((float)0.2*numberOfUesEpc2);

  // Create FTP UEs (the rest)
  NodeContainer lteFtpUeContainer2;
  lteFtpUeContainer2.Create((float)0.1*numberOfUesEpc2);

  // Position of UEs attached to eNodeB
  MobilityHelper uemobility2;
  uemobility2.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
			"X", DoubleValue (distance * 2),
			"Y", DoubleValue (0.0),
			"rho", DoubleValue (distance));

  uemobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility2.Install(lteVoiceUeContainer2);
  uemobility2.Install(lteVideoUeContainer2);
  uemobility2.Install(lteGamingUeContainer2);
  uemobility2.Install(lteHttpUeContainer2);
  uemobility2.Install(lteFtpUeContainer2);

  // Install LTE Devices to the nodes
  NetDeviceContainer lteVoiceUeDevice2 = lteHelper->InstallUeDevice (lteVoiceUeContainer2);
  NetDeviceContainer lteVideoUeDevice2 = lteHelper->InstallUeDevice (lteVideoUeContainer2);
  NetDeviceContainer lteGamingUeDevice2 = lteHelper->InstallUeDevice (lteGamingUeContainer2);
  NetDeviceContainer lteHttpUeDevice2 = lteHelper->InstallUeDevice (lteHttpUeContainer2);
  NetDeviceContainer lteFtpUeDevice2 = lteHelper->InstallUeDevice (lteFtpUeContainer2);

  // Install the IP stack on the UEs
  stack.Install(lteVoiceUeContainer2);
  stack.Install(lteVideoUeContainer2);
  stack.Install(lteGamingUeContainer2);
  stack.Install(lteHttpUeContainer2);
  stack.Install(lteFtpUeContainer2);

  for (uint16_t i = 0; i < lteVoiceUeContainer2.GetN(); i++)
  {
		lteHelper->Attach (lteVoiceUeDevice2.Get(i), enbLteDevs2.Get(0));
  }

  for (uint16_t i = 0; i < lteVideoUeContainer2.GetN(); i++)
  {
		lteHelper->Attach (lteVideoUeDevice2.Get(i), enbLteDevs2.Get(0));
  }

  for (uint16_t i = 0; i < lteGamingUeContainer2.GetN(); i++)
  {
		lteHelper->Attach (lteGamingUeDevice2.Get(i), enbLteDevs2.Get(0));
  }

  for (uint16_t i = 0; i < lteHttpUeContainer2.GetN(); i++)
  {
		lteHelper->Attach (lteHttpUeDevice2.Get(i), enbLteDevs2.Get(0));
  }

  for (uint16_t i = 0; i < lteFtpUeContainer2.GetN(); i++)
  {
		lteHelper->Attach (lteFtpUeDevice2.Get(i), enbLteDevs2.Get(0));
  }

  // VoIP Application
  // Create one VoIPClient application to send UDP datagrams from UE nodes
  GeneralUdpClientHelper clientVoip2 ("3.0.0.5", lteVoiceRemotePort, 3);
  ApplicationContainer voipClient2 = clientVoip2.Install (lteVoiceUeContainer2);
  voipClient2.Start (Seconds (0.0));

  // Gaming Application
  // Create one Gaming application to send UDP datagrams from UE nodes
  GeneralUdpClientHelper gamingClientUe2 ("3.0.0.6", lteGamingRemotePort, 1);
  ApplicationContainer gamingClient2 = gamingClientUe2.Install (lteGamingUeContainer2);
  gamingClient2.Start (Seconds (0.1));

  // Video Application
  // Create one Video application to send UDP datagrams from UE nodes
  GeneralUdpClientHelper VideoClientUe2 ("3.0.0.7", lteVideoRemotePort, 0);
  ApplicationContainer videoClient2 = VideoClientUe2.Install (lteVideoUeContainer2);
  videoClient2.Start (Seconds (0.1));

  // FTP
  // Create one FTP application to send TCP packets from UE nodes
  BulkSendHelper sourceUe2 ("ns3::TcpSocketFactory", InetSocketAddress ("3.0.0.8", portFTP));
  ApplicationContainer sourceAppsUe2 = sourceUe2.Install (lteFtpUeContainer2);
  sourceAppsUe2.Start (Seconds (0.1));

  // HTTP Client
  HttpHelper httpHelper2;
  HttpClientHelper httpClient2;
  // Create one HTTP application to send TCP packets from UE nodes
  httpClient2.SetAttribute ("Peer", AddressValue (InetSocketAddress ("3.0.0.9", portHttp)));
  httpClient2.SetAttribute ("HttpController", PointerValue (httpHelper2.GetController ()));
  ApplicationContainer clientHttp2 = httpClient2.Install (lteHttpUeContainer2);
  clientHttp2.Start (Seconds (0.2));

  /** *************** IP ADDRESS CONTINUITY **************** **/
  Ipv4AddressHelper tunPool1 = epcHelper1->GetHelper();
  Ipv4AddressHelper tunPool2 = epcHelper2->GetHelper();

  Ipv4AddressHelper tunPool2new = epcHelper1->AddTunDevice(tunPool2);
  Ipv4AddressHelper tunPool1new = epcHelper2->AddTunDevice(tunPool1);

  epcHelper1->ReSetHelper(tunPool1new);
  epcHelper2->ReSetHelper(tunPool2new);

  /** ****************** CONTROLLER SETUP ****************** **/
  lteHelper->CreateController(controller);
  for(uint16_t i=0; i<ofIngress.GetN(); i++)
    lteHelper->ConnectSocketsIngress(ofIngress.Get(i), ofIngress.Get(i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal());

  lteHelper->ConnectSocketsEgress(ofEgress.Get(0), ofEgress.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),
                                  epcHelper1->GetUeDefaultGatewayAddress());
  lteHelper->ConnectSocketsEgress(ofEgress.Get(1), ofEgress.Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),
                                  epcHelper2->GetUeDefaultGatewayAddress());

  /** ********************* MME SETUP ********************** **/
  if(singleMme){
    lteHelper->AttachMme(sourceMme, epcHelper1->GetUeDefaultGatewayAddress());
    lteHelper->AttachMme(sourceMme, epcHelper2->GetUeDefaultGatewayAddress());
  }
  else {
    lteHelper->AttachMme(sourceMme, epcHelper1->GetUeDefaultGatewayAddress());
    lteHelper->AttachMme(targetMme, epcHelper2->GetUeDefaultGatewayAddress());
    lteHelper->ConnectMmes(sourceMme, targetMme, epcHelper2->GetUeDefaultGatewayAddress());
  }

  /** **************** BACKGROUND TRAFFIC ***************** **/
  InstallBgApplication(backboneNodes.Get(1), backboneNodes.Get(21), 10, bgActive);
  SetupRouting(backboneNodes.Get(21), backboneNodes.Get(1), 3);
  InstallBgApplication(backboneNodes.Get(3), backboneNodes.Get(6), 10, bgActive);
  InstallBgApplication(backboneNodes.Get(1), backboneNodes.Get(5), 10, bgActive);
  SetupRouting(backboneNodes.Get(5), backboneNodes.Get(1), 1);
  InstallBgApplication(backboneNodes.Get(0), backboneNodes.Get(3), 10, bgActive);
  SetupRouting(backboneNodes.Get(3), backboneNodes.Get(0), 2);
  InstallBgApplication(backboneNodes.Get(2), backboneNodes.Get(3), 10, bgActive);
  SetupRouting(backboneNodes.Get(3), backboneNodes.Get(2), 1);
  InstallBgApplication(backboneNodes.Get(25), backboneNodes.Get(28), 10, bgActive);

  InstallBgApplication(sourceMme, sourceEnb, 1, bgActive);
  InstallBgApplication(targetMme, targetEnb, 1, bgActive);
  InstallBgApplication(sourceSPgw, sgiAR1, 1, bgActive);
  InstallBgApplication(targetSPgw, sgiAR2, 1, bgActive);
  if(singleMme)
	InstallBgApplication(sourceMme, sgiAR2, 1, bgActive);

  /** **************** X2-HANDOVER CONFIG ***************** **/
  // Add X2 interfaces
  NodeContainer x2Nodes;
  x2Nodes.Add(enbNodesEpc1.Get(0));
  x2Nodes.Add(enbNodesEpc2.Get(0));
  lteHelper->AddX2Interface (x2Nodes);

  /** *************** P-GW HANDOVER CONFIG **************** **/
  NodeContainer uesEpc2(lteVoiceUeContainer2,
                        lteVideoUeContainer2,
                        lteGamingUeContainer2,
                        lteHttpUeContainer2,
                        lteFtpUeContainer2);

  epcHelper2->SetContainer(uesEpc2);

  Ptr<NormalRandomVariable> x = CreateObject<NormalRandomVariable> ();
  x->SetAttribute ("Mean", DoubleValue (32.1));
  x->SetAttribute ("Variance", DoubleValue (pow(4.33, 2)));

  ostringstream handoverFile;
  handoverFile << version << "_handoverTimes";
  ofstream out ((handoverFile.str ()).c_str (), ios::app);

  if(singleMme) {
    for(uint16_t i=0; i<lteVoiceUeContainerHO.GetN(); i++)
    {
      double hoTime = TriggerHO(radius, x->GetValue()) + 7.5;
      hoTime = 12;
      out << "handover UE " << lteVoiceUeContainerHO.Get(i)->GetId() << " at " << hoTime << " seconds" << endl;
      lteHelper->S1HandoverRequest (Seconds(hoTime), lteVoiceUeContainerHO.Get(i),
                                    epcHelper2->GetUeDefaultGatewayAddress(), enbLteDevs.Get(0), enbLteDevs2.Get(0));
    }
  }
  else {
    for(uint16_t i=0; i<lteVoiceUeContainerHO.GetN(); i++)
    {
      double hoTime = TriggerHO(radius, x->GetValue()) + 7.5;
      out << "handover UE " << lteVoiceUeContainerHO.Get(i)->GetId() << " at " << hoTime << " seconds" << endl;
      lteHelper->S1HandoverRequestMmeRelocation (Seconds(hoTime), lteVoiceUeContainerHO.Get(i),
                                    epcHelper2->GetUeDefaultGatewayAddress(), enbLteDevs.Get(0), enbLteDevs2.Get(0));
    }
  }

  /** ***************** SIMULATOR STUFFS ****************** **/
  /*Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (2), routingStream);*/

  //lteHelper->EnableRlcTraces ();

  // connect custom trace sinks for RRC connection establishment and handover notification
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
                   MakeCallback (&NotifyHandoverStartEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
                   MakeCallback (&NotifyHandoverStartUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkUe));

  FlowMonitorHelper flowMonHelper;
  Ptr<FlowMonitor> flowMonitor;
  //flowMonitor = flowMonHelper.InstallAll (); // problem in epc-sgw-pgw module
  flowMonitor = flowMonHelper.Install (lteVoiceUeContainerHO);
  flowMonitor = flowMonHelper.Install (remoteHosts);
  flowMonitor->SetAttribute ("StartTime", TimeValue(Seconds(1.)));
  flowMonitor->SetAttribute ("DelayBinWidth", DoubleValue(0.001));
  flowMonitor->SetAttribute ("JitterBinWidth", DoubleValue(0.001));
  flowMonitor->SetAttribute ("PacketSizeBinWidth", DoubleValue(20));

  /*AsciiTraceHelper asciiH;
  Ptr<OutputStreamWrapper> stream = asciiH.CreateFileStream (ascii);
  csmah.EnableAsciiAll (stream);*/
  // Enable PCAP tracing for all devices
  //p2ph.EnablePcapAll("traceP2p", false);
  //csmah.EnablePcapAll("traceCsma", false);

  NS_LOG_UNCOND("");
  NS_LOG_UNCOND("*** START SIMULATION ***");
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  NS_LOG_UNCOND("");
  NS_LOG_UNCOND("*** SIMULATION COMPLETED ***");

  flowMonitor->SerializeToXmlFile(xml,true,true);
  uint32_t bytes = lteHelper->GetSignalingBytes();
  out << "total signaling bytes: "<< bytes << endl;
  out << "signaling load (bps): "<< bytes * 8 / simTime << endl;

  Simulator::Destroy();
  return 0;

}
