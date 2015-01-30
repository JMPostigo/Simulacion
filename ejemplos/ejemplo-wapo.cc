
/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Pham Quoc Viet 
 * Date: 01/07/2014
 * Email: vietpq90@gmail.com
 */

// Declare header files
#include <ns3/object.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include "ns3/radio-bearer-stats-calculator.h"
#include <ns3/constant-position-mobility-model.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include <ns3/netanim-module.h>
#include "../src/point-to-point/helper/point-to-point-helper.h"
#include "../build/ns3/ptr.h"
#include "../build/ns3/animation-interface.h"
#include "../build/ns3/simulator.h"
#include "ns3/gnuplot.h"

#include <ns3/lte-enb-phy.h>
#include <ns3/lte-ue-phy.h>

#include "ns3/string.h"
#include "ns3/double.h"
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <iomanip>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

//Add for config.txt file
#include "ns3/config-store.h"
#include "ns3/core-module.h"

//FlowMonitor Trial
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

// Declare namespace 
using namespace ns3;
using std::vector;

NS_LOG_COMPONENT_DEFINE("myX2-MultiUes5-TrialHandover");

void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << Simulator::Now ().GetSeconds () << " " << context
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
  std::cout << Simulator::Now ().GetSeconds () << " " << context
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
  std::cout << Simulator::Now ().GetSeconds () << " " << context
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
  std::cout << Simulator::Now ().GetSeconds () << " " << context
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
  std::cout << Simulator::Now ().GetSeconds () << " " << context
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
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

/* 
 * In this simulation, I want to create a topology with 5 HeNBs and only one UE. The UE trajectory
 * is known in advance and handover algorithm is automatically performed using the existing algorithms
 *
 * From results will be obtained, I will investigate my proposed algorithm and then competitive
 * results will be compared with others.
 */


class myX2Handover
  {
  public:
    // Number of UEs
    uint16_t numberOfUes;
    uint16_t numberOfEnbUes;
    // Number of HeNBs
    uint16_t numberOfHenbs;
    uint16_t numberOfEnbs;
    // Number of bearers per UE
    //uint16_t numberOfBearersPerUE = 2;
    uint16_t numberOfBearersPerUE;
    // UE speed (m/s)
    double ueSpeed;
    // Distance (constant distance) between two eNBs (m)
    double henbDistance;
    double ueHenbDistance;
    // Simulation times
    double simulationTime;
    // Power transmission of each HeNB/eNB (dbm, and transmission default power are respectively 20dbm and 46dbm)
    double henbTxPowerDbm;
    double henbNoiseFigure;
    double enbTxPowerDbm;
    double enbNoiseFigure;
    double ueTxPowerDbm;
    double ueNoiseFigure;    

    // Name of file for animation output
    std::string animFile;    
  public:
    myX2Handover();
    ~myX2Handover();
    
    void parConfiguration(int argc, char** agrv);
    //void InstallEPC ();
    //void InstallUeEnbs ();
    //void InstallMobility ();
    //void InstallApplications ();
    
    // Set FlowMonitor of a node
    //void SetFlowMonitor (Ptr<Node> flowMonitorUeNode, Ptr<Node> flowMonitorEnbNode);
    
    // Set FlowMonitor of all nodes
    //void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> monitor);
    
    void Run ();
  };

myX2Handover::myX2Handover()
  {
    // Number of UEs
    numberOfUes = 6;
    numberOfEnbUes = 4;
    numberOfHenbs = 6;	// Number of HeNBs
    numberOfEnbs = 1;
    numberOfBearersPerUE = 2;
    ueSpeed = 10;	// UE speed (m/s)
    henbDistance = 240;	// Distance (constant distance) between two eNBs (m)
    ueHenbDistance = 10;
    simulationTime = (double) (( numberOfHenbs + 1) * henbDistance / ueSpeed);	// Simulation times
    // Power transmission of each HeNB/eNB (dbm, and transmission default power are respectively 20dbm and 46dbm)
    henbTxPowerDbm = 20;
    henbNoiseFigure = 8;
    enbTxPowerDbm = 46;
    enbNoiseFigure = 5;
    ueTxPowerDbm = 20;
    ueNoiseFigure = 8;
    
    // Name of file for animation output
    animFile = "myX2-MultiUes5-TrialHandover.xml";
  }

myX2Handover::~myX2Handover()
  {

  }

