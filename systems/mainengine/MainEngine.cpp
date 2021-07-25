#include "core/Common.h"
#include "systems/VesselSystem.h"
#include "MainEngine.h"
#include "core/OrbitalHauler.h"


MainEngine::MainEngine(OrbitalHauler* vessel, PROPELLANT_HANDLE phLH2, PROPELLANT_HANDLE phLO2) : VesselSystem(vessel) {
	mode = LANTR_MODE_OFF;
	this->phLH2 = phLH2;
	this->phLO2 = phLO2;
}

MainEngine::~MainEngine() {}


void MainEngine::init() {
	Olog::trace("Main engine init");

	// Create the propellant tank.
	// TODO: If we're going to do anything complex with tanks, they should probably become an own VesselSystem instance.
	double propMass = 20000;
	
	
	// Create the thruster
	// TODO: Some of this should be configurable in the CFG.

	VECTOR3 pos = { 0, 0, -5 };
	VECTOR3 dir = { 0, 0, 1 };
	double thrust = 67000;
	double isp = 9221;
	
	THRUSTER_HANDLE thrustHandle = vessel->CreateThruster(pos, dir, thrust, propHandle, isp);

	vessel->CreateThrusterGroup(&thrustHandle, 1, THGROUP_MAIN);
	vessel->AddExhaust(thrustHandle, 8, 1, pos, dir * -1);

}