#pragma once

using namespace std;

#include <cmath>
#include "model/ThrusterConfig.h"
#include "systems/VesselSystem.h"
#include "event/Events.h"

const double RPM = 2.0 * PI / 60.0;

const int LANTR_MODE_OFF		= 0;
const int LANTR_MODE_ELECTRIC	= 100;
const int LANTR_MODE_NTR		= 200;
const int LANTR_MODE_LANTR		= 300;
const int LANTR_MODE_SCRAM		= 1000;

const int LANTR_STATE_ACTIVATE_CONTROLLER = 51;
const int LANTR_STATE_CONTROLLER_BITE = 52;
const int LANTR_STATE_NEUTRONDETECTOR_TEST = 53;
const int LANTR_STATE_CIRCULATE_COOLANT = 60;
const int LANTR_STATE_PREHEAT_CORE = 70;
const int LANTR_STATE_ENTER_CRITICALITY = 90;

const double RATED_THERMAL_POWER = 555.0E6;
const double RATED_PEAK_TEMPERATURE = 2700.0;

const double HEXE_MOLAR_MASS = 40.0;
//Assumption: Monoatomic gas
const double HEXE_GAMMA = 5.0 / 3.0;
//Standard heat capacity of HeXe coolant in J/kg
const double HEXE_HEATCAPACITY_PER_MASS = 14304.0;		



const double HEXE_REFERENCE_RPM = 53000.0 * RPM;

/* Maximum pressure at which the Brayton cycle hardware operates. 
 * At higher chamber pressure, the valves are closed and the Brayton cycle powered only 
 * by the heated LH2 from the gamma shield recuperator. 
 */
const double LANTR_BRAYTON_CYCLE_MAX_PRESSURE = 1.0E6;

/* Energy gain by a fission event in Joule (Ws)
 * Can be used for calculating the neutron flux based on a set power level
 */
const double JOULE_PER_FISSION = 3.2E-11;
const double DETECTOR_SIZE = pow(0.01, 2);
//Distance from Reactor core center to detector
const double DETECTOR_DISTANCE = 2.5;
const double DETECTOR_CONSTANT = DETECTOR_SIZE / pow(DETECTOR_DISTANCE, 2);
//Create a weak neutron flux with a neutron source
const double NEUTRON_SOURCE_FLUX = 5.0E9;

//TODO: Scale that it fits "real"-world values better.
const double PRIMARY_LOOP_VOLUME = 2.0;
//Primary loop compressor diameter - 13 cm in reference model.
const double PRIMARY_LOOP_COMP_DIAMETER = 0.13;

class OrbitalHauler;

struct REACTOR_ERROR_TYPE {
	char type;
	bool confirmed;
	double mjd;
	char cause[40];
};

