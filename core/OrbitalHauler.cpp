
#define STRICT
#define ORBITER_MODULE

#include "core/Common.h"
#include "systems/VesselSystem.h"
#include "systems/mainengine/MainEngine.h"
#include "systems/rcs/ReactionControlSystem.h"
#include "systems/dockport/DockPort.h"

#include "core/OrbitalHauler.h"



// Module Init and cleanup
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	return new OrbitalHauler(hvessel, flightmodel);
}

DLLCLBK void ovcExit(VESSEL* vessel)
{
	if (vessel) delete (OrbitalHauler*)vessel;
}

DLLCLBK void InitModule(HINSTANCE hModule) {
	// Set default log level here.
	Olog::loglevel = OLOG_INFO;
	Olog::assertlevel = OLOG_DEBUG;
	Olog::projectName = "Orbital Hauler";
	Olog::debug("Initialising Module.");
}


// Vessel class

OrbitalHauler::OrbitalHauler(OBJHANDLE hVessel, int flightmodel) : VESSEL4(hVessel, flightmodel) { }

OrbitalHauler::~OrbitalHauler() {
	
	// Delete vessel systems
	for (const auto& it : systems) {
		delete it;
	}

}

void OrbitalHauler::clbkSetClassCaps(FILEHANDLE cfg) {

	Olog::setLogLevelFromFile(cfg);
	Olog::info("Log level set to %i", Olog::loglevel);

	//Define propellant resources in the order how they are read in the scenario file:
	for (unsigned int i = 0; i < TUG_NUMBER_LH2_TANKS; ++i) {
		phLH2[i] = CreatePropellantResource(TUG_LH2TANK_MAXIMUM_MASS);
	}
	phLO2 = CreatePropellantResource(TUG_LO2TANK_MAXIMUM_MASS);
	phLH2Sump = CreatePropellantResource(500.0);

	// Initialise vessel systems
	systems.push_back(new MainEngine(this, phLH2Sump, phLO2));
	systems.push_back(new ReactionControlSystem(this));
	systems.push_back(new DockPort(this));

	for (const auto& it : systems) {
		it->init();
	}
}



