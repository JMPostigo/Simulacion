/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*  PRÁCTICA 8. PLANIFICACIÓN Y SIMULACIÓN DE REDES
*     ejemplo-EPC.cc
*       -Ramón Pérez Hernández
*       -José Manuel Postigo Aguilar
*       -María Valero Campaña
*
*   Repositorio Git, con todo el proceso realizado:
*       https://github.com/JMPostigo/Simulacion
*
*   Activar todas las trazas de interés: en línea de comandos -> 
*       export 'NS_LOG=p8_lte=level_debug:Observador=level_warn'
*
*   Para información más detallada sobre el modelo, referenciamos la página:
*       http://www.nsnam.org/docs/models/html/lte-design.html
*/

/*
*   TODO: capturar trazas Tx y Rx para el EPC, Imprimir las estadisticas del PDCPStats tras la simulacion
*   By Ramón. Funciona con UdpClient y con BulkSendApplication. Debería funcionar con OnOffApplication.
*   Calcula el porcentaje de correctos perfectamente (sale, en su valor máximo, un 50%).
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/error-model.h"
//#include "ns3/rlc-stats-calculator.h"
#include "Observador.h"


#define  N_UE                 2 // Valor por defecto del número de nodos UE.
#define  N_ENB                2 // Valor por defecto del número de nodos eNB.
#define  T_SIM             5.0 // Valor por defecto del tiempo de simulación.
#define  DIST_NODOS       100.0 // Valor por defecto de la distancia entre nodos eNB.
#define  NUM_REMOTOS          1 // Número de equipos remotos accesibles por los UE.
#define  PTO_BAJADA       10000 // Puerto de subida.
#define  PTO_SUBIDA       20000 // Puerto de bajada.


using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("p8_lte");


  /*        Funciones que manejan las trazas RRC.
     Habría que modificar el cout por NS_LOG, y ponerlo en Observador, si acaso.    */
