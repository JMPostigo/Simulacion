/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 					Práctica 6 PyS de Redes					*/
/*				  José Manuel Postigo Aguilar					*/
/*					Fichero: Observador.h   						*/
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
#include "ns3/wifi-phy.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"


using namespace ns3;

class Observador       	
{
public:
  //Constructor de la clase
  Observador(Ptr<NetDevice>,Ptr<NetDevice>);
  void RecibeMac(Ptr<const Packet> pqt);
  void EnviaMac(Ptr<const Packet> pqt);
  // Funcion que comprueba si la estructura esta vacia
  //int Vacia();
  // Porcentaje de paquetes transmitidos
  int porcentTrans();
  // Vacia la tabla time_array, que contiene los tiempos de envío de los paquetes
  int Vacia();
  
  
  //Variable donde se va a ir almacenando el retardo de transmisión
    Average<double>   av_retardo;
  
  
  
private:
  // Guarda los tiempos de envio de cada paquete
  std::map <uint64_t, Time> time_array;
  int p_recib;
  int p_env;
  double tam_pqt_rx;
  Time inicio;
  Time duracion;
  int uid_paquete_recibido;

  
  
  
};
