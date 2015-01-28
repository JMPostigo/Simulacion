
/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   TODO 1: comparar traficos VoIP, Datos y Video viendo el Delay y modificando el instante de traspaso 
*   TODO 2: Ver en qué parte se pierden más paquetes (Ue-eNode-pgw-server)
*
*   Info: este documento intenta cambiar el tipo de celula usando lteHelper->SetFfrAlgorithmType ("ns3::LteFrNoOpAlgorithm");
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/lte-helper.h"


#include "Observador.h"
//#include "ns3/rlc-stats-calculator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LenaX2HandoverExample");

/*  Mi intento de notificaciones  */

void NotifyIntento (std::string path, uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  std::cout << Simulator::Now ().GetSeconds () << " Esto es el intento de notificacion " << std::endl;
}


/*   NOTIFICADORES DEL RRC    */

/*                         NOTIFICADORES DE TX y RX                                                   */

void LteSimpleHelperDlTxPduCallback (Ptr<RadioBearerStatsCalculator> rlcStats, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  std::cout << " Se envia una trama " << std::endl;

  NS_LOG_FUNCTION (rlcStats << path << rnti << (uint16_t)lcid << packetSize);
  uint64_t imsi = 111;
  uint16_t cellId = 222;
  rlcStats->DlTxPdu (cellId, imsi, rnti, lcid, packetSize);
}

void LteSimpleHelperDlRxPduCallback (Ptr<RadioBearerStatsCalculator> rlcStats, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  std::cout << " Se recibe una trama " << std::endl;

  NS_LOG_FUNCTION (rlcStats << path << rnti << (uint16_t)lcid << packetSize << delay);
  uint64_t imsi = 333;
  uint16_t cellId = 555;
  rlcStats->DlRxPdu (cellId, imsi, rnti, lcid, packetSize, delay);
}


/**
 * Sample simulation script for a X2-based handover.
 * It instantiates two eNodeB, attaches one UE to the 'source' eNB and
 * triggers a handover of the UE towards the 'target' eNB.
 */