void myX2Handover::parConfiguration(int argc, char *argv[])
  {	
    // change some default attributes so that they are reasonable for
    // this scenario, but do this before processing command line
    // arguments, so that the user is allowed to override these settings
    Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
    Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
    Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
    Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue (henbTxPowerDbm));
    Config::SetDefault("ns3::LteEnbPhy::NoiseFigure", DoubleValue (henbNoiseFigure));
    Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue (ueTxPowerDbm));
    Config::SetDefault("ns3::LteUePhy::NoiseFigure", DoubleValue (ueNoiseFigure));
	  
    // Command line arguments
    CommandLine cmd;
    cmd.AddValue ("numberOfEnbUes", "Number of UEs connected to eNB", numberOfEnbUes);
    cmd.AddValue ("numberOfUes", "Number of UEs", numberOfUes);
    cmd.AddValue ("numberOfHenbs", "Number of HeNBs", numberOfHenbs);
    cmd.AddValue ("numberOfEnbs", "Number of HeNBs", numberOfEnbs);
    cmd.AddValue ("numberOfBearersPerUE", "Number of Bearers per UE", numberOfBearersPerUE);
    cmd.AddValue ("henbTxPowerDbm", "Transmission default power of a HeNB", henbTxPowerDbm);
    cmd.AddValue ("enbTxPowerDbm", "Transmission default power of an eNB", enbTxPowerDbm);
    cmd.AddValue ("ueTxPowerDbm", "Transmission default power of an UE", ueTxPowerDbm);
    cmd.AddValue ("ueSpeed", "Speed of UE", ueSpeed);
    cmd.AddValue ("henbDistance", "Distance between two adjacent HeNBs", henbDistance);
    cmd.AddValue ("simulationTime", "Total duration of the simulation", simulationTime);
    cmd.Parse (argc, argv);
    
    // LogLevel logLevel = (LogLevel)(LOG_PREFIX_ALL | LOG_LEVEL_ALL);

    //LogComponentEnable ("LteHelper", logLevel);
    //LogComponentEnable ("EpcHelper", logLevel);
    //LogComponentEnable ("EpcEnbApplication", logLevel);
    //LogComponentEnable ("EpcX2", logLevel);
    //LogComponentEnable ("EpcSgwPgwApplication", logLevel);
    //LogComponentEnable ("LteEnbRrc", logLevel);
    //LogComponentEnable ("LteEnbNetDevice", logLevel);
    //LogComponentEnable ("LteUeRrc", logLevel);
    //LogComponentEnable ("LteUeNetDevice", logLevel);
    //LogComponentEnable ("A2A4RsrqHandoverAlgorithm", logLevel);
    //LogComponentEnable ("A3RsrpHandoverAlgorithm", logLevel);
    //LogComponentEnable ("ProposedAlgorithm", logLevel);
    
    std::cout << "Parameters configuration: Done" << std::endl;
  }
  
/*
 * FlowMonitor captures end-to-end Layer3 (IP) traffic, and there is no traffic between a UE and an eNB
 * in the LENA architecture; therefore, we might get 0 throughput from UE to eNB when using FlowMonitor
 */
void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset dataset)
  {	
    double tempThroughput;
    flowMon->CheckForLostPackets(); 
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
    
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
      {	
	// A tuple: Source-ip, destination-ip, protocol, source-port, destination-port
	Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
	std::cout<<"Flow ID: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
	std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
	std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
	std::cout<<"Duration: "<<stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
	std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
	
	tempThroughput = stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024;
	std::cout<<"Throughput: " << tempThroughput  << " Kbps"<<std::endl;
	std::cout<<"------------------------------------------"<<std::endl;
	
	dataset.Add((double)stats->first,(double) tempThroughput);
      }	
    Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon, dataset);
    
    // Write results to an XML file
    flowMon->SerializeToXmlFile ("ThroughputMonitor.xml", true, true);
  }

int
main (int argc, char *argv[])
{ 
  myX2Handover myLteHandover;
  
  myLteHandover.parConfiguration(argc, argv);
  //myLteHandover.SetFlowMonitor();
  
  // Create lteHelper and then epcHelper
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
	
  // Path loss Models
  //lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
    
  // Setting Fading Model
  //lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
  //lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EVA_60kmph.fad"));
  //lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
  //lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
    
  /*
  // These parameters have to setted only in case of the trace format differs from the standard one, that is
  // - 10 seconds length trace
  // - 10,000 samples
  // - 0.5 seconds for window size
  // - 100 RB
  lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
  lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
  lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
  lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
  */
	
  // Handover decision based on A2, A4 event
  lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
  lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (1));
	
  /* Handover decision based on A3 event
    
   lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
   lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue (3.0));
   lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (256)));
   */
   //NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
   
	 
  /* My proposed handover decision
   * lteHelper->SetHandoverAlgorithmType ("ns3::ProposedHandoverAlgorithm");
   * lteHelper->SetHandoverAlgorithmAttribute ("Attribute1", ValueOfAttribute1);
   * lteHelper->SetHandoverAlgorithmAttribute ("Attribute2", ValueOfAttribute2);
   * //NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
   */
	
  // Create a Gateway Node
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  // If we do not use the following command, it will display a Warning that our nodes
  // do not have an X, Y position co-ordinate and AnimationInterface is choosing a random 
  // value for our stationary nodes. But our xml file will be readable by NetAnim.
  AnimationInterface::SetConstantPosition (pgw, 10, 10);	
  AnimationInterface::SetNodeDescription (pgw, "DN Gateway");
	
  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  AnimationInterface::SetConstantPosition (remoteHost, 10, 30);
  AnimationInterface::SetNodeDescription (remoteHost, "Remote Host");
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
	
  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));	// Data rate
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));		// MTU
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));	// Delay
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);	// Attach the remote host to a PGW
  // Set IPv4 address of the gateway Node
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);	
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
	
  // Routing of the Internet Host (towards the LTE network or the address of the remote node)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  
	
  // Node Container
  NodeContainer ueNodes;
  NodeContainer henbNodes;
  ueNodes.Create (myLteHandover.numberOfUes);
  // henbNodes.Create (numberOfHenbs + numberOfEnbs);
  henbNodes.Create (myLteHandover.numberOfHenbs);





	
  // Mobility Helper
  MobilityHelper ueMobility;
  MobilityHelper henbMobility;
	
  // Install mobility model in UE(s)
  ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  ueMobility.Install (ueNodes.Get (0));
  ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, myLteHandover.henbDistance, 0));
  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (myLteHandover.ueSpeed, 0, 0));
  AnimationInterface::SetNodeDescription (ueNodes.Get (0), "UE-0");
  for (uint16_t i = 1; i < myLteHandover.numberOfUes; i++)
    {
      ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      ueMobility.Install (ueNodes.Get (i));
      ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (myLteHandover.henbDistance * (i + 0.2), 1.2*(myLteHandover.henbDistance), 0));
      AnimationInterface::SetNodeDescription (ueNodes.Get (i), "UE");
    }
  
  // Install mobility model in HeNB(s)
  // Allocate positions from a deterministic list specified the user.
  Ptr<ListPositionAllocator> HenbPositionAlloc = CreateObject<ListPositionAllocator> ();
  Vector henbPosition (myLteHandover.henbDistance * (4), myLteHandover.henbDistance * (3.8), 0);
  HenbPositionAlloc->Add (henbPosition);
  for (uint16_t i = 1; i < myLteHandover.numberOfHenbs; i++)
    {
      Vector henbPosition (myLteHandover.henbDistance * (i), myLteHandover.henbDistance, 0);
      HenbPositionAlloc->Add (henbPosition);
    }
  henbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  henbMobility.SetPositionAllocator (HenbPositionAlloc);
  henbMobility.Install (henbNodes);
  AnimationInterface::SetNodeDescription (henbNodes.Get (0), "eNB");
  for (uint16_t i = 1; i < myLteHandover.numberOfHenbs; i++)
    {
      std::string henbName = "HeNB";
      AnimationInterface::SetNodeDescription (henbNodes.Get (i), henbName);
    }
  
 /**************** *****************/





  // Install LTE devices in HeNB(s) and UE(s)
  NetDeviceContainer henbLteDevs = lteHelper->InstallEnbDevice (henbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);



 /**************** *****************/
  // Set eNB's transmission power
  // In my simulation, I want to add a eNB, but the problem is that ns-3 do not support for small cells like femtocell, picocell, etc.
  // So, I assume that an eNBs and a HeNBs is similar to each other in view of Phy regarless of transmission power.
  Ptr<LteEnbNetDevice> enbLteNetDevs = henbLteDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy = enbLteNetDevs->GetPhy();
  enbPhy->SetAttribute ("TxPower", (DoubleValue) (myLteHandover.enbTxPowerDbm/2));
  enbPhy->SetAttribute ("NoiseFigure", (DoubleValue) (myLteHandover.enbNoiseFigure));

 /**************** *****************/


	
  // Install IP Stack on the UE(s)
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpInterface;
  // Assign IP addresses to UEs automatically
  ueIpInterface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)  
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
	
  // Attach UE(s) to HeNB(s)/eNB(s)
  // But, in this scenario, I simulate only one UE and I then attach this UE to the first HeNB
  lteHelper->Attach (ueLteDevs.Get (0), henbLteDevs.Get (1));
  for (uint16_t i = 1; i < myLteHandover.numberOfUes; i++)	
    {	
      lteHelper->Attach (ueLteDevs.Get (i), henbLteDevs.Get (i));
    }
	
  NS_LOG_LOGIC ("Setting up applications on UE(s)");
	
  // Downlink port and Uplink port
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;
	
  /* 
   * randomize a bit start times to avoid simulation artefacts
   * (e.g., buffer overflows due to packet transmissions happening exactly at the same time)
   */ 
  Ptr<UniformRandomVariable> startSimTimeSeconds = CreateObject<UniformRandomVariable> ();
  startSimTimeSeconds->SetAttribute ("Min", DoubleValue (0));		// 0 second
  startSimTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));	// 0.010 second
	
  for (uint32_t u = 0; u < myLteHandover.numberOfUes; ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      for (uint32_t b = 0; b < myLteHandover.numberOfBearersPerUE; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;
					
	  // Config UDP Downlink
          NS_LOG_LOGIC ("Installing UDP Downlink Application(s) for UE " << u);
          UdpClientHelper dlClientHelper (ueIpInterface.GetAddress (u), dlPort);
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));	

	  // Config UDP Uplink
          NS_LOG_LOGIC ("Installing UDP Uplink Application(s) for UE " << u);
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);	// IP address: 1.0.0.1/8
          clientApps.Add (ulClientHelper.Install (ue));
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

	  // Filter downlink local port
          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
	  // Filter uplink remote port
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
	  /*
	   * Activate bearer to transfer TCP-based video application, and UEs are a kind of NGBR UEs.
	   * NGBR: None Guaranteed Bit Rate
	   */
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get (u), bearer, tft);

          Time startSimTime = Seconds (startSimTimeSeconds->GetValue ());
          serverApps.Start (startSimTime);
          clientApps.Start (startSimTime);
        } // end for b (index b)
    }
		
  // Add X2 interface
  lteHelper->AddX2Interface (henbNodes);
  // Enable traces
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  
  // Set files for calculating throughput
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));
	
  // Connect custom trace sinks for RRC connection establishment and handover notification
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

  std::cout << "The simulation is running..." << std::endl;
  
  // @Exctract data from xml file - ns-3-users google group
  // Gnuplot parameters
  std::string fileNameWithNoExtension = "FlowVsThroughput";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = "Flow vs Throughput";
  std::string dataTitle               = "Throughput";

  // Instantiate the plot and set its title.
  Gnuplot gnuplot (graphicsFileName);
  gnuplot.SetTitle (plotTitle);
  gnuplot.SetTerminal ("png");			// Make the .PNG file, which the plot file will be when it is used with Gnuplot
  gnuplot.SetLegend ("Flow", "Throughput");   	// Set the labels for each axis
  Gnuplot2dDataset dataset;			// Instantiate the dataset, set its title, and make the points be plotted along with connecting lines.
  dataset.SetTitle (dataTitle);
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  
  // Create the animation object and configure for specified output
  AnimationInterface anim (myLteHandover.animFile);

  // Throughput Monitor
  FlowMonitorHelper fmhelper;
  NodeContainer flowMon_nodes;
  flowMon_nodes.Add (ueNodes);
  flowMon_nodes.Add (remoteHostContainer);
  Ptr<FlowMonitor> monitor;
  //monitor = fmhelper.InstallAll();
  monitor = fmhelper.Install (flowMon_nodes);
  
  // Start simulation
  Simulator::Stop (Seconds (myLteHandover.simulationTime));
    
  // Suffice to say that enabling the flow monitor is just the matter of replacing the line ns3.Simulator.Run ()
  ThroughputMonitor (&fmhelper, monitor, dataset);
  
  //Gnuplot ...continued
  gnuplot.AddDataset (dataset);
  std::ofstream plotFile (plotFileName.c_str());// Open the plot file.
  gnuplot.GenerateOutput (plotFile);		// Write the plot file.
  plotFile.close ();				// Close the plot file.
  
  Simulator::Run ();
  
  std::cout << "Animation Trace File was created:" << myLteHandover.animFile.c_str () << std::endl;
	
  // End simulation
  Simulator::Destroy ();
  
  std::cout << "The simulation finished!" << std::endl; 
  std::cout << "" << std::endl;
	
  return 0;
}


