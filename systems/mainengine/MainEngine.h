#pragma once

const int LANTR_MODE_OFF = 0;
const int LANTR_MODE_ELECTRIC = 1;
const int LANTR_MODE_NTR = 2;
const int LANTR_MODE_LANTR = 3;

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
	/* Propellant resource for hydrogen, only consumed for propulsion, ACS or venting.
	 */
	PROPELLANT_HANDLE phLH2;
	/* Propellant resource for LO2, engine consumes this only for propulsion.
	 */
	PROPELLANT_HANDLE phLO2;
public:
	MainEngine(OrbitalHauler *vessel, PROPELLANT_HANDLE phLH2, PROPELLANT_HANDLE phLO2);
	~MainEngine();

	void init();

	/* Read the chamber pressure within the nuclear reactor module.
	 */
	double GetChamberPressure() const;
};