int main (int argc, char *argv[])
{
  // LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
  // LogComponentEnable ("LteHelper", logLevel);
  // LogComponentEnable ("EpcHelper", logLevel);
  // LogComponentEnable ("EpcEnbApplication", logLevel);
  // LogComponentEnable ("EpcX2", logLevel);
  // LogComponentEnable ("EpcSgwPgwApplication", logLevel);
  // LogComponentEnable ("LteEnbRrc", logLevel);
  // LogComponentEnable ("LteEnbNetDevice", logLevel);
  // LogComponentEnable ("LteUeRrc", logLevel);
  // LogComponentEnable ("LteUeNetDevice", logLevel);

  uint16_t numberOfUes = 1;
  uint16_t numberOfEnbs = 2;
  uint16_t numBearersPerUe = 2;
  double simTime = 8.0;
  //double distance = 100.0;

  // change some default attributes so that they are reasonable for
  // this scenario, but do this before processing command line
  // arguments, so that the user is allowed to override these settings
  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("numberOfUes", "Number of UEs", numberOfUes);
  cmd.AddValue ("numberOfEnbs", "Number of eNodeBs", numberOfEnbs);
  cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)", simTime);
  cmd.Parse (argc, argv);
  
  // Para la parte del LTE
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();




  // Ponemos el tipo de celula (No funciona)
  //lteHelper->SetFfrAlgorithmType ("ns3::LteFrNoOpAlgorithm");




  // La parte del epc
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  // Se unen las dos partes
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  //lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm"); // Se desabilita el handover automatico


  /*             PARTE DEL EPC                */

  // Guardamos el nodo SGW/PFW que sera la pasarela a internet
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Se crea un host remoto
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Creamos la conexion a Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  
  // Se instalan sgw/pgw y remoteHost en Internet
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);



  /*                     Parte del LTE                       */

  // Se crean los nodos UE y ENB
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numberOfEnbs);
  ueNodes.Create (numberOfUes);


  // Se establece el modelo de movilidad: estarán quietos sin moverse
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (-100, 0, 0));
  positionAlloc->Add (Vector (100, 0, 0));
  positionAlloc->Add (Vector (-50, 0, 0));
  
  MobilityHelper mobilitycons;
  mobilitycons.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilitycons.SetPositionAllocator (positionAlloc);
  mobilitycons.Install (enbNodes);

  //Mobility Constant Velocity Model 
  MobilityHelper mobspeed;
  mobspeed.SetMobilityModel  ("ns3::ConstantVelocityMobilityModel");
  mobspeed.SetPositionAllocator (positionAlloc);
  mobspeed.Install(ueNodes);
  
  Ptr<ConstantVelocityMobilityModel> mob = ueNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  mob->SetVelocity(Vector(1000.0, 0, 0));



  // Install LTE Devices in eNB and UEs
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach all UEs to the first eNodeB
  for (uint16_t i = 0; i < numberOfUes; i++)
    {
      lteHelper->Attach (ueLteDevs.Get (i), enbLteDevs.Get (0));
    }

  NS_LOG_LOGIC ("setting up applications");
  
  /*                Install and start applications on UEs and remote host                */
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // Sehace aleatorio el inicio de la simulacion
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (1.0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (1.010));

  // Se pone la aplicacion en cada UE
  for (uint32_t u = 0; u < numberOfUes; ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);

      // Ponemos como  gateway el pgw
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          NS_LOG_LOGIC ("installing UDP DL app for UE " << u);

          // APLICACION DOWNLINK: Acepta paquetes 
          UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          clientApps.Add (dlClientHelper.Install (remoteHost)); // downlink
          // Servidor recibe paquetes de cualquiera con PACKET SINK HELPER
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));
          NS_LOG_LOGIC ("installing UDP UL app for UE " << u);

          // UPLINK: Envia paquetes
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          clientApps.Add (ulClientHelper.Install (ue)); // uplink
          // Se instala la aplicacion en el Servidor
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
          

          // Se implementa el mensajero TFT que comunica LTE con EPC
          Ptr<EpcTft> tft = Create<EpcTft> (); // Impliementa al mensajeto Traffic Flow Template de EPC
          // Downlink
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          // Uplink
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);

          // Mensajero
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get (u), bearer, tft); // Activa el envio de UE a Internet
          Time startTime = Seconds (startTimeSeconds->GetValue ());
          serverApps.Start (startTime);
          clientApps.Start (startTime);

        } // end for b

    }

  // Pone una interfaz X2 en el eNodeB para señalizacion entre eNodesB (Por ejemplo para tareas de handover)
  lteHelper->AddX2Interface (enbNodes);

  //          -------- Se establece una peticion de handover         --------------  //
  //lteHelper->HandoverRequest (Seconds (4.100), ueLteDevs.Get (0), enbLteDevs.Get (0), enbLteDevs.Get (1));

  // Descomentar la siguiente linea para capturar las trazas
  p2ph.EnablePcapAll("lena-x2-handover");
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats (); // Calculador de estadisticas del DL y UL
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.05))); // Se establece la duracion de la epoca

  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats (); // Calculador de estadisticas del DL y UL
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.05))); // Se establece la duracion de la epoca

  // Se capturan las trazas

  Observador obser ( DynamicCast<LteUeNetDevice> (ueLteDevs.Get (0)), DynamicCast<LteEnbNetDevice> (enbLteDevs.Get (0)),pgw->GetDevice (2), internetDevices.Get (1));
 
  Config::Connect ("/NodeList/*/DeviceList/*/LtePdcp/TxPDU", MakeCallback (&NotifyIntento));
  
  Config::Connect ("/NodeList/*/DeviceList/*/LtePdcp/TxPDU", MakeBoundCallback (&LteSimpleHelperDlTxPduCallback, rlcStats));
  Config::Connect ("/NodeList/*/DeviceList/*/LteRlc/RxPDU", MakeBoundCallback (&LteSimpleHelperDlRxPduCallback, rlcStats));  
 
  // Se inicia el simulador
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  
  
  // GtkConfigStore config;
  // config.ConfigureAttributes ();
  Simulator::Destroy ();

  // Imprime el retraso
  for (int j = 0;j<1;j++) {
        NS_LOG_UNCOND ("  ----  Ue: " << j << " ----- " );
                
        for (int i = 4; i<6;i++) {
                uint64_t targetNodeID = ueLteDevs.Get (j)->GetObject<LteUeNetDevice> ()->GetImsi ();
                std::vector< double > stats = lteHelper->GetRlcStats()->GetDlDelayStats(targetNodeID,i); // imsi, lcid
  
                NS_LOG_UNCOND ("DL:" << i << ":  avg Dl delay (in seconds) of target UE = " << stats[0] / 1.0e+9 );
                NS_LOG_UNCOND ("DL:" << i << ":  std dev Dl delay (in seconds) of target UE = " << stats[1] / 1.0e+9 );  
                NS_LOG_UNCOND ("DL:" << i << ":  min Dl delay (in seconds) of target UE = " << stats[2] / 1.0e+9 );
                NS_LOG_UNCOND ("DL:" << i << ":  max Dl delay (in seconds) of target UE = " << stats[3] / 1.0e+9 );
        }
  }

  return 0;
}


