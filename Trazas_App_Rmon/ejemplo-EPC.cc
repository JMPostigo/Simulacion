/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*  PRÁCTICA 8. PLANIFICACIÓN Y SIMULACIÓN DE REDES. GRUPO 4
*     p8_lte.cc
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/error-model.h"
#include <map>
#include "ns3/average.h"
#include <iostream>
#include <fstream>
#include "ns3/gnuplot.h"
#include "Observador.h"


#define  VALORES_T_STUDENT  101 // Número de valores guardados de la tabla t-Student (IC 90%).
#define  NUM_UE               1 // Número de nodos UE (fijo).
#define  NUM_ENB              1 // Número de nodos eNB de partida.
#define  NUM_REMOTOS          1 // Número de equipos remotos accesibles por el UE (fijo).
#define  POS_UE               0 // Posición inicial del UE.
#define  LONG_COB          1000 /* Valor por defecto de la longitud (en metros) del tramo 
                                   que se pretende cubrir. */
#define  INCR_POS_UE         20 // Incremento en la posición del nodo UE.
#define  NUM_ITERACIONES      3 /* Valor por defecto del número de iteraciones por cada 
                                   punto de la gráfica. */
#define  NUM_CURVAS           3 // Número de curvas a representar en cada gráfica.
#define  PTO_SUBIDA       20000 // Puerto TCP de subida (flujo del UE al equipo remoto).


using namespace ns3;


/* Definimos un nuevo tipo, llamado PLOT_ESTADISTICAS, que contendrá las dos variables 
Gnuplot encargadas de generar las curvas de cada estadística. */

typedef struct plot_estadisticas {
  Gnuplot porcentaje_correctos;
  Gnuplot numero_nodos_eNB;
} PLOT_ESTADISTICAS;


// Admitimos el uso de trazas.

NS_LOG_COMPONENT_DEFINE ("p8_lte");



double CalculaIC(Average<double> acumulador) {
  NS_LOG_FUNCTION("Entramos en el método CalculaIC.");

  /* Básicamente, calculamos el valor que se suma y resta a la media del estadístico
  muestral (que es la variable Average que se pasa como parámetro), para determinar
  su intervalo de confianza al 90%.
  Como el número de iteraciones se pasa como parámetro, tomaremos el t(x,n-1)=(1-alfa)/2=0.05, que
  nos da un valor de x.
  A partir de ahí, el extremo buscado será x*sqrt(s2(n)/n), con s2(n)=n*Var(estadístico)/(n-1),
  y n=nº iteraciones. */
  
  double valores_t_student[VALORES_T_STUDENT]= { 6.3137, 2.9200, 2.3534, 2.1318, 2.0150, 1.9432, 
         1.8946, 1.8595, 1.8331, 1.8125, 1.7959, 1.7823, 1.7709, 1.7613, 1.7531, 1.7459, 1.7396, 
         1.7341, 1.7291, 1.7247, 1.7207, 1.7171, 1.7139, 1.7109, 1.7081, 1.7056, 1.7033, 1.7011, 
         1.6991, 1.6973, 1.6955, 1.6939, 1.6924, 1.6909, 1.6896, 1.6883, 1.6871, 1.6860, 1.6849, 
         1.6839, 1.6829, 1.6820, 1.6811, 1.6802, 1.6794, 1.6787, 1.6779, 1.6772, 1.6766, 1.6759, 
         1.6753, 1.6747, 1.6741, 1.6736, 1.6730, 1.6725, 1.6720, 1.6716, 1.6711, 1.6706, 1.6702, 
         1.6698, 1.6694, 1.6690, 1.6686, 1.6683, 1.6679, 1.6676, 1.6672, 1.6669, 1.6666, 1.6663, 
         1.6660, 1.6657, 1.6654, 1.6652, 1.6649, 1.6646, 1.6644, 1.6641, 1.6639, 1.6636, 1.6634, 
         1.6632, 1.6630, 1.6628, 1.6626, 1.6624, 1.6622, 1.6620, 1.6618, 1.6616, 1.6614, 1.6612, 
         1.6611, 1.6609, 1.6607, 1.6606, 1.6604, 1.6602, 1.6449};  // Posibles valores de X.

  double gr_libertad = acumulador.Count()-1;  // Valor de n-1.
  double s2n = (acumulador.Count()*acumulador.Var())/gr_libertad; // Valor de s2(n).

  double valor_t_student;

  /* Como en la función principal ya acotamos el límite inferior de los grados de libertad, ahora
  acotamos el límite superior, de forma que si fuese mayor que 100, le asociamos el último valor
  de la tabla (n->Inf). En otro caso, le asignamos su valor correspondiente de la tabla. 
  No nos hace falta el valor de "iteraciones", ya que será el número de elementos que tenga la
  variable Average (luego nos podemos ahorrar ese segundo parámetro). */

  if (gr_libertad > VALORES_T_STUDENT-1)
    valor_t_student=valores_t_student[VALORES_T_STUDENT-1];
  else
    valor_t_student=valores_t_student[(int) gr_libertad-1];

  // Con esto, hallamos el valor antes mencionado:
  double ic = valor_t_student*(sqrt(s2n/acumulador.Count()));

  // Imprimimos el valor medio obtenido y el intervalo de confianza.
  NS_LOG_DEBUG("   " << acumulador.Avg() << ", con IC: [ " << acumulador.Avg()-ic << " , " 
                                                            << acumulador.Avg()+ic << " ]");    

  NS_LOG_INFO("Devolviendo valor del método CalculaIC.");  
  
  return ic;
}



double RealizaSimulacion(double t_simulacion, double posicion_UE, double radio_cobertura_eNB,
                         uint16_t num_nodos_eNB) {

  NS_LOG_FUNCTION("Entramos en el método RealizaSimulacion.");
 
  /* Ante los problemas que se ha tenido intentando sacar algún Helper o variable
  fuera de esta función, se ha decidido realizar el escenario completo para cada
  simulación. En otro caso, aparecen problemas de violación de segmento que impiden
  la ejecución de la simulación. */

      // MODELO LTE (entre nodos UE y nodos eNB)

  // En primer lugar, instanciamos el Helper para la parte LTE.

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  /* Instanciamos también el Helper para la parte EPC, que será punto a punto, para
  luego asociarlo al Helper de LTE. */

  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  // Se utilizará un planificador para los nodos eNB basado en Round Robin.

  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  /* Se deshabilita el traspaso automático de nodo eNB cuando un nodo UE se mueve (de hecho,
  no vamos a mover el terminal; lo colocaremos en distintas posiciones y mediremos los
  estadísticos que pretendemos estudiar para decidir cuántos nodos eNB son necesarios para
  cubrir todo el trayecto). */

  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");

  NS_LOG_INFO("Modelo LTE inicializado.");

      // MODELO EPC (entre nodos eNB y el encaminador (SGW/PGW) a Internet)

  // Guardamos el nodo SGW/PGW, que será la pasarela a Internet.

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  NS_LOG_INFO("Modelo EPC inicializado.");

      // MODELO DE INTERNET (equipos accesibles a través de Internet)

  /* Primero, generamos los nodos que serán accesibles por el UE a través de
  Internet. Para nuestro escenario, supondremos que sólo habrá un equipo.
  Obtendremos el nodo de dicho equipo con el método Get del Container. */

  NodeContainer nodosRemotos;
  nodosRemotos.Create (NUM_REMOTOS);
  Ptr<Node> equipoRemoto = nodosRemotos.Get (0);
  
  // Instalamos la pila TCP/IP en el nodo remoto.  

  InternetStackHelper internet;
  internet.Install (nodosRemotos);

  /* A continuación, generamos el enlace que unirá el nodo SGW/PGW con el equipo remoto.
  El acceso a dicho equipo se modelará con un enlace punto a punto, al que se le acumulará
  cierto valor de retardo debido al paso por encaminadores intermedios. Este retardo se
  ha calculado en base al tiempo de ida y vuelta de un ping desde un ordenador personal
  hasta un servidor cualquiera que esté tras Internet (por ejemplo, Google). */

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
  El último parámetro del método que añade la ruta AddNetworkRouteTo, cuando vale 1, 
  indica que el siguiente salto es el otro equipo conectado al enlace punto a punto. */

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> rutaEstaticaEquipoRemoto = ipv4RoutingHelper.GetStaticRouting 
                                                            (equipoRemoto->GetObject<Ipv4> ());
  rutaEstaticaEquipoRemoto->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  /* Por último, vamos a configurar un modelo de errores para nuestro modelo de Internet, de manera
  que tendremos un porcentaje del 99.99% de paquetes correctos en este modelo (porcentaje alto,
  para asegurar que no debe haber problemas en la conexión al equipo remoto desde Internet).
  Notar que la probabilidad de error es multiplicativa: la probabilidad de error en la parte radio
  se multiplicará a la probabilidad de error de este modelo, si hacemos el cómputo extremo a extremo. */

  Ptr<RateErrorModel> error_tx_pkt = CreateObject<RateErrorModel> ();
  error_tx_pkt->SetRate(0.0001);
  error_tx_pkt->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);

  equiposInternet.Get(0)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_tx_pkt);
  equiposInternet.Get(1)->GetObject<PointToPointNetDevice>()->SetReceiveErrorModel(error_tx_pkt);

  NS_LOG_INFO("Modelo de Internet inicializado.");

      // INSTALACIÓN Y CONFIGURACIÓN DE NODOS UE Y ENB

  // Se generan los nodos UE y eNB a utilizar.

  NodeContainer nodosUE;
  NodeContainer nodoseNB;
  nodoseNB.Create (num_nodos_eNB);
  nodosUE.Create (NUM_UE);

  NS_LOG_INFO("Nodos del modelo LTE generados.");

  /* A continuación, establecemos el modelo de movilidad de la parte LTE. En nuestro 
  escenario, colocaremos los nodos eNB separados cierta distancia, y el nodo UE se irá 
  colocando de forma que recorra todo el radio de cobertura de cada nodo eNB que se vaya 
  poniendo. 
  Si el radio de cobertura del nodo eNB es de 150 metros, cubrirá de 0 a 300 metros con
  seguridad (el primero). Luego, debemos colocar el siguiente nodo eNB en la posición 
  450 metros (con cobertura asegurada de 300-600 metros), y así sucesivamente.*/

  Ptr<ListPositionAllocator> posicionNodos = CreateObject<ListPositionAllocator> ();

  for (uint16_t i = 0; i < num_nodos_eNB; i++) {
    posicionNodos->Add (Vector (radio_cobertura_eNB*(2*i+1), 0, 0));
  }

  for (uint16_t i = 0; i < NUM_UE; i++) {
    posicionNodos->Add (Vector (posicion_UE, 0, 0));
  }

  // En nuestro escenario, los nodos estarán fijos en una posición.

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

  /* Instalamos la pila TCP/IP en el nodo UE, y les asignamos direcciones IP. La
  generación de rutas y unión con nodos eNB se hará dentro del bucle que configura
  las aplicaciones, para ser más cómodo. */

  internet.Install (nodosUE);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (dispLTE_UE));

  NS_LOG_INFO("Direcciones de nodos UE configuradas.");

  /* Generamos las rutas estáticas para el nodo UE. La única ruta a generar será la salida
  por defecto hacia el router que da acceso a los equipos remotos a través de Internet. 
  Tras ello, fijamos el nodo UE al último nodo eNB instalado. */

  Ptr<Node> nodoUE = nodosUE.Get (0);
  Ptr<Ipv4StaticRouting> rutaEstaticaUE = ipv4RoutingHelper.GetStaticRouting (nodoUE->GetObject<Ipv4> ());
  rutaEstaticaUE->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  lteHelper->Attach (dispLTE_UE.Get (0), dispLTEeNB.Get (num_nodos_eNB-1));

  NS_LOG_INFO("Nodos UE configurados y unidos a su nodo eNB.");

      // INSTALACIÓN DE APLICACIONES EN LOS EQUIPOS FINALES

  /* Para nuestro ejemplo, sólo trabarejos con el tráfico ascendente (de los UE al host remoto).
  La aplicación emisora estará en el nodo UE. Necesitará la dirección y puerto del equipo
  remoto. Dicho equipo remoto se limitará a escuchar peticiones en el puerto de subida. 
  Supondremos que el nodo UE siempre tiene tráfico que enviar. Por ello, utilizaremos un
  BulkSendApplication. */

  ApplicationContainer appsCliente;
  ApplicationContainer appsServidor;

  BulkSendHelper ulClientHelper ("ns3::TcpSocketFactory", 
                                      InetSocketAddress(dirEquipoRemoto, PTO_SUBIDA));
  appsCliente.Add (ulClientHelper.Install (nodoUE));

  PacketSinkHelper ulPacketSinkHelper ("ns3::TcpSocketFactory", 
                                      InetSocketAddress (Ipv4Address::GetAny (), PTO_SUBIDA));
  appsServidor.Add (ulPacketSinkHelper.Install (equipoRemoto));

  NS_LOG_INFO("Configuradas aplicaciones de tráfico ascendente.");

  /* Por último, fijamos el instante de inicio para las aplicaciones, determinado según una
  distribución aleatoria uniforme, con un rango de valores para el valor mínimo y máximo. */

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (1.0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (1.010));

  Time startTime = Seconds (startTimeSeconds->GetValue ());
  appsServidor.Start (startTime);
  appsCliente.Start (startTime);  

  NS_LOG_INFO("Aplicaciones configuradas.");

  // Fijamos una interfaz X2 para los nodos eNB para tareas de señalización entre ellos.

  lteHelper->AddX2Interface (nodoseNB);

  /* Instanciamos la variable Observador, que gestionará las trazas de transmisión y
  recepción de paquetes para el cómputo de las estadísticas. */

  Observador nodos_obs;
  nodos_obs.CapturaTrazas(appsCliente.Get(0), appsServidor.Get(0)); 

  NS_LOG_INFO("Observador instanciado con las aplicaciones y capturando trazas.");

  // Fijamos el instante de fin de la simulación.

  Simulator::Stop (Seconds (t_simulacion));

  NS_LOG_INFO("Fijado instante final de la simulación.");

  // Tras esto, iniciamos la simulación.

  NS_LOG_INFO("Iniciando simulación.");

  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_INFO("Fin de la simulación.");

  /* Devolvemos el porcentaje de paquetes correctos y la media del retardo de propagación,
  guardándolos en una variable de tipo ESTADISTICAS, que se devolverá al método principal. */

  double resultado_sim = nodos_obs.DevuelvePorcentajeCorrectos();

  NS_LOG_INFO("Valor del porcentaje de paquetes correctos: " << resultado_sim);

  /* Para acabar, reinicializamos los atributos del observador (incluyendo la estructura map, 
  que de tener algún valor, volverá a vaciarse). */

  nodos_obs.Inicializa();

  NS_LOG_INFO("Estadísticas inicializadas.");

  NS_LOG_INFO("Devolviendo valor del método RealizaSimulacion.");  

  return resultado_sim;
}



int main (int argc, char *argv[])
{
  NS_LOG_FUNCTION("Entrando en el método principal.");  

  // Parámetros de la simulación, con sus valores por defecto.

  double long_cobertura = LONG_COB;       // Longitud del tramo que se pretende cubrir.
  double t_simulacion[NUM_CURVAS] = {5.0, 8.0, 20.0};  // Tiempos de simulación para las pruebas.
  uint16_t iteraciones = NUM_ITERACIONES; // Número de iteraciones por punto de la gráfica.
  double posicion_UE = POS_UE;            // Posición inicial del nodo UE (se incrementará).
  double radio_cobertura_eNB[NUM_CURVAS] = {150, 140, 130}; /* Radio de cobertura de cada nodo 
                                                               eNB (en metros), para cada caso de 
                                                               tiempo de simulación. */
  double min_porcentaje[NUM_CURVAS] = {40, 50, 60};   /* Valor del porcentaje de paquetes correctos 
                                                         mínimo a cumplir, por cada caso del tiempo 
                                                         de simulación. */
  uint16_t num_nodos_eNB = NUM_ENB;       // Número de nodos eNB de partida (se incrementará).

  NS_LOG_INFO("Instanciadas e inicializadas las variables de simulación.");

  /* Antes de procesar los argumentos pasados por línea de comandos, vamos a modificar
  algunos atributos por defecto de clases que se utilizarán en el código para que tengan
  valores razonables. Al hacerlo antes, el usuario podrá cambiarlo si lo desea (por línea
  de comandos, por ejemplo). */

  /* Usaremos el modelo real para la señalización RRC (de gestión de recursos radio). 
  El resto de parámetros de los nodos eNB estarán en sus valores por defecto, que implica
  que se utilizarán modelo de pérdidas. Por ello; en los resultados obtenidos, veremos que
  nunca se llegará a un 100% de paquetes correctos debido a las pérdidas producidas por
  interferencias y otros fenómenos en el radioenlace. */

  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));

  /* Usaremos un modelo exponencial para las pérdidas en propagación en el radioenlace. El
  modelo por defecto utilizado era el modelo de Friis, pero daba problemas debido a que
  se tenía que llegar a distancias del orden del kilómetro para notar los efectos, y con
  cambios muy bruscos. Como pretendemos realizar un escenario a pequeña escala, para ver
  cómo empeora la situación al alejarse del nodo eNB, utilizaremos este modelo exponencial,
  con valor 3 de exponente de la fórmula de dicho modelo. */

  Config::SetDefault ("ns3::LteHelper::PathlossModel", 
                                   StringValue ("ns3::LogDistancePropagationLossModel"));
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::Exponent", DoubleValue (3));

  NS_LOG_INFO("Fin de configuración por defecto de atributos de clases usadas.");

  // Admitimos algunos de los parámetros anteriores por línea de comandos.

  CommandLine cmd;

  cmd.AddValue ("long_cobertura", "Longitud del tramo que se pretende cubrir (en metros)", 
                                                                               long_cobertura);
  cmd.AddValue ("iteraciones", "Número de iteraciones por punto de la gráfica", iteraciones);

  cmd.Parse (argc, argv);

  /* Si hemos introducido un valor de iteraciones menor que 2 (1, por ejemplo), no se
  puede calcular el intervalo de confianza porque no tendríamos un valor correcto de
  grados de libertad. Luego, si ocurre, le asignamos el valor por defecto. */

  if (iteraciones < 2)
    iteraciones = NUM_ITERACIONES;

  NS_LOG_INFO("Fin de tratamiento de variables de entorno.");

  /* Esta variable de tipo ACUMULADOR contendrá las 2 variables de tipo Average<double> que 
  almacenarán las estadísticas obtenidas de RealizaSimulacion. De esta forma, y realizando 
  un valor de "iteraciones" simulaciones, computaremos la media de los valores obtenidos y 
  hallaremos el intervalo de confianza para cada punto de la gráfica a calcular. */

  Average<double> acum_porcentaje_correctos;
  acum_porcentaje_correctos.Reset();

  NS_LOG_INFO("Acumuladores instanciados y reseteados.");

  /* En esta variable, se guardarán los distintos resultados de la simulación, que se
  irán añadiendo a la variable ACUMULADOR con un Update, para luego computar la media. */

  double resultado_sim;

  NS_LOG_INFO("Instanciada la variable que almacenará los resultados.");

  /* Ahora, comenzamos con las simulaciones, plasmando los resultados en curvas que
  instanciamos a continuación con los valores pedidos. */

  PLOT_ESTADISTICAS plot;
  plot.porcentaje_correctos.SetTitle ("Práctica 8. PORCENTAJE CORRECTOS. Grupo 4");
  plot.porcentaje_correctos.SetLegend ("posición_nodo_UE (m)", "porcentaje_correctos (%)");
  plot.numero_nodos_eNB.SetTitle ("Práctica 8. EVOLUCIÓN DEL NÚMERO DE NODOS ENB. Grupo 4");
  plot.numero_nodos_eNB.SetLegend ("posición_nodo_UE (m)", "número_nodos_eNB");

  std::string titulo_curva[NUM_CURVAS] = {"t_sim=5s", "t_sim=8s", "t_sim=20s"};    
  Gnuplot2dDataset curvas_porcentaje[NUM_CURVAS];
  Gnuplot2dDataset curvas_nodos_eNB[NUM_CURVAS];   /* Se generarán como tablas y no
                                                      como estructuras por si variase
                                                      el valor de NUM_CURVAS. */
  
  NS_LOG_INFO("Inicialización de plot e instanciación de curvas realizada.");

  /* Entramos en el bucle de simulación, que consta de: 

  while {

    for {
      Bucle en el que se realizan las "iteraciones" simulaciones. Después, se guardan
      los resultados en los acumuladores, para computar la media posteriormente.    
    }

    if {
      Cuando el porcentaje de paquetes correctos, en la media calculada tras la
      simulación considerada, baje de un 40%, debemos considerar, por calidad de servicio,
      el colocar un nuevo nodo eNB (recordar que nuestro objetivo es garantizar que
      el porcentaje de paquetes correctos nunca bajará de ese porcentaje).
      Notar que repetiríamos la simulación con el nodo UE colocado en la misma posición,
      pero añadiendo otro nodo eNB más, conectándose el nodo UE a ese nuevo nodo eNB.
    }

    else {
      En otro caso, se garantiza el porcentaje de paquetes correctos deseado. Luego
      podemos quedarnos con los valores obtenidos de las simulaciones, guardándolos
      en la curva correspondiente con Add. Además, incrementamos la posición del
      nodo UE. 
    }

    Ṕor último, reseteamos los acumuladores para dejarlos limpios para la nueva batería
    de simulaciones.
  }*/

  for (uint16_t i = 1; i < 2; i++) {
    NS_LOG_DEBUG("Generación de curva número: " << i+1);

    /* Establecemos el título de la curva (su nombre vendrá en función del tiempo de
    simulación utilizado), y fijamos el estilo y las líneas de error para el intervalo 
    de confianza. */

    curvas_porcentaje[i].SetTitle(titulo_curva[i]);
    curvas_porcentaje[i].SetStyle(Gnuplot2dDataset::LINES_POINTS);
    curvas_porcentaje[i].SetErrorBars (Gnuplot2dDataset::Y);   

    curvas_nodos_eNB[i].SetTitle(titulo_curva[i]);
    curvas_nodos_eNB[i].SetStyle(Gnuplot2dDataset::LINES_POINTS);

    NS_LOG_INFO("Gráfica " << i+1 << " preparada para las estadísticas.\n");

    while (posicion_UE <= long_cobertura) {
      NS_LOG_DEBUG("\nPosición del nodo UE: " << posicion_UE << " metros");
      NS_LOG_DEBUG("Tenemos " << num_nodos_eNB << " nodos eNB colocados\n");

      // Realizamos las "iteraciones" simulaciones, añadiendo los resultados a su acumulador.
      for (uint16_t j = 1; j <= iteraciones; j++) {
        NS_LOG_INFO("Simulación número: " << j);

        resultado_sim = RealizaSimulacion(t_simulacion[i], posicion_UE, radio_cobertura_eNB[i], 
                                        num_nodos_eNB);
        NS_LOG_UNCOND(resultado_sim);
        acum_porcentaje_correctos.Update(resultado_sim);

        NS_LOG_INFO("Realizada simulación número: " << j);
      }

      if (acum_porcentaje_correctos.Avg() < min_porcentaje[i]) {
        num_nodos_eNB++;
        NS_LOG_DEBUG("Se necesita un nuevo nodo eNB. Añadiendo un nodo eNB más.");
      }

      else {
        NS_LOG_DEBUG("\nValor medio del porcentaje de paquetes correctos (%): ");
        curvas_porcentaje[i].Add(posicion_UE, acum_porcentaje_correctos.Avg(), 
                                                        CalculaIC(acum_porcentaje_correctos));

        curvas_nodos_eNB[i].Add(posicion_UE, (double) num_nodos_eNB);

        posicion_UE += INCR_POS_UE;

        NS_LOG_INFO("\nCalculado punto de la curva con posición del nodo UE: " 
                                                                  << posicion_UE << " metros");
      }

      // Y reseteamos los acumuladores.

      acum_porcentaje_correctos.Reset();

      NS_LOG_INFO("Acumuladores reseteados.");
    }

    // Añadimos los puntos conseguidos a su respectivo plot.

    plot.porcentaje_correctos.AddDataset(curvas_porcentaje[i]);
    plot.numero_nodos_eNB.AddDataset(curvas_nodos_eNB[i]);

    NS_LOG_INFO("Se ha generado la curva " << i+1);
  }

  /* Finalmente, generamos el fichero, añadimos pause -1 para que no se cierre repentinamente
  al llamarlo con gnuplot, y cerramos el descriptor de fichero. */
  
  std::ofstream fichero_porcentaje("practica08-1.plt");
  std::ofstream fichero_nodos_eNB("practica08-2.plt");
  plot.porcentaje_correctos.GenerateOutput(fichero_porcentaje);
  plot.numero_nodos_eNB.GenerateOutput(fichero_nodos_eNB);
  fichero_porcentaje << "pause -1" << std::endl;
  fichero_nodos_eNB << "pause -1" << std::endl;
  fichero_porcentaje.close();
  fichero_nodos_eNB.close();

  NS_LOG_INFO("Ficheros generados. Fin del programa.");  

  return 0;
}