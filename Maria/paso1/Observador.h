/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 					Práctica 8 PyS de Redes					*/
/*                                      Autor: Maria Valero                                     */

//   Hay  que poner "export 'NS_LOG=Observador=level_info'" para que se vean las trazas por pantalla

#include "ns3/packet.h"
#include "ns3/application.h"
#include "ns3/csma-module.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/average.h"
#include "ns3/nstime.h"
#include "ns3/applications-module.h"

#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-ue-net-device.h"

#include "ns3/lte-ue-rrc.h"

using namespace ns3;

class Observador       	
{
public:
  //Constructor de la clase
  Observador(Ptr<LteUeNetDevice>,Ptr<LteEnbNetDevice>);
  // Funcion que maneja la traza que indica la conexion entre el UE y el eNodeB
  void Conexion(unsigned long, unsigned short, unsigned short);
 
private:
 
  
  
};
