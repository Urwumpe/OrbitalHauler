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
	shaftSpeed = 0.0;
	accuMols = 400.0;
	timer = 0.0;
	functionInit = false;
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
	calculatePrimaryLoop(simt, simdt);
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
	if (timer > 0.0) {
		timer = max(0.0, timer - simdt);
	}

	switch (currentMode) {
	case LANTR_MODE_OFF:
		//Controller is in standby, reduced power demand, only limited measurements
		//If neutron detector senses higher than 5E5 flux, raise an alert
		onTargetGoto(LANTR_MODE_ELECTRIC, LANTR_STATE_ACTIVATE_CONTROLLER);
		break;
	case LANTR_STATE_ACTIVATE_CONTROLLER:
		onTargetGoto(LANTR_MODE_ELECTRIC, LANTR_STATE_CONTROLLER_BITE);
		break;
	case LANTR_STATE_CONTROLLER_BITE:
		initTimer(5.0);
		initEnd();
		if(timerDone()) onTargetGoto(LANTR_MODE_ELECTRIC, LANTR_STATE_NEUTRONDETECTOR_TEST);
		break;
	case LANTR_STATE_NEUTRONDETECTOR_TEST:
		//Verify for 3 seconds, that the measured neutron flux never drops below minimum.
		initTimer(3.0);
		initEnd();
		if(timerDone()) onTargetGoto(LANTR_MODE_ELECTRIC, LANTR_STATE_CIRCULATE_COOLANT);
		break;
	case LANTR_STATE_CIRCULATE_COOLANT:
		//Watchdog timer set to 60 seconds
		initTimer(60.0);
		initEnd();
		//Modulate globe valve to raise pressure in loop
		if (getPrimaryLoopInP() < 0.25E6) {
			//Open valve to raise pressure (~ 50 kPa/s)
		}
		else {
			//Close valve
		}
		//Open ball valves
		//Start compressor at low power for ventilation

		//If the coolant loop does not reach conditions for startup in 60 seconds abort the start
		//Downmode to OFF
		if (timerDone()) downMode("PRILOOP_FAILURE", LANTR_MODE_OFF, LANTR_MODE_OFF);
		if(getPrimaryLoopInP() > 0.2E6) 
			onTargetGoto(LANTR_MODE_ELECTRIC, LANTR_STATE_PREHEAT_CORE);
		//TODO Might leave some loose ends. Goto downmode entry point. 
		onTargetGoto(LANTR_MODE_OFF, LANTR_MODE_OFF);
		break;
	case LANTR_MODE_SCRAM:
		break;
	default:
		scram("ILLEGAL_FUNCTION");
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

void MainEngine::calculatePrimaryLoop(double simt, double simdt) {
	//First step: Calculate flow resistance
	//Assume it to be zero initially.
	double loopDensity = HEXE_MOLAR_MASS * primaryLoopMols  / PRIMARY_LOOP_VOLUME;
	//Simplified function: In reality more complicated
	double totalMassFlow = loopDensity * shaftSpeed * PRIMARY_LOOP_COMP_DIAMETER / HEXE_REFERENCE_RPM;
	primaryLoop5.massflow = totalMassFlow;
	primaryLoop5.P = heXeCompressorPressureCoeff(shaftSpeed) * primaryLoop4a.P;
	primaryLoop5.T = pow(heXeCompressorPressureCoeff(shaftSpeed), HEXE_GAMMA / (HEXE_GAMMA-1.0)) * primaryLoop4a.T;
}

double MainEngine::getChamberPressure() const {
	//TODO Implement me
	return 0.0;
}
double MainEngine::getPrimaryLoopInP() const {
	return primaryLoop9.P;
}

double MainEngine::getPrimaryLoopOutletT() const {
	return primaryLoop1.T;
}

double MainEngine::getPrimaryLoopInletT() const {
	return primaryLoop9.T;
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
	functionInit = false;
	targetMode = mode;
}

void MainEngine::scram(char* cause) {
	if (targetMode != LANTR_MODE_SCRAM) {
		functionInit = false;
		targetMode = LANTR_MODE_SCRAM;
		currentMode = LANTR_MODE_SCRAM;
		logAnomaly('X', cause);
	}
}

void MainEngine::downMode(char* cause, int newMode, int entryPoint) {
	functionInit = false;
	targetMode = newMode;
	currentMode = entryPoint;
	logAnomaly('E', cause);
}

void MainEngine::onTargetGoto(int targetMode, int nextFunction) {
	if (targetMode == this->targetMode) {
		functionInit = false;
		currentMode = nextFunction;
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

bool MainEngine::getError(unsigned int pos, REACTOR_ERROR_TYPE* entry) const {
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

double MainEngine::heXeCompressorPressureCoeff(double shaftSpeed) const {
	return 2.9 * shaftSpeed / HEXE_REFERENCE_RPM;
}

/*
* Only steady state right now, no inertia by structural parts/walls
*/
void MainEngine::doHeatTransfer(double eff, GasFlow& flow1, GasFlow& flow2, double simdt) {
	double c1 = flow1.massflow * flow1.heatcap;
	double c2 = flow2.massflow * flow2.heatcap;
	double T1 = c1 * flow1.T;
	double T2 = c2 * flow2.T;

	double dT = T1 - T2;

	T1 = T1 - 0.5 * dT * eff;
	T2 = T2 + 0.5 * dT * eff;

	flow1.T = T1 / c1;
	flow2.T = T2 / c2;

}