/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   By Ramón. He quitado todo el tratamiento de trazas. En este ejemplo, sólo trataré las de aplicación.
*     Ya he conseguido arreglarlo. Por lo pronto, he tenido que descartar el BulkSendApplication y el
*     OnOffApplication, ya que daba problemas del tipo "intenta transmitir mientras transmite", y el
*     UdpClient, al configurarle el tiempo entre transmisiones, no lo hace. Por eso, habrá que tirar
*     al final por ese cliente (que no tiene trazas, por lo que sólo nos valen las de capas de enlace/física).
*     Podría ser problema de usar los 2 bearer (lo cual no lo entiendo). He quitado 1, y con eso me va el UdpClient.
*     Probaré mañana con los otros 2, Bulk y OnOff.
*
*   Activar todas las trazas: en línea de comandos -> export 'NS_LOG=p8_lte=level_all'
*/
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("p8_lte");


/**
 * Sample simulation script for a X2-based handover.
 * It instantiates two eNodeB, attaches one UE to the 'source' eNB and
 * triggers a handover of the UE towards the 'target' eNB.
 */
int main (int argc, char *argv[])
{
  NS_LOG_FUNCTION("Entrando en el método principal.");  

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true)); // Fija el checksum a true.
  Time::SetResolution (Time::US);   // Fijamos la resolución a microsegundos.

  // Parámetros de la simulación, con sus valores por defecto.

  uint16_t numberOfUes = 1;     // Número de nodos UE.
  uint16_t numberOfEnbs = 2;    // Número de nodos eNB.
  double simTime = 8.0;         // Tiempo de simulación.
  double distance = 100.0;      // Distancia entre nodos eNB.

  /* Se cambiarán algunos atributos por defecto de clases que se utilizarán en el código,
  para que tengan valores razonables según el escenario planteado. Se hace antes de procesar
  los argumentos pasados por línea de comandos para que el usuario pueda cambiarlos si lo
  desea. */

  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));

  // Usará el modelo real para la señalización RRC (que añadirá pérdidas y tal, supongo).

  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));

  /* Admitimos los parámetros de número de nodos UE, eNB y tiempo de simulación por línea
  de comandos. */

  CommandLine cmd;
  cmd.AddValue ("numberOfUes", "Número de nodos UE", numberOfUes);
  cmd.AddValue ("numberOfEnbs", "Número de nodos eNB", numberOfEnbs);
  cmd.AddValue ("simTime", "Duración total de la simulación (en segundos)", simTime);
  cmd.Parse (argc, argv);

  NS_LOG_INFO("Fin de tratamiento de variables de entorno.");
  
  // Para la parte del LTE
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  // La parte del epc. PointToPoint, usado en el EPC (ver diagrama de la documentación)
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  // Se unen las dos partes
  lteHelper->SetEpcHelper (epcHelper);
  // Planificador para los nodos eNB.
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  // Se desabilita el traspaso automático (de hecho, no hace nada).
  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");


  /*   PARTE DEL EPC  */

  // Guardamos el nodo SGW/PGW que sera la pasarela a internet
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Se crea un host remoto
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet; // Mezcla rutas estáticas y globales, por defecto.
  internet.Install (remoteHostContainer);

  // Creamos la conexion a Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  
  // Se instalan sgw/pgw y remoteHost en Internet
  /* Hay una conexión punto a punto entre el SGW/PGW y el equipo remoto. Se intenta modelar con un
  enlace punto a punto con retardo. Puede parecer más o menos realista, pero la parte donde realmente
  habría problemas de interferencias y tal es la parte LTE, luego veo bien el modelado de esta parte (si
  acaso mirar más detenidamente si admite alguna configuración más para complementar) */
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1); // Para el cliente UDP.
  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device. Ruta para ese destino.
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  /*                     Parte del LTE                       */

  // Se crean los nodos UE y ENB
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numberOfEnbs);
  ueNodes.Create (numberOfUes);

  // Se establece el modelo de movilidad: estarán quietos sin moverse
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
      positionAlloc->Add (Vector (distance * 2 * i - distance, 0, 0));
    }
  for (uint16_t i = 0; i < numberOfUes; i++)
    {
      positionAlloc->Add (Vector (0, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (enbNodes);
  mobility.Install (ueNodes);
  /* Opino. Si van a estar quietos, bastaría con poner un nodo enb, porque si no se mueve,
  nuncá habrá un traspaso del UE de un nodo enb a otro. Aunque habría que verlo en las
  simulaciones. */

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
  
  
  
  /*                Instalar e iniciar las aplicaicones de los UEs y el remote host                */
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // Se hace aleatorio el inicio de la simulacion
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (1.0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (1.010));

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

  // Se pone la aplicacion en cada UE
  // Comentario: Aun no entiendo muy bien como va lo de las aplicaciones
  for (uint32_t u = 0; u < numberOfUes; ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);

      // Ponemos como  gateway el pgw
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      // Este paso de poner el gateway lo hace en la línea 210-216. Habría que ver si se está repitiendo.

          /* Un "bearer", según he estado leyendo, es una especie de túnel entre UE y SGW, que dice el tipo
          de tráfico que estás enviando (datos, voz, vídeo, etc.), y se le puede dar o no también cierta
          calidad de servicio. */
          ++dlPort;
          ++ulPort;

          NS_LOG_LOGIC ("installing UDP DL app for UE " << u);

          /* Tanto el UE como el nodo remoto pueden transmitir y recibir tráfico. Es bidireccional. Por
          esto el downlink y uplink. */

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
    }

  // Pone una interfaz X2 en el eNodeB para señalizacion entre eNodesB (Por ejemplo para tareas de handover)
  // Comentario: la verdad es que no entiendo muy bien para que sirve lo del X2.
    // Es una interfaz entre enb's para evitar pérdidas de paquetes por traspaso de ue's entre enb's. Vamos,
    // señalización.
  lteHelper->AddX2Interface (enbNodes);

  // Se establece una peticion de handover
  lteHelper->HandoverRequest (Seconds (0.100), ueLteDevs.Get (0), enbLteDevs.Get (0), enbLteDevs.Get (1));

  Observador nodos_obs;
  nodos_obs.CapturaTrazas(serverApps.Get(0), serverApps.Get(1)); 

  // Se inicia el simulador
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();
  Simulator::Destroy ();

  double resultado = nodos_obs.DevuelvePorcentajeCorrectos1();
  NS_LOG_UNCOND("Paquetes 1 correctos = " << resultado);
  double resultado2 = nodos_obs.DevuelvePorcentajeCorrectos2();
  NS_LOG_UNCOND("Paquetes 2 correctos = " << resultado2);

  return 0;
}


