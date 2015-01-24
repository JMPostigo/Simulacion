/*
	Ejemplo montado a partir de 
	http://www.nsnam.org/docs/release/3.10/manual/html/lte.html#attributes
	
	COMENTARIO: Me da errores de compilación
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>

using namespace ns3;

int main (int argc, char *argv[])
{	
  	CommandLine cmd;
  	cmd.Parse (argc, argv);
	
	NodeContainer ueNodes;	//Dispositivo móvil
	NodeContainer enbNodes;	//Antena (NodoB)

	ueNodes.Create (1);
	enbNodes.Create (1);

	LteHelper lte;

	NetDeviceContainer ueDevs, enbDevs;
	//Creamos dispositivo LTE, las capas físicas DL(DownLink) y UL(UpLink) 
	//y las conecta a sus propios canales LTE
	ueDevs = lte.Install (ueNodes, LteHelper::DEVICE_TYPE_USER_EQUIPMENT);
	enbDevs = lte.Install (enbNodes, LteHelper::DEVICE_TYPE_ENODEB);

	//Instalamos la pila IP
	InternetStackHelper stack;
	stack.Install (ueNodes);
	stack.Install (enbNodes);
	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer UEinterfaces = address.Assign (ueDevs);
	Ipv4InterfaceContainer ENBinterface = address.Assign (enbDevs);

	//Registramos el dispositivo terminal (UE) en el nodo B dado (eNB)
	Ptr<EnbNetDevice> enb = enbDevs.Get (0)->GetObject<EnbNetDevice> ();
	Ptr<UeNetDevice> ue = ueDevs.Get (i)->GetObject<UeNetDevice> ();
	lte.RegisterUeToTheEnb (ue, enb);

	//Creamos un modelo de movilidad para cada dispositivo
	Ptr<ConstantPositionMobilityModel> enbMobility = new ConstantPositionMobilityModel ();
	enbMobility->SetPosition (Vector (0.0, 0.0, 0.0));
	lte.AddMobility (enb->GetPhy (), enbMobility);

	Ptr<ConstantVelocityMobilityModel> ueMobility = new ConstantVelocityMobilityModel ();
	ueMobility->SetPosition (Vector (30.0, 0.0, 0.0));
	ueMobility->SetVelocity (Vector (30.0, 0.0, 0.0));
	lte.AddMobility (ue->GetPhy (), ueMobility);

	//Definimos un conjunto de subcanales a usar para la transmisión
	//tanto de subida como de bajada
	std::vector<int> dlSubChannels;
	for (int i = 0; i < 25; i++)
	  {
	    dlSubChannels.push_back (i);
	  }
	std::vector<int> ulSubChannels;
	for (int i = 50; i < 100; i++)
	  {
	    ulSubChannels.push_back (i);
	  }

	enb->GetPhy ()->SetDownlinkSubChannels (dlSubChannels);
	enb->GetPhy ()->SetUplinkSubChannels (ulSubChannels);
	ue->GetPhy ()->SetDownlinkSubChannels (dlSubChannels);
	ue->GetPhy ()->SetUplinkSubChannels (ulSubChannels);

	//Definimos un canal
	lte.AddDownlinkChannelRealization (enbMobility, ueMobility, ue->GetPhy ());




	return 0;
}
