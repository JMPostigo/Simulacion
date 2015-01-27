/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// Observador.h . Por Ramón Pérez Hernández.

#include "ns3/application.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/double.h"

using namespace ns3;

class Observador
{
public:

  // Constructor de la clase.
  Observador();

  // Método para resetear los atributos del observador.
  void Inicializa();

  /* Método de gestión de la traza de envío completo de un paquete por la estación
  WiFi transmisora. */
  void GestionaTrazaTxWifi(Ptr <const Packet> p);

  /* Método de gestión de la traza de recepción completa de un paquete en la estación 
  WiFi receptora. */
  void GestionaTrazaRxWifi(Ptr <const Packet> p);

  /* Método de gestión de la traza de recepción completa de un paquete en la aplicación
  destino. */
  void GestionaTrazaRxApp(Ptr <const Packet> p, const Address & direccion);

  // Método que captura las trazas consideradas.
  void CapturaTrazas(Ptr<Application> tx, Ptr<Application> rx);

  // Método que devuelve el porcentaje de paquetes correctos en la estación WiFi receptora.
  double DevuelvePorcentajeCorrectos();

  // Método que devuelve el total de bits recibidos por la aplicación destino.
  double DevuelveBitsRxApp();

private:

  // Aplicación transmisora en el escenario.
  Ptr<Application> transmisor;

  // Aplicación receptora en el escenario.
  Ptr<Application> receptor;

  /* Paquetes que se envían (contando tanto correctos como tirados) por la estación
  WiFi transmisora. */
  double pkts_enviados_wifi;
  
  // Paquetes correctos que llegan a la estación WiFi receptora.
  double pkts_correctos_wifi;

  // Número de octetos que llegan a la aplicación destino.
  uint32_t octetos_rx_app;
};
