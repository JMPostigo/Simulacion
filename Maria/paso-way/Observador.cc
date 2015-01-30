/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*  PRÁCTICA 8. PLANIFICACIÓN Y SIMULACIÓN DE REDES
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
  pkts_enviados=0;
  pkts_recibidos=0;
  mac_paquetesEnviados = 0;
  mac_paquetesRecibidos = 0;

  NS_LOG_INFO("Fin del método Inicializa.");  
}

void Observador::CapturaTrazas(Ptr<Application> tx, Ptr<Application> rx,Ptr<LteUeNetDevice> net_ue,Ptr<NetDevice> pgw, Ptr<NetDevice> server) {
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
  
  // Capturamos las trazas de ConexionEstablished y HandoverStart
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("ConnectionEstablished",MakeCallback (&Observador::Conexion,this));
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("HandoverStart",MakeCallback (&Observador::NotifyHandoverStart,this));
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("HandoverEndOk",MakeCallback (&Observador::NotifyHandoverEnd,this));


  // Ponemos a capturar la traza "MacTx" para ver cuantos paquetes envia el netDevice
  pgw->TraceConnectWithoutContext ("MacTx",MakeCallback (&Observador::PacketSend,this));
  // Ponemos a capturar la traza "MacRx" para ver cuantos paquetes envia el netDevice
  server->TraceConnectWithoutContext ("MacRx",MakeCallback (&Observador::PacketReceive,this));

  NS_LOG_INFO("Fin del método CapturaTrazas.");  
}


/*       FUNCIONES QUE MANEJAN EVENTOS         */


void Observador::Conexion( uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  NS_LOG_INFO("Conexion establecida por el UE: " << imsi << " al CELLID " << cellid );
}

void Observador::NotifyHandoverStart (uint64_t imsi, uint16_t cellid, uint16_t rnti, uint16_t targetCellId)
{
  NS_LOG_INFO("Inicio de Handover entre : " << imsi << " al CELLID " << cellid );
}


void Observador::NotifyHandoverEnd (uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  NS_LOG_INFO("Fin de handover entre el UE: " << imsi << " al CELLID " << cellid );
}


// Manejador del evento "MacTx"
void Observador::PacketSend(Ptr<const Packet> p)
{
  //NS_LOG_INFO("Se acaba de mandar un paquete.");
  mac_paquetesEnviados ++;
}

// Manejador del evento "MacRx"
void Observador::PacketReceive(Ptr<const Packet> p)
{
  //NS_LOG_INFO("Se acaba de recibir un paquete.");
  mac_paquetesRecibidos ++;
}



void Observador::GestionaTrazaTxApp(Ptr <const Packet> p) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaTxApp.");

  /* Como se ha transmitido un paquete desde la aplicación origen, se incrementa el 
  contador de paquetes enviados. */

  pkts_enviados++;

  // Trazamos la traza correspondiente a la transmisión de un paquete con nivel LOGIC.
  NS_LOG_LOGIC("Paquete enviado número: " << pkts_enviados);  

  //NS_LOG_INFO("Fin del método GestionaTrazaTxApp.");  
}


void Observador::GestionaTrazaRxApp(Ptr <const Packet> p, const Address & direccion) {
  NS_LOG_FUNCTION("Entramos en el método GestionaTrazaRxApp.");

  /* Como ha llegado un paquete correcto a la aplicación destino, incrementamos el valor
  del contador de paquetes recibidos. */

  pkts_recibidos++;

  // Trazamos la traza correspondiente a la recepción de un paquete con nivel LOGIC.
  NS_LOG_LOGIC("Paquete recibido número: " << pkts_recibidos);  

  //NS_LOG_INFO("Fin del método GestionaTrazaRxApp.");  
}


/*   FUNCIONES PUBLICAS PARA DEVOLVER PARAMETROS    */

double Observador::DevuelvePorcentajeCorrectos() {
  NS_LOG_FUNCTION("Entramos en el método DevuelvePorcentajeCorrectos.");

  /* Devolvemos el porcentaje de paquetes correctos, que serán los paquetes correctos
  recibidos en la aplicación destino entre los paquetes enviados por la aplicación
  transmisora.

  Notar que dicho porcentaje está en %; por eso, multiplicamos por 100. */

  NS_LOG_INFO("Devolviendo valor del método DevuelvePorcentajeCorrectos.");  

  return (pkts_recibidos/pkts_enviados)*CONV_PORCENTAJE;
}

