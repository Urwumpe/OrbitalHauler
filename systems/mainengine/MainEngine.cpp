#include "core/Common.h"
#include "OpStdLibs.h"
#include "OpForwardDeclare.h"
#include "event/Events.h"

#include "systems/VesselSystem.h"
#include "MainEngine.h"
#include "core/OrbitalHauler.h"



MainEngine::MainEngine(OrbitalHauler* vessel, const LANTRConfig &config, PROPELLANT_HANDLE phLH2, PROPELLANT_HANDLE phLO2) : VesselSystem(vessel), configuration(config) {
	targetMode = LANTR_MODE_OFF;
	currentMode = LANTR_MODE_OFF;
	this->phLH2 = phLH2;
	this->phLO2 = phLO2;
	thermalPowerLevel = 0.0;
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
	doAbsorptionReactions(simt, simdt);
	doDecayReactions(simt, simdt);
	//Use Newtons Law of cooling for calculating the heat transfers
	
}

void MainEngine::doAbsorptionReactions(double simt, double simdt) {
	//Calculate what happens to neutrons absorbed inside the fuel rods.
	//1. How many hit U235 atoms?
	//2. How many hit Xenon? etc.
	//3. Only calculate the changes in the atom population inside the fuels rods by absorbtion.
	//4. Decay and fission reactions are handled in another function.
	//5. Absorption of neutrons increases the internal energy of the fuel.
	//6. Also calculate the absorption of neutrons hitting the control drums. This only heats the control drums.
	//7. The control drums slowly age over time and become less efficient and corroded.
}

double MainEngine::getThermalPower() const {
	return thermalPowerLevel * RATED_THERMAL_POWER;
}

double MainEngine::getChamberTemperature() const {
	return 3.0 + thermalPowerLevel * (RATED_PEAK_TEMPERATURE - 3.0);
}

void MainEngine::doDecayReactions(double simt, double simdt) {
	//Decay elements in fuel pellets and control drums.
	//(Maybe include other materials in the core later)

}

double MainEngine::getChamberPressure() const {
	//TODO Implement me
	return 0.0;
}

double MainEngine::getNeutronFlux() const {
	//Must still get improved a lot
	//Currently represents prompt neutrons from fuel and neutron source
	return (2.3 * getThermalPower() / JOULE_PER_FISSION) * DETECTOR_CONSTANT + NEUTRON_SOURCE_FLUX;
}

void MainEngine::receiveEvent(Event_Base* event, EVENTTOPIC topic) {

	if (*event == EVENTTYPE::SIMULATIONSTARTEDEVENT) {
		Olog::info("Main engine received sim started event!");
	}
}