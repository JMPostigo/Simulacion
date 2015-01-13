/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 					Práctica 6 PyS de Redes					*/
/*				  José Manuel Postigo Aguilar					*/
/*				Fichero: Observador.cc 						*/
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Observador");

// Lo de WifiNetDevice es lo que no va
Observador::Observador( Ptr<NetDevice> net_ue, Ptr<NetDevice> net_enb)
{
  p_recib = 0;
  p_env = 0;
  tam_pqt_rx = 0;
  uid_paquete_recibido = 0;
  av_retardo.Reset ();
  time_array.erase(time_array.begin(),time_array.end());
  
  NS_LOG_INFO("Entramos en el constructor");

  // Las trazas para los NetDevice, se captura cuando un apquete pasa por la capa mac
//  net_ue->GetMac ()->TraceConnectWithoutContext ("MacTx",MakeCallback (&Observador::EnviaMac,this));
//  net_enb->GetMac ()->TraceConnectWithoutContext ("MacRx",MakeCallback (&Observador::RecibeMac,this));

  
}

void Observador::EnviaMac(Ptr<const Packet> pqt)
{
  p_env ++;
  NS_LOG_INFO("jojojo Send " << pqt->GetUid () );
}

void Observador::RecibeMac(Ptr<const Packet> pqt)
{
  p_recib ++;
  uid_paquete_recibido = pqt->GetUid ();
  NS_LOG_INFO("jijiji Rec " << uid_paquete_recibido );
}


int Observador::Vacia()
{
  int num = time_array.size();
  time_array.erase(time_array.begin(),time_array.end());
  return num;
}

int Observador::porcentTrans()
{
  return ((100*((double)p_recib))/p_env);
}


