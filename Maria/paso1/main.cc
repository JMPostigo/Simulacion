/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/* 					Práctica 8 PyS de Redes					*/
/*				        Autor: Maria Valero   					*/


//   Hay  que poner "export 'NS_LOG=Observador=level_info'" para que se vean las trazas por pantalla


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
//#include "ns3/gtk-config-store.h"
#include "Observador.h"


using namespace ns3;

int main (int argc, char *argv[])
{	
  CommandLine cmd;
  cmd.Parse (argc, argv);
	
  // to save a template default attribute file run it like this:
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim
  //
  // to load a previously created default attribute file
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Se parsea para sobreescribir los valores por defecto de linea de comandos
  cmd.Parse (argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Descomentar la siguiente linea para habilitar el logging
  //  lteHelper->EnableLogComponents ();

  // Crear los nodos: eNodeB y UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (2);


  /* Aniadido de ejemploposti.cc
  //Registramos el dispositivo terminal (UE) en el nodo B dado (eNB)
  Ptr<EnbNetDevice> enbNetD = enbDevs.Get (0)->GetObject<EnbNetDevice> ();
  Ptr<UeNetDevice> ueNetD = ueDevs.Get (i)->GetObject<UeNetDevice> ();
  lteHelper.RegisterUeToTheEnb (ue, enb);  
  */

  // Se pone la movilidad
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);

  // Crea los dispositivos de red y los instala en los nodos (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  // Por defecto, el scheduler es PF, descomentar la siguiente linea para usar RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);
  
  // Se añade los netdevice UE al eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activa el data radio bearer entre el eNodeB y el UE
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);
  lteHelper->EnableTraces ();
  
  // Se pone el tiempod e parada
  Simulator::Stop (Seconds (1.05));
  
  // Llamamos al observador
  Observador obser (DynamicCast<LteUeNetDevice> (ueDevs.Get (0)), DynamicCast<LteEnbNetDevice> (enbDevs.Get (0)));

  // Iniciamos la simulacion
  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  return 0;
}




