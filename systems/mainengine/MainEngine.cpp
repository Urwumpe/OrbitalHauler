#include "core/Common.h"
#include "OpStdLibs.h"
#include "OpForwardDeclare.h"
#include "event/Events.h"

#include "systems/VesselSystem.h"
#include "model/ThrusterConfig.h"
#include "MainEngine.h"
#include "core/OrbitalHauler.h"


LANTRConfig::LANTRConfig() {
	controlDrumAbsorptionEffect = 0.1;
	controlDrumReflectionEffect = 1.5;
	maxReactorChamberPressure = 15E6;
	initialFuelEnrichment = 0.5;
	maxBraytonCyclePressure = LANTR_BRAYTON_CYCLE_MAX_PRESSURE;
}

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
	
	THRUSTER_HANDLE thrustHandle = vessel->CreateThruster(pos, dir, thrust, phLH2, isp);

	vessel->CreateThrusterGroup(&thrustHandle, 1, THGROUP_MAIN);
	vessel->AddExhaust(thrustHandle, 8, 1, pos, dir * -1);

}

void MainEngine::preStep(double simt, double simdt, double mjd) {

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