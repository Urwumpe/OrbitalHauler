#pragma once
#include "Oparse.h"

struct ThrusterConfig {
	double isp;
	double thrust;

	Oparse::OpModelDef GetModelDef();
};

struct LANTRConfig {
	double maxBraytonCyclePressure;
	double maxReactorChamberPressure;
	double controlDrumAbsorptionEffect;
	double controlDrumReflectionEffect;
	double initialFuelEnrichment;
	/* Convective heat transfer coefficient for the fuel rod
	 * cooled by gaseous hydrogen
	 */
	double h_cladding;
	/*
	* Convective heat transfer coefficient for the reactor equipment
	* Cooled by liquid hydrogen 
	*/
	double h_equipment;
	/* Convective heat transfer cofficient for the nozzle
	*  Inner wall to LH2, LH2 to outer wall.
	 */
	double h_nozzle;
	/*
	 * Convective heat transfer coefficient for the radiators
	 * LH2 to tubes.
	 */
	double h_radiator;

	Oparse::OpModelDef GetModelDef();
};