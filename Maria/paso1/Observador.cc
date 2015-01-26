/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 					Práctica 8 PyS de Redes					*/
/*				        Autor: Maria Valero   					*/

//   Hay  que poner "export 'NS_LOG=Observador=level_info'" para que se vean las trazas por pantalla

// Se ha capturado la traza que indica la conexion entre el eNodeB y el UE

#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Observador");

// Lo de WifiNetDevice es lo que no va
Observador::Observador( Ptr<LteUeNetDevice> net_ue, Ptr<LteEnbNetDevice> net_enb)
{
  
  NS_LOG_INFO("Entramos en el constructor");

  // Capturamos la traza de la conexion entre UE y eNodeB
  net_ue->GetRrc ()-> TraceConnectWithoutContext ("ConnectionEstablished",MakeCallback (&Observador::Conexion,this));
  
}

void Observador::Conexion(unsigned long uno, unsigned short dos, unsigned short tres)
{
  NS_LOG_INFO("Conexion establecida" );
}



