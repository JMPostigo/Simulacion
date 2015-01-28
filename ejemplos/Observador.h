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

  /* Método de gestión de la traza de envío completo de un paquete por la aplicación origen. */
  void GestionaTrazaRxApp1(Ptr <const Packet> p, const Address & direccion);

  /* Método de gestión de la traza de recepción completa de un paquete en la aplicación
  destino. */
  void GestionaTrazaRxApp2(Ptr <const Packet> p, const Address & direccion);

  // Método que captura las trazas consideradas.
  void CapturaTrazas(Ptr<Application> rx1, Ptr<Application> rx2);

  // Método que devuelve el porcentaje de paquetes correctos en la aplicación destino.
  double DevuelvePorcentajeCorrectos1();
    double DevuelvePorcentajeCorrectos2();


private:

  // Aplicación transmisora en el escenario.
  Ptr<Application> receptor_1;

  // Aplicación receptora en el escenario.
  Ptr<Application> receptor_2;

  // Paquetes enviados por la aplicación origen.
  double pkts_recibidos1;
  
  // Paquetes recibidos por la aplicación destino.
  double pkts_recibidos2;


};
