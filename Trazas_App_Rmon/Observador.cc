/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*  PRÁCTICA 8. PLANIFICACIÓN Y SIMULACIÓN DE REDES. GRUPO 4
*     Observador.cc
*       -Ramón Pérez Hernández
*       -José Manuel Postigo Aguilar
*       -María Valero Campaña
*/

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/application.h"
#include "ns3/bulk-send-application.h"
#include "ns3/packet-sink.h"
#include "ns3/packet.h"
#include "ns3/average.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include <map>
#include "Observador.h"

#define  CONV_PORCENTAJE    100 // Factor de conversión de tanto por uno a %.

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
  map_t_envio.erase(map_t_envio.begin(), map_t_envio.end());
  acum_retardo_tx.Reset();
  pkts_enviados=0;
  pkts_recibidos=0;

  NS_LOG_INFO("Fin del método Inicializa.");  
}


void Observador::GestionaTrazaTxApp(Ptr <const Packet> p) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaTxApp.");

  /* Guardamos el instante en el que se envía un paquete completo, en función de su
  UID. */
//  map_t_envio[p->GetUid()]=Simulator::Now();

  /* Como se ha transmitido un paquete desde la aplicación origen, se incrementa el 
  contador de paquetes enviados. */

  pkts_enviados++;

  // Trazamos la traza correspondiente a la transmisión de un paquete con nivel LOGIC.
  NS_LOG_LOGIC("Paquete enviado número: " << pkts_enviados);  

  NS_LOG_INFO("Fin del método GestionaTrazaTxApp.");  
}


void Observador::GestionaTrazaRxApp(Ptr <const Packet> p, const Address & direccion) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaRxApp.");

  // Obtenemos el tiempo de envío correspondiente al UID del paquete.
 // std::map<uint64_t, Time>::iterator iter_t_envio = map_t_envio.find(p->GetUid());

  /* Comprobamos que se ha encontrado el tiempo a buscar. De no encontrarse, se
  trazará con nivel WARN, y no se hará nada más. */

 // if (iter_t_envio == map_t_envio.end())
 //   NS_LOG_WARN("No se ha encontrado el UID del paquete en la estructura map.");
    
 // else {
    /* Si se encuentra, guardamos en el acumulador la diferencia entre el tiempo 
    de envío y llegada.
    Con "second", accedemos a dicho valor de tiempo.*/
 //   acum_retardo_tx.Update((Simulator::Now()-iter_t_envio->second).GetSeconds());
    
    // Y borramos el tiempo de envío de la estructura. 
 //   map_t_envio.erase(iter_t_envio); 

    /* Como ha llegado un paquete correcto a la aplicación destino, incrementamos el valor
    del contador de paquetes recibidos. */

    pkts_recibidos++;

    // Trazamos la traza correspondiente a la recepción de un paquete con nivel LOGIC.
    NS_LOG_LOGIC("Paquete recibido número: " << pkts_recibidos);  
//  }

  NS_LOG_INFO("Fin del método GestionaTrazaRxApp.");  
}


void Observador::CapturaTrazas(Ptr<Application> tx, Ptr<Application> rx) {
  NS_LOG_FUNCTION("Entramos en el método CapturaTrazas.");

  /* Obtenemos las aplicaciones transmisora y receptora, para facilitar el trabajo 
  de captura de trazas. Ambas se pasan como Application. */

  transmisor = tx;
  receptor = rx;

  /* Las trazas a considerar serán: 
  -Tx (en BulkSendApplication), para detectar la transmisión de los paquetes enviados 
  por la aplicación origen.
  -Rx (en PacketSink), para detectar la recepción de los paquetes que llegan correctamente 
  a la aplicación destino. */

  transmisor->GetObject<BulkSendApplication>()->TraceConnectWithoutContext("Tx", 
                                    MakeCallback(&Observador::GestionaTrazaTxApp, this));      
  receptor->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx", 
                                    MakeCallback(&Observador::GestionaTrazaRxApp, this));    
  
  NS_LOG_INFO("Fin del método CapturaTrazas.");  
}


double Observador::DevuelveMediaRetardo() {
  NS_LOG_FUNCTION("Entramos en el método DevuelveMediaRetardo.");

  // Devolvemos la media del acumulador de tiempos (en segundos).
  NS_LOG_INFO("Devolviendo valor del método DevuelveMediaRetardo.");  

  return acum_retardo_tx.Avg();
}


std::map<uint64_t, Time> Observador::DevuelveEstrMap() {
  NS_LOG_FUNCTION("Entramos en el método DevuelveEstrMap.");

  // Devolvemos la estructura map.
  NS_LOG_INFO("Devolviendo valor del método DevuelveEstrMap.");  

  return map_t_envio;
}


double Observador::DevuelvePorcentajeCorrectos() {
  NS_LOG_FUNCTION("Entramos en el método DevuelvePorcentajeCorrectos.");

  /* Devolvemos el porcentaje de paquetes correctos, que serán los paquetes correctos
  recibidos en la aplicación destino entre los paquetes enviados por la aplicación
  transmisora.

  Notar que dicho porcentaje está en %; por eso, multiplicamos por 100. */

  NS_LOG_INFO("Devolviendo valor del método DevuelvePorcentajeCorrectos.");  

  return (pkts_recibidos/pkts_enviados)*CONV_PORCENTAJE;
}