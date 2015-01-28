
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 					Práctica 8 PyS de Redes					*/
/*				        Autor: Maria Valero   					*/

//   Hay  que poner "export 'NS_LOG=Observador=level_info'" para que se vean las trazas por pantalla

// Se han capturado trazas del eNodeB, el UE, server y pgw

#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Observador");

// Lo de WifiNetDevice es lo que no va
Observador::Observador( Ptr<LteUeNetDevice> net_ue, Ptr<LteEnbNetDevice> net_enb,Ptr<NetDevice> pgw, Ptr<NetDevice> server)
{
  
  NS_LOG_INFO("Entramos en el constructor");

  // Capturamos la traza de la conexion entre UE y eNodeB
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("ConnectionEstablished",MakeCallback (&Observador::Conexion,this));
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("HandoverStart",MakeCallback (&Observador::NotifyHandoverStart,this));
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("HandoverEndOk",MakeCallback (&Observador::NotifyHandoverEnd,this));

  //Ptr< LteEnbRrc > aux =  net_enb->GetRrc ();
  //aux-> TraceConnectWithoutContext ("ConnectionEstablished",MakeCallback (&Observador::Conexion,this));

    // Ponemos a capturar la traza "MacTx" para ver cuantos paquetes envia el netDevice
  pgw->TraceConnectWithoutContext ("MacTx",MakeCallback (&Observador::PacketSend,this));
  // Ponemos a capturar la traza "MacRx" para ver cuantos paquetes envia el netDevice
  server->TraceConnectWithoutContext ("MacRx",MakeCallback (&Observador::PacketReceive,this));

}

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

void Observador::Intento (Ptr<Packet> pqt)
{
  NS_LOG_INFO("  ------------ Intentoooooo ---------" );
}

// Manejador del evento "MacTx"
void Observador::PacketSend(Ptr<const Packet> p)
{
  NS_LOG_INFO("Se acaba de mandar un paquete.");
  m_paquetesEnviados ++;
}

// Manejador del evento "MacRx"
void Observador::PacketReceive(Ptr<const Packet> p)
{
  NS_LOG_INFO("Se acaba de recibir un paquete.");
  m_paquetesRecibidos ++;
}