struct GasFlow {
	//specific heat capacity
	double heatcap;
	//Temperature
	double T;
	//Pressure
	double P;		
	//Mass flow (kg/s)
	double massflow;
};

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
 * The fuel is made of Uranium Carbide, U2C3, which melts at 2500°C. It decomposes to UC and UC2 crystals above 1800°C and returns to U2C3 when exposed to vacuum at 1400°C (recycling)
 * The density of the fuel is 13.04 g/cm3. The fuel uses a carbon weight ratio of 6% - 94% of the mass is Uranium, resulting in a mix of U2C3 and few UC grains below 1400°C.
 * 
 * 
 * Possible Failure Mode: Water vapor returning into the combustion chamber after shutdown can result in fuel element erosion, because of an exothermic reaction forming UO2, 
 * hydrocarbons (methane) and hydrogen, degrading fuel density, damaging the fuel rod surface (cracks!) and impurities in the cooling loop.
 * 
 * Gas impurities will form in the H2 loops during reactor operation (0.1% of the gaseous elements produced in the reactor fuel). No idea yet, if this could be a problem in Brayton operation.
 * During NTR mode operations, theses gasses should be ejected into space with the exhaust. 
 * These gases are radioactive, making their inclusion in H2 tank pressurization a potential health risk for the crew. A cyclone filter should be enough to reduce this risk.
 * 
 * Reaction rate = Number of particles in volume * Neutron Flux * Cross section.
 * 
 * Simplification: We control Reactor power directly (assuming a fast computer controls the power based on neutron flux), burnup is linear to reactor power.
 * In this case, we could ignore most of the neutronics (neutron flux is proportional to power level), the changing composition of the fuel could be derived from the burn up, 
 * Iodide and Xenon could be estimated from the power level. Xenon level would then just scale the speed how fast power can ramp up.
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
	int targetMode;
	/*
	* 
	*/
	int currentMode;

	/*
	* The current thermal power of the reactor, 1.0 = 100% rated power. 
	*/
	double thermalPowerLevel;

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

	//From reactor to NTR mode heat exchanger
	GasFlow primaryLoop1;
	//From NTR mode heat exchanger to turbine bypass valve
	GasFlow primaryLoop2; 
	//From turbine bypass to turbine
	GasFlow primaryLoop3a;
	//From turbine bypass to turbine mixer
	GasFlow primaryLoop3b;
	//From turbine to turbine mixer
	GasFlow primaryLoop4a;
	//From turbine mixer to radiator bypass valve
	GasFlow primaryLoop5;
	//From radiator bypass to radiator
	GasFlow primaryLoop6a;
	//From radiatior bypass to radiator mixer
	GasFlow primaryLoop6b;
	//From radiator to radiator mixer
	GasFlow primaryLoop7;
	//From radiator mixer to compressor
	GasFlow primaryLoop8;
	//From compressor to junction
	GasFlow primaryLoop9;
	//From junction to compressor
	GasFlow primaryLoop10a;
	//From junction to accu charger checkvalve
	GasFlow primaryLoop10b;
	//From pressurization globe valve to radiator mixer
	GasFlow primaryLoop11;
	//Content of the Accumulator in mols (40g / mol -  400 mol in accu at startup)
	double accuMols;

	double primaryLoopMols;

	double shaftSpeed; // (rad / s)

	/* Number of absorbed neutrons in this timestep
	 */
	double neutronsAbsorbed;
	/* Propellant resource for hydrogen, only consumed for propulsion, ACS or venting
	 */
	PROPELLANT_HANDLE phLH2;
	/* Propellant resource for LO2, engine consumes this only for propulsion.
	 */
	PROPELLANT_HANDLE phLO2;

	THRUSTER_HANDLE thNTR;
	THRUSTER_HANDLE thLANTR;

	const LANTRConfig &configuration;

	vector<REACTOR_ERROR_TYPE> errorLog;

	bool functionInit;

	double timer;

	inline void initTimer(double delay) {
		if (delay <= 0.0)
			return;

		if (!functionInit) {
			timer = delay;
		}
	}

	inline bool timerDone() {
		return (timer == 0.0);
	}
	
	inline void initEnd() {
		if(!functionInit)
			functionInit = true;
	}

	void onTargetGoto(int targetMode, int nextFunction);

	double heXeCompressorPressureCoeff(double shaftSpeed) const;


	void doHeatTransfer(double eff, GasFlow& side1, GasFlow& side2, double simdt);
public:
	MainEngine(OrbitalHauler *vessel, const LANTRConfig &config, PROPELLANT_HANDLE phLH2, PROPELLANT_HANDLE phLO2);
	~MainEngine();

	void init(EventBroker& eventBroker);

	/*
	 * calculations based on reactor state
	 * @sa VesselSystem::preStep 
	 */
	virtual void preStep(double simt, double simdt, double mjd);

	/**
	* Get the thermal power of the reactor in Watt. 
	* 
	* Reference model uses about 500 MW thermal power.
	*/
	double getThermalPower() const;

	/* Read the chamber pressure within the nuclear reactor module. Can be up to 136 atm in the reference.
	 * Range: [0.0 ... 7 MPa]
	 */
	double getChamberPressure() const;


	/**
	 * Read the chamber temperature of the reactor coolant. 
	 * Range: [0.0 ... 2800K]
	 */
	double getChamberTemperature() const; 
	/* Get the position of the nozzle plug valve
	 * 0.0 - closed
	 * 1.0 - open
	 */
	float getNozzlePlugValvePosition() const;
	/* Measurement of the neutron flux towards the gamma shield as measurement of the reactor reaction rate.
	 * 0.0 - minimum measurement, likely damaged sensor
	 * TBD - maximum nominal power level
	 * TBD - sensor limit
	 */
	double getNeutronFlux() const;
	/* Get the coefficient of criticality (alpha). Should be around 1.0 to be stable.
	*/
	double getCriticality() const;

	double getPrimaryLoopInP() const;
	double getPrimaryLoopOutletT() const;
	double getPrimaryLoopInletT() const;

	const string& getModeAsText() const;

	int getCurrentMode() const;

	void setTargetMode(int mode);

	void scram(char* cause = "USER");
	void downMode(char* cause, int newmode, int entryPoint);

	void logAnomaly(char type, char* cause);

	bool getError(unsigned int pos, REACTOR_ERROR_TYPE* entry) const;

	int countErrors() const;
protected:
	virtual void receiveEvent(Event_Base* event, EVENTTOPIC topic);
	void createDefaultPropellantLoad();

	void doAbsorptionReactions(double simt, double simdt);
	void doDecayReactions(double simt, double simdt);
	void doController(double simt, double simdt);
	void calculatePrimaryLoop(double simt, double simdt);
	
};