void NotifyConnectionEstablishedUe (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " Conexión establecida:  UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void NotifyHandoverStartUe (std::string context, uint64_t imsi,uint16_t cellid,uint16_t rnti, uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " HANDOVER START: UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void NotifyHandoverEndOkUe (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " HANDOVER END: UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void NotifyConnectionEstablishedEnb (std::string context, uint64_t imsi, uint16_t cellid,uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << "CONEXION ESTABLECIDA: eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void NotifyHandoverStartEnb (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti, uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << "HANDOVER START: eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}

void NotifyHandoverEndOkEnb (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << "HANDOVER END: eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}


int main (int argc, char *argv[])
{
  NS_LOG_FUNCTION("Entrando en el método principal.");  

      /* RAMÓN. Dejo esto, por si tenemos que coger alguna traza de aquí.

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
      */

  // Parámetros de la simulación, con sus valores por defecto.

  uint16_t num_UEs = N_UE;              // Número de nodos UE.
  uint16_t num_eNBs = N_ENB;            // Número de nodos eNB.
  double t_simulacion = T_SIM;          // Tiempo de simulación.
  double distancia_nodos = DIST_NODOS;  // Distancia entre nodo UE y eNB.

  /* Antes de procesar los argumentos pasados por línea de comandos, vamos a modificar
  algunos atributos por defecto de clases que se utilizarán en el código para que tengan
  valores razonables. Al hacerlo antes, el usuario podrá cambiarlo si lo desea (por línea
  de comandos, por ejemplo). */

  // Usaremos el modelo real para la señalización RRC (de gestión de recursos radio).

  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
  Config::SetDefault ("ns3::LteHelper::PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));

  // Admitimos los parámetros anteriores por línea de comandos.

  CommandLine cmd;

  cmd.AddValue ("num_UEs", "Número de nodos UE", num_UEs);
  cmd.AddValue ("num_eNBs", "Número de nodos eNB", num_eNBs);
  cmd.AddValue ("t_simulacion", "Duración total de la simulación (en segundos)", t_simulacion);
  cmd.AddValue ("distancia_nodos", "Distancia entre nodo UE y eNB (en metros)", distancia_nodos);

  cmd.Parse (argc, argv);

  NS_LOG_INFO("Fin de tratamiento de variables de entorno.");

      // CONFIGURACIÓN DEL MODELO LTE (entre nodos UE y nodos eNB)

  // En primer lugar, instanciamos el Helper para la parte LTE.

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  /* Instanciamos también el Helper para la parte EPC, que será punto a punto, para
  luego asociarlo al Helper de LTE. */

  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  // Se utilizará un planificador para los nodos eNB basado en Round Robin.
    // Aquí, igual, podría meterse modelo de error.

  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  // Se deshabilita el traspaso automático de nodo eNB cuando un nodo UE se mueve.
    // Tocar aquí para habilitar el movimiento de UE.

  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");

  NS_LOG_INFO("Modelo LTE configurado.");

      // CONFIGURACIÓN DEL MODELO EPC (entre nodos eNB y el encaminador (SGW/PGW) a Internet)

  // Guardamos el nodo SGW/PGW, que será la pasarela a Internet.

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  NS_LOG_INFO("Modelo EPC configurado.");

      // CONFIGURACIÓN DEL MODELO DE INTERNET (equipos accesibles a través de Internet)

  /* Primero, generamos los nodos que serán accesibles por los nodos UE a través de
  Internet. Para nuestro escenario, supondremos que sólo habrá un equipo.
  Obtendremos el nodo de dicho equipo con el método Get del Container. */

  NodeContainer nodosRemotos;
  nodosRemotos.Create (NUM_REMOTOS);
  Ptr<Node> equipoRemoto = nodosRemotos.Get (0);
  
  // Instalamos la pila TCP/IP en los nodos remotos.  

  InternetStackHelper internet;
  internet.Install (nodosRemotos);

  /* A continuación, generamos el enlace que unirá el nodo SGW/PGW con el equipo remoto.
  El acceso a dicho equipo se modelará con un enlace punto a punto, al que se le acumulará
  cierto valor de retardo debido al paso por encaminadores intermedios. */

  PointToPointHelper p2pInternet;
  p2pInternet.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pInternet.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pInternet.SetChannelAttribute ("Delay", TimeValue (Seconds (0.020)));

  /* Ahora, instanciamos el contenedor de NetDevice, que contendrán los 2 equipos conectados
  en el enlace punto a punto: el encaminador SGW/PGW, y el equipo remoto.
  Después, asignamos direcciones IP a los equipos del escenario. */

  NetDeviceContainer equiposInternet = p2pInternet.Install (pgw, equipoRemoto);
  Ipv4AddressHelper ipv4Internet;
  ipv4Internet.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4Internet.Assign (equiposInternet);

  // Guardamos la dirección del equipo remoto, para usarla al instanciar las aplicaciones cliente.

  Ipv4Address dirEquipoRemoto = internetIpIfaces.GetAddress (1);

  /* Para nuestro escenario, generaremos rutas estáticas en el equipo remoto hacia la
  red LTE. 
  El último parámetro del método que añade la ruta (AddNetworkRouteTo, cuando vale 1, 
  indica que el siguiente salto es el otro equipo conectado al enlace punto a punto. */

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> rutaEstaticaEquipoRemoto = ipv4RoutingHelper.GetStaticRouting 
                                                            (equipoRemoto->GetObject<Ipv4> ());
  rutaEstaticaEquipoRemoto->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  /* Por último, vamos a configurar un modelo de errores para nuestro modelo de Internet, de manera
  que tendremos un porcentaje del 95% de paquetes correctos. 
    TODAVÍA NO FUNCIONA

  Ptr<RateErrorModel> error_tx_pkt = CreateObject<RateErrorModel> ();
  error_tx_pkt->SetRate(0.05);
  error_tx_pkt->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);

  equiposInternet.Get(0)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_tx_pkt);
  equiposInternet.Get(1)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_tx_pkt); */

  NS_LOG_INFO("Modelo de Internet configurado.");

      // INSTALACIÓN Y CONFIGURACIÓN DE NODOS UE Y ENB

  // Se generan los nodos UE y eNB a utilizar.

  NodeContainer nodosUE;
  NodeContainer nodoseNB;
  nodoseNB.Create (num_eNBs);
  nodosUE.Create (num_UEs);

  NS_LOG_INFO("Nodos del modelo LTE generados.");

  /* A continuación, establecemos el modelo de movilidad de la parte LTE. En nuestro escenario,
  colocaremos los nodos eNB separados a cierta distancia, de forma que los nodos UE colocados
  estén en el punto intermedio de la línea que une a dos nodos eNB, a una distancia de valor
  distancia_nodos de cada nodo eNB. 
  Trabajaremos en 2 dimensiones. */

  Ptr<ListPositionAllocator> posicionNodos = CreateObject<ListPositionAllocator> ();

  for (uint16_t i = 0; i < num_eNBs; i++) {
    posicionNodos->Add (Vector (distancia_nodos * 2 * i - distancia_nodos, 0, 0));
  }

  for (uint16_t i = 0; i < num_UEs; i++) {
    posicionNodos->Add (Vector (0, 0, 0));
  }

  /* En nuestro escenario, los nodos estarán fijos en una posición. */

  MobilityHelper modeloMovilidad;
  modeloMovilidad.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  modeloMovilidad.SetPositionAllocator (posicionNodos);
  modeloMovilidad.Install (nodoseNB);
  modeloMovilidad.Install (nodosUE);

  NS_LOG_INFO("Establecido modelo de movilidad del modelo LTE.");

  /* Haciendo uso del LteHelper, instalamos los nodos UE y eNB en sus respectivos
  contenedores de NetDevice. */

  NetDeviceContainer dispLTEeNB = lteHelper->InstallEnbDevice (nodoseNB);
  NetDeviceContainer dispLTE_UE = lteHelper->InstallUeDevice (nodosUE);

  /* Instalamos la pila TCP/IP en los nodos UE, y les asignamos direcciones IP. La
  generación de rutas y unión con nodos eNB se hará dentro del bucle que configura
  las aplicaciones, para ser más cómodo. */

  internet.Install (nodosUE);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (dispLTE_UE));

  NS_LOG_INFO("Direcciones de nodos UE configuradas.");

      // INSTALACIÓN DE APLICACIONES EN LOS EQUIPOS FINALES

  /* Instanciamos los enteros que contendrán los puertos tanto para la conexión de subida
  (del UE hacia fuera) como de bajada (de fuera hacia el UE). */

  uint16_t pto_bajada = PTO_BAJADA;
  uint16_t pto_subida = PTO_SUBIDA;

  /* Se ponen aquí las aplicaciones porque, de declararlas en el for, no se
  podrían utilizar fuera del bucle. */

  ApplicationContainer appsCliente;
  ApplicationContainer appsServidor;

  /* Entramos en el bucle de generación de aplicaciones, además de fijar las rutas en los UE
  para salir a Internet, junto a la unión de nodos UE al nodo eNB que le corresponda. */

  for (uint32_t i = 0; i < num_UEs; i++) {
    /* Generamos las rutas estáticas para cada nodo UE. La única ruta a generar será la salida
    por defecto hacia el router que da acceso a los equipos remotos a través de Internet. 
    Tras ello, fijamos cada nodo UE al primer nodo eNB instalado. */

    Ptr<Node> nodoUE = nodosUE.Get (i);
    Ptr<Ipv4StaticRouting> rutaEstaticaUE = ipv4RoutingHelper.GetStaticRouting (nodoUE->GetObject<Ipv4> ());
    rutaEstaticaUE->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

    lteHelper->Attach (dispLTE_UE.Get (i), dispLTEeNB.Get (0));

    NS_LOG_INFO("Nodos UE configurados y unidos a su nodo eNB.");

    /* Tanto el UE como el nodo remoto pueden transmitir y recibir tráfico. Es bidireccional. Por
    esto, utilizaremos tráfico ascendente y descendente. */

        // TRÁFICO DESCENDENTE. El equipo remoto es emisor, y los UE receptores.

    /* En este caso, la aplicación emisora será el equipo remoto, que necesitará la dirección y puerto
    de cada UE (supondremos que todos los UE utilizan el mismo puerto de escucha). 
    La aplicación receptora estará en los nodos UE. Vemos que, aunque se use siempre el mismo
    puerto, la correspondencia IP-puerto es unívoca (la IP sí varía), luego no hace falta cambiar
    el puerto. */

    BulkSendHelper dlClientHelper ("ns3::TcpSocketFactory", 
                                      InetSocketAddress(ueIpIfaces.GetAddress (i), pto_bajada));
    appsCliente.Add (dlClientHelper.Install (equipoRemoto));

    PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory", 
                                      InetSocketAddress (Ipv4Address::GetAny (), pto_bajada));
    appsServidor.Add (dlPacketSinkHelper.Install (nodoUE));

    NS_LOG_INFO("Configuradas aplicaciones de tráfico descendente.");

        // TRÁFICO ASCENDENTE. Los nodos UE son emisores, y el tráfico lo recibe el equipo remoto.

    /* Ahora, la aplicación emisora estará en los nodos UE. Necesitarán la dirección y puerto del
    equipo remoto. Vemos que tendremos una aplicación en el equipo remoto por cada UE que le
    transmita paquetes. Por ello, tras instanciar el sumidero, incrementamos el valor del puerto, 
    para que la aplicación de los siguientes UE usen un puerto distinto. */

    BulkSendHelper ulClientHelper ("ns3::TcpSocketFactory", 
                                      InetSocketAddress(dirEquipoRemoto, pto_subida));
    appsCliente.Add (ulClientHelper.Install (nodoUE));

    PacketSinkHelper ulPacketSinkHelper ("ns3::TcpSocketFactory", 
                                      InetSocketAddress (Ipv4Address::GetAny (), pto_subida++));
    appsServidor.Add (ulPacketSinkHelper.Install (equipoRemoto));

    NS_LOG_INFO("Configuradas aplicaciones de tráfico ascendente.");

        // PROBLEMA. Esto no sirve para nada. No hace efecto.
    /*    Un "bearer", según he estado leyendo, es una especie de túnel entre UE y SGW, que dice el tipo
    de tráfico que estás enviando (datos, voz, vídeo, etc.), y se le puede dar o no también cierta
    calidad de servicio.

    // Se implementa el mensajero TFT que comunica LTE con EPC
    Ptr<EpcTft> tft = Create<EpcTft> (); // Impliementa al mensajeto Traffic Flow Template de EPC
    // Downlink
    EpcTft::PacketFilter dlpf;
    dlpf.localPortStart = pto_bajada;
    dlpf.localPortEnd = pto_bajada;
    tft->Add (dlpf);
    // Uplink
    EpcTft::PacketFilter ulpf;
    ulpf.remotePortStart = pto_subida;
    ulpf.remotePortEnd = pto_subida;
    tft->Add (ulpf);

    // Mensajero
    EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
    EpsBearer bearer2 (EpsBearer::NGBR_VIDEO_TCP_PREMIUM);

    if (i==0)
      lteHelper->ActivateDedicatedEpsBearer (dispLTE_UE.Get (i), bearer, tft); // Activa el envio de UE a Internet
    else
      lteHelper->ActivateDedicatedEpsBearer (dispLTE_UE.Get (i), bearer2, tft); // Activa el envio de UE a Internet
      */
    }

  /* Por último, fijamos el instante de inicio para las aplicaciones, determinado según una
  distribución aleatoria uniforme, con un rango de valores para el valor mínimo y máximo. */

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (1.0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (1.010));

  Time startTime = Seconds (startTimeSeconds->GetValue ());
  appsServidor.Start (startTime);
  appsCliente.Start (startTime);  

  NS_LOG_INFO("Aplicaciones configuradas.");

      // COMENTARIOS INTERFAZ X2 (POR VER)
  // Pone una interfaz X2 en el eNodeB para señalizacion entre eNodesB (Por ejemplo para tareas de handover)
  // Comentario: la verdad es que no entiendo muy bien para que sirve lo del X2.
    // Es una interfaz entre enb's para evitar pérdidas de paquetes por traspaso de ue's entre enb's. Vamos,
    // señalización.

  /*Postigo: Tengo entendido efectivamente que X2 es una interfaz que interconecta en forma de MALLA todos los
  enB, pero no tengo claro que sea PURA SEÑALIZACION, pues cuando se produce un traspaso (handover), lo que
  esta interfaz permite es que los paquetes "que se hayan quedado en la anterior antena almacenados" puedan
  enviarse a traves de esta interfaz, luego mas que de señalizacion diria q es de DATOS */ 
  lteHelper->AddX2Interface (nodoseNB);

  // Se establece la petición de traspaso al segundo nodo eNB.

  lteHelper->HandoverRequest (Seconds (0.100), dispLTE_UE.Get (0), dispLTEeNB.Get (0), dispLTEeNB.Get (1));
  lteHelper->HandoverRequest (Seconds (0.100), dispLTE_UE.Get (1), dispLTEeNB.Get (0), dispLTEeNB.Get (1));

  // Descomentar la siguiente linea para capturar las trazas
  //p2pInternet.EnablePcapAll("lena-x2-handover.pcap");
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats (); // Calculador de estadisticas del DL y UL
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.05))); // Se establece la duracion de la epoca

  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats (); // Calculador de estadisticas del DL y UL
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.05))); // Se establece la duracion de la epoca

  // Se capturan las trazas
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished", MakeCallback (&NotifyConnectionEstablishedEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished", MakeCallback (&NotifyConnectionEstablishedUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart", MakeCallback (&NotifyHandoverStartEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart", MakeCallback (&NotifyHandoverStartUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk", MakeCallback (&NotifyHandoverEndOkEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk", MakeCallback (&NotifyHandoverEndOkUe));

  Observador nodos_obs;
  nodos_obs.CapturaTrazas(appsCliente.Get(1), appsServidor.Get(1)); 

  // Se inicia el simulador
  Simulator::Stop (Seconds (t_simulacion));
  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();
  Simulator::Destroy ();

  double resultado = nodos_obs.DevuelvePorcentajeCorrectos();
  NS_LOG_UNCOND("Porcentaje correctos: " << resultado);

  return 0;
}


