#include "core/Common.h"
#include "OpStdLibs.h"
#include "OpForwardDeclare.h"
#include "event/Events.h"

#include "systems/VesselSystem.h"
#include "MainEngine.h"
#include "core/OrbitalHauler.h"
#include <sstream>







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
	doController(simt, simdt);
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

void MainEngine::doController(double simt, double simdt) {
	switch (currentMode) {
	case LANTR_MODE_OFF:
		//Controller is in standby, reduced power demand, only limited measurements
		if (targetMode == LANTR_MODE_ELECTRIC) {
			//Perform first step towards operation: Activate controller
			currentMode = LANTR_STATE_ACTIVATE_CONTROLLER;
		}
		return;
	case LANTR_STATE_ACTIVATE_CONTROLLER:
		break;
	case LANTR_STATE_CONTROLLER_BITE:
		break;
	case LANTR_STATE_NEUTRONDETECTOR_TEST:
		//Verify for 3 seconds, that the measured neutron flux never drops below minimum.
		break;
	case LANTR_MODE_SCRAM:
		break;
	default:
		targetMode = LANTR_MODE_SCRAM;
		//Log cause into reactor error log.
	}
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
	return (2.3 * getThermalPower() / JOULE_PER_FISSION + NEUTRON_SOURCE_FLUX) * DETECTOR_CONSTANT;
}

void MainEngine::receiveEvent(Event_Base* event, EVENTTOPIC topic) {

	if (*event == EVENTTYPE::SIMULATIONSTARTEDEVENT) {
		Olog::info("Main engine received sim started event!");
	}
}

const string MODE_OFF_TEXT = "OFF";
const string MODE_ELECTRIC_TEXT = "IDLE";
const string MODE_NTR_TEXT = "NTR";
const string MODE_LANTR_TEXT = "LANTR";
const string MODE_SCRAM_TEXT = "SCRAM";
const string MODE_ERROR_TEXT = "#ERROR";

const string& MainEngine::getModeAsText() const {
	switch (targetMode) {
	case LANTR_MODE_OFF:
		return MODE_OFF_TEXT;
	case LANTR_MODE_ELECTRIC:
		return MODE_ELECTRIC_TEXT;
	case LANTR_MODE_NTR:
		return MODE_NTR_TEXT;
	case LANTR_MODE_LANTR:
		return MODE_LANTR_TEXT;
	case LANTR_MODE_SCRAM:
		return MODE_SCRAM_TEXT;
	default:
		return MODE_ERROR_TEXT;;
	}
}


int MainEngine::getCurrentMode() const {
	return currentMode;
}

void MainEngine::setTargetMode(int mode) {
	//Check that this is a valid mode change
	//LANTR_MODE_SCRAM is possible anytime
	
	targetMode = mode;
}

void MainEngine::scram(char* cause) {
	if (targetMode != LANTR_MODE_SCRAM) {
		targetMode = LANTR_MODE_SCRAM;
		currentMode = LANTR_MODE_SCRAM;
		logAnomaly('X', cause);
	}
}

void MainEngine::logAnomaly(char type, char* cause) {
	REACTOR_ERROR_TYPE newAnomaly;
	newAnomaly.type = type;
	newAnomaly.mjd = oapiGetSimMJD();
	newAnomaly.confirmed = false;
	strncpy(newAnomaly.cause, cause, 39);
	errorLog.insert(errorLog.begin(), newAnomaly);
}

bool MainEngine::getError(int pos, REACTOR_ERROR_TYPE* entry) const {
	if (pos < errorLog.size()) {
		*entry = errorLog.at(pos);
		return true;
	}
	else {
		return false;
	}
}

int MainEngine::countErrors() const {
	return errorLog.size();
}