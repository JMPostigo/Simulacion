/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// Observador.cc . Por Ramón Pérez Hernández.

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/application.h"
#include "ns3/net-device.h"
#include "ns3/wifi-net-device.h"
#include "ns3/packet-sink.h"
#include "ns3/wifi-phy.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "Observador.h"

#define  CONV_PORCENTAJE    100 // Factor de conversión de tanto por uno a %.
#define  CONV_BITS            8 // Factor de conversión de B a b.

using namespace ns3;

// Admitimos el uso de trazas.

NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador()
{
  NS_LOG_FUNCTION("Entramos en el constructor de la clase Observador.");

  // Inicializamos el valor de los atributos a 0 con Inicializa.

  Inicializa();

  NS_LOG_INFO("Fin del constructor.");
}


void Observador::Inicializa() {
  NS_LOG_FUNCTION("Entramos en el método Inicializa.");

  // Inicializamos el valor de los atributos a 0.

  transmisor=NULL;
  receptor=NULL;
  pkts_enviados_wifi=0;
  pkts_correctos_wifi=0;
  octetos_rx_app=0;

  NS_LOG_INFO("Fin del método Inicializa.");  
}


void Observador::GestionaTrazaTxWifi(Ptr <const Packet> p) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaTxWifi.");

  // Como se ha enviado un paquete, incrementamos la variable de paquetes enviados.
  pkts_enviados_wifi++;

  // Trazamos la traza correspondiente al envío del paquete con nivel LOGIC.
  NS_LOG_LOGIC("Paquete enviado por el transmisor Wifi número " << pkts_enviados_wifi);  

  NS_LOG_INFO("Fin del método GestionaTrazaTxWifi."); 
}


void Observador::GestionaTrazaRxWifi(Ptr <const Packet> p) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaRxWifi.");

  // Como ha llegado un paquete correcto, incrementamos su contador correspondiente.
  pkts_correctos_wifi++;

  // Trazamos la traza correspondiente a la recepción del paquete con nivel LOGIC.
  NS_LOG_LOGIC("Paquete recibido por el receptor Wifi número " << pkts_correctos_wifi);  

  NS_LOG_INFO("Fin del método GestionaTrazaRxWifi.");  
}


void Observador::GestionaTrazaRxApp(Ptr <const Packet> p, const Address & direccion) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaRxApp.");

  /* Como ha llegado un paquete correcto a la aplicación destino, le sumamos al contador
  de octetos el tamaño del paquete recibido. */

  octetos_rx_app += p->GetSize();

  // Trazamos la traza correspondiente a la recepción de octetos correctos con nivel LOGIC.
  NS_LOG_LOGIC("Se han recibido " << p->GetSize() << " B. Total: " << octetos_rx_app << " B.");  

  NS_LOG_INFO("Fin del método GestionaTrazaRxApp.");  
}


void Observador::CapturaTrazas(Ptr<Application> tx, Ptr<Application> rx) {
  NS_LOG_FUNCTION("Entramos en el método CapturaTrazas.");

  /* Obtenemos las aplicaciones transmisora y receptora, para facilitar el trabajo de captura de
  trazas. Ambas se pasan como Application. */

  transmisor = tx;
  receptor = rx;

  /* Para capturar las trazas a tratar, se convierte cada aplicación usando GetObject en el tipo adecuado,
  a la vez que se utilizan los métodos Get para llegar a la clase que contenga la traza requerida.
  Se tratarán las siguientes trazas:

  -PhyTxBegin: captura todo paquete que comience a enviar la estación WiFi transmisora, tanto si se tiran
  como si se envían por el canal, y será gestionado con el método GestionaTrazaTxWifi. Se hace así porque
  pueden haber paquetes que se tiran y que hubiesen interesado al receptor. Si hubiese más equipos en la
  red, esta consideración no sería correcta, porque un paquete que se tira podría ir a un equipo que no
  fuese el observado (pero no es el caso).
  -PhyRxEnd: recepción completa del paquete en la estación WiFi receptora, que será gestionado con el 
  método GestionaTrazaRxWifi. 
  -Rx: recepción del paquete completo en la aplicación destino, que será gestionado con el método 
  GestionaTrazaRxApp. */

  transmisor->GetNode()->GetDevice(0)->GetObject<WifiNetDevice>()->GetPhy()->TraceConnectWithoutContext("PhyTxBegin", 
                                            MakeCallback(&Observador::GestionaTrazaTxWifi, this));  
  receptor->GetNode()->GetDevice(0)->GetObject<WifiNetDevice>()->GetPhy()->TraceConnectWithoutContext("PhyRxEnd", 
                                            MakeCallback(&Observador::GestionaTrazaRxWifi, this));    
  receptor->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx", 
                                            MakeCallback(&Observador::GestionaTrazaRxApp, this));    
  
  NS_LOG_INFO("Fin del método CapturaTrazas.");  
}


double Observador::DevuelvePorcentajeCorrectos() {
  NS_LOG_FUNCTION("Entramos en el método DevuelvePorcentajeCorrectos.");

  /* Devolvemos el porcentaje de paquetes correctos, que serán los paquetes correctos
  recibidos en el receptor entre los paquetes enviados por el transmisor.

  Notar que dicho porcentaje está en %; por eso, multiplicamos por 100. */

  NS_LOG_INFO("Devolviendo valor del método DevuelvePorcentajeCorrectos.");  

  return (pkts_correctos_wifi/pkts_enviados_wifi)*CONV_PORCENTAJE;
}


double Observador::DevuelveBitsRxApp() {
  NS_LOG_FUNCTION("Entramos en el método DevuelveBitsRxApp.");

  /* Devolvemos los bits que se han recibido en el receptor. Para ello, simplemente,
  devolvemos los octetos recibidos y los multiplicamos por 8 para convertirlo a bits.
  Se hace así para poder comparar el resultado obtenido con la tasa presente, ya
  que estamos usando tasas en Mbps, y no MBps. 
  Se debe hacer un cast de entero a double, ya que octetos_rx_app es un entero. */

  NS_LOG_INFO("Devolviendo valor del método DevuelveBitsRxApp.");  

  return (double) octetos_rx_app*CONV_BITS;
}
