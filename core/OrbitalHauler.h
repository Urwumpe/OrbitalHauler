#pragma once

#include <vector>
#include <VesselAPI.h>
#include <systems/VesselSystem.h>
#include <event/Events.h>

using namespace std;

const unsigned int TUG_NUMBER_LH2_TANKS = 6;
/* Maximum mass of the LH2 inside the tank. Six tanks in the spacecraft.
 */
const double TUG_LH2TANK_MAXIMUM_MASS = TUG_NUMBER_LH2_TANKS * 5000.0;
/* Maximum mass of the LO2 inside the tank, based on the LUNOX shuttle concept, assuming a MR of 3.0 for high-DV maneuvers.
 */
const double TUG_LO2TANK_MAXIMUM_MASS = 30000.0;

class MainEngine;


class OrbitalHauler : public VESSEL4 {
public:
	OrbitalHauler(OBJHANDLE hVessel, int flightmodel);
	~OrbitalHauler();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPreStep(double  simt, double  simdt, double  mjd);
	MainEngine* Powerplant() const;
private:
	MainEngine* mainEngine;

	vector<VesselSystem*> systems;
	/* Six cylindrical LH2 tanks around the propellant tank section. Can be individually enabled or isolated.
	 * Single point refueling is possible over the sump tank.
	 * Propellant tanks leaks could be a possible failure mode.
	 */
	PROPELLANT_HANDLE phLH2;
	/* There is only a single central LO2 tank, since it is not flight critical - return to any point of safety should be possible on LH2 only.
	 */
	PROPELLANT_HANDLE phLO2;

	EventBroker eventBroker;

	void registerPowerplantMFD();
};