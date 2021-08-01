#pragma once

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
};