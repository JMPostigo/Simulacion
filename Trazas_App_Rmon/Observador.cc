/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// Observador.cc . Por Ramón Pérez Hernández.

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/application.h"
#include "ns3/net-device.h"
#include "ns3/bulk-send-application.h"
#include "ns3/packet-sink.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "Observador.h"

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
  pkts_enviados=0;
  pkts_recibidos=0;
  porcentaje_correctos=0;

  NS_LOG_INFO("Fin del método Inicializa.");  
}


void Observador::GestionaTrazaTxApp(Ptr <const Packet> p) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaTxApp.");

  // Como se ha enviado un paquete, incrementamos la variable de paquetes enviados.
  pkts_enviados++;

  // Trazamos la traza correspondiente al envío del paquete con nivel LOGIC.
  NS_LOG_LOGIC("Paquete enviado por la aplicación origen número " << pkts_enviados);  

  NS_LOG_INFO("Fin del método GestionaTrazaTxApp."); 
}


void Observador::GestionaTrazaRxApp(Ptr <const Packet> p, const Address & direccion) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaRxApp.");

  /* Como ha llegado un paquete correcto a la aplicación destino, incrementamos el valor
  del contador de paquetes recibidos. */

  pkts_recibidos++;

  // Trazamos la traza correspondiente a la recepción de octetos correctos con nivel LOGIC.
  NS_LOG_LOGIC("Se han recibido " << p->GetSize() << " B. Llevamos: " << pkts_recibidos << " paquetes.");  

  NS_LOG_INFO("Fin del método GestionaTrazaRxApp.");  
}


void Observador::CapturaTrazas(Ptr<Application> tx, Ptr<Application> rx) {
  NS_LOG_FUNCTION("Entramos en el método CapturaTrazas.");

  /* Obtenemos las aplicaciones transmisora y receptora, para facilitar el trabajo de captura de
  trazas. Ambas se pasan como Application. */

  transmisor = tx;
  receptor = rx;

  transmisor->GetObject<BulkSendApplication>()->TraceConnectWithoutContext("Tx", 
                                            MakeCallback(&Observador::GestionaTrazaTxApp, this));     
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

  return (pkts_recibidos/pkts_enviados)*100;
}