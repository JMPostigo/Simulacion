/*
*               Ejemplo simple de LTE
*
*/
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
//#include "ns3/gtk-config-store.h"

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

  // Se parsea para establecer los valores introducidos por linea de comandos
  cmd.Parse (argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Descomentar la siguiente linea para habilitar el logging
  //  lteHelper->EnableLogComponents ();

  // Creamos los nodos: eNodeB y UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);

  // Establecemos el modelo de mobilidad
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);

  // El scheduler por defecto es PF, descomentar la siguiente linea para poner RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  // Create Devices and Instalamos los nodos (eNB and UE) en el LTEhelper (devuvelven los NetDevice)
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Aniade UE al eNnodeB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activa el data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);
  lteHelper->EnableTraces ();

  // Ejecutamos la simulacion
  Simulator::Stop (Seconds (1.05));

  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  return 0;
}

