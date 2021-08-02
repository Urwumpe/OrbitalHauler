#include "core/Common.h"
#include "OpStdLibs.h"
#include "OpForwardDeclare.h"
#include "event/Events.h"

#include "systems/VesselSystem.h"
#include "MainEngine.h"
#include "core/OrbitalHauler.h"

MainEngine::MainEngine(OrbitalHauler* vessel, const LANTRConfig &config, PROPELLANT_HANDLE phLH2, PROPELLANT_HANDLE phLO2) : VesselSystem(vessel), configuration(config) {
	mode = LANTR_MODE_OFF;
	this->phLH2 = phLH2;
	this->phLO2 = phLO2;
}

MainEngine::~MainEngine() {}


void MainEngine::init(EventBroker& eventBroker) {
	Olog::trace("Main engine init");

	// create event subscriptions
	eventBroker.subscribe((EventSubscriber*)this, EVENTTOPIC::GENERAL);


	// Create the propellant tank.

	
	// Create the thruster
	// TODO: Some of this should be configurable in the CFG.

	VECTOR3 pos = { 0, 0, -5 };
	VECTOR3 dir = { 0, 0, 1 };
	double thrust = 67000;
	double isp = 9221;
	
	thNTR = vessel->CreateThruster(pos, dir, thrust, phLH2, isp);
	thLANTR = vessel->CreateThruster(pos, dir, thrust, phLH2, isp);

	vessel->CreateThrusterGroup(&thNTR, 1, THGROUP_MAIN);
	vessel->AddExhaust(thNTR, 8, 1, pos, dir * -1);

}

void MainEngine::preStep(double simt, double simdt, double mjd) {
	//Use Newtons Law of cooling for calculating the heat transfers

}

double MainEngine::GetChamberPressure() const {
	//TODO Implement me
	return -1.0;
}

double MainEngine::GetNeutronFlux() const {
	//TODO Implement me
	return 0.0;
}

void MainEngine::receiveEvent(Event_Base* event, EVENTTOPIC topic) {

	if (*event == EVENTTYPE::SIMULATIONSTARTEDEVENT) {
		Olog::info("Main engine received sim started event!");
	}
}