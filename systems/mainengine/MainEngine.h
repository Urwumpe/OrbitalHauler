#pragma once

#include "model/ThrusterConfig.h"

const int LANTR_MODE_OFF		= 0;
const int LANTR_MODE_ELECTRIC	= 100;
const int LANTR_MODE_NTR		= 200;
const int LANTR_MODE_LANTR		= 300;

/* Maximum pressure at which the Brayton cycle hardware operates. 
 * At higher chamber pressure, the valves are closed and the Brayton cycle powered only 
 * by the heated LH2 from the gamma shield recuperator. 
 */
const double LANTR_BRAYTON_CYCLE_MAX_PRESSURE = 5.0E6;

/* Implementation of a LANTR type main engine
 * There is no throttle function, since there is no need for it. Engine can be either off or 100% on.
 * The spacecraft computer shall control most burns and automatically select the best engine mode.
 * Nuclear reaction rate depends mostly on hydrogen flow, self-regulating by negative void coefficient.
 * Control drums around the reactor are used to maintain even burn down of nuclear fuel pellets.
 * Wet H2 vapor at boiling point is provided for tank pressurization.
 * Wet O2 vapor at boiling point is provided for tank pressurization.
 * 
 * O2 tank can be used for resupplying ECLSS tanks.
 * Wet GH2 vapor can be provided to an attitude control system before passing through the radiators.
 * O2 mixture ratio in LANTR mode COULD be variable and controlled by the crew within a small range to control fuel economy (higher thrust vs lower ISP)
 * Optimal mixture ratio for lunar missions is approx. at MR=3.0.
 * 
 * The amount of neutrons generated inside core should be proportional to the thermal power generation in Watt.
 * Control drum position should weakly influence the amount of neutrons escaping the core.
 * The amount of neutrons absorbed should depend on a function of control drum position and H2 (moderator) flow.
 * Absorption also depends on temperature (~sqrt(T/20°))
 * The amount of neutrons absorbed by the fuel is the utilization factor, this should be a function of reactor design and fuel burn down.
 * The hotter the hydrogen, the weaker it is as absorber or moderator, reducing its cross section and the reaction rate.
 */
class MainEngine :
    public VesselSystem
{
	/* Operation mode for the engine controller, commanded by crew or ship AI
	 * LANTR_MODE_OFF		Engine cold, fully shut down (nuclear refueling)
	 * LANTR_MODE_ELECTRIC	Reactor providing electrical power only. Constant TCGA shaft speed.
	 * LANTR_MODE_NTR		Nuclear thermal rocket mode
	 * LANTR_MODE_LANTR		O2 injection, requires a higher minimum chamber pressure
	 */
	int mode;

	/* Controls the flow from the reactor into the nozzle. Opens automatically at 15 MPa (unless stuck).
	 * 0.0 fully closed, no flow, no thrust
	 * 1.0 fully opened, full thrust as by chamber pressure/nozzle function
	 */
	float throatValve;
	/* Bypass valve position for the turbo-compressor/generator assembly. Controlled by engine controller.
	 * 0.0f = fully closed
	 * 1.0f = fully opened
	 */
	float TCGA_bypass;
	/* Bypass valve position for the H2 turbopump assembly. Controlled by engine controller.
	 * 0.0f = fully closed
	 * 1.0f = fully opened
	 */
	float H2TPA_bypass;
	/* Bypass valve position for the O2 turbopump assembly. Controlled by engine controller.
	 * 0.0f = fully closed
	 * 1.0f = fully opened
	 */
	float O2TPA_bypass;

	/* The Hot LH2 valve controls the flow rate of heated LH2 into ACS and radiators. 
	 * -1.0		Use only Brayton cycle LH2
	 *  0.0		Fully closed, reactor isolated
	 *  1.0		Use only gamma shield LH2
	 */
	float HotLH2_valve;
	/* This valves controls how much LH2 can be redirected into the nozzle radiator.
	 * 0.0		All flow bypasses nozzle radiator
	 * 1.0		All flow passes nozzle radiator
	 */
	float nozzleLH2_valve;
	/* Use the electric pump to feed LH2 from the tank for replenishing Brayton cycle LH2
	 * or conditioning NTR components for burn.
	 */
	bool electricPumpEnabled;
	/* Temperature of the reactor hardware (control drum drives, valves)
	 */
	double tempReactorHW;
	/* Temperature of the reactor fuel rods
	 */
	double tempReactor;
	/* Temperature of the gamma shield. Heated by engine radiation (IR, neutron flux)
	 */
	double tempGammaShield;
	/* Propellant resource for hydrogen, only consumed for propulsion, ACS or venting.
	 */
	PROPELLANT_HANDLE phLH2;
	/* Propellant resource for LO2, engine consumes this only for propulsion.
	 */
	PROPELLANT_HANDLE phLO2;

	THRUSTER_HANDLE thNTR;
	THRUSTER_HANDLE thLANTR;

	const LANTRConfig &configuration;
public:
	MainEngine(OrbitalHauler *vessel, const LANTRConfig &config, PROPELLANT_HANDLE phLH2, PROPELLANT_HANDLE phLO2);
	~MainEngine();

	void init(EventBroker& eventBroker);

	/*
	 * calculations based on reactor state
	 * @sa VesselSystem::preStep 
	 */
	virtual void preStep(double simt, double simdt, double mjd);

	/* Read the chamber pressure within the nuclear reactor module. Can be up to 136 atm in the reference.
	 * Range: [0.0 ... 15 MPa]
	 */
	double GetChamberPressure() const;
	/* Get the position of the nozzle plug valve
	 * 0.0 - closed
	 * 1.0 - open
	 */
	float GetNozzlePlugValvePosition() const;
	/* Measurement of the neutron flux towards the gamma shield as measurement of the reactor reaction rate.
	 * 0.0 - minimum measurement, likely damaged sensor
	 * TBD - maximum nominal power level
	 * TBD - sensor limit
	 */
	double GetNeutronFlux() const;
	/* Get the coefficient of criticality (alpha). Should be around 1.0 to be stable.
	*/
	double GetCriticality() const;

protected:
	virtual void receiveEvent(Event_Base* event, EVENTTOPIC topic);
};

