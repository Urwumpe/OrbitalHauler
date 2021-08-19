
#define STRICT
#define ORBITER_MODULE

#include "core/Common.h"
#include "OpStdLibs.h"
#include "Oparse.h"
#include "model/Models.h"
#include "event/Events.h"

#include "systems/VesselSystem.h"
#include "systems/mainengine/MainEngine.h"
#include "systems/rcs/ReactionControlSystem.h"
#include "systems/dockport/DockPort.h"

#include "core/OrbitalHauler.h"
#include "mfds/LANTRMFD.h"


using namespace Oparse;



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

OrbitalHauler::OrbitalHauler(OBJHANDLE hVessel, int flightmodel) : VESSEL4(hVessel, flightmodel) { 
	registerPowerplantMFD();
}

OrbitalHauler::~OrbitalHauler() {
	
	// Delete vessel systems
	for (const auto& it : systems) {
		delete it;
	}

}

void OrbitalHauler::registerPowerplantMFD() {
	static char* name = "POWERPLANT";
	MFDMODESPECEX spec;
	spec.name = name;
	spec.key = OAPI_KEY_P;
	spec.context = NULL;
	spec.msgproc = LANTRMFD::MsgProc;
	Olog::info("Registering Powerplant MFD");
	RegisterMFDMode(spec);
}

void OrbitalHauler::clbkSetClassCaps(FILEHANDLE cfg) {

	Olog::setLogLevelFromFile(cfg);

	// Load vessel config
	OrbitalHaulerConfig config = OrbitalHaulerConfig();
	OpModelDef modelDef = config.GetModelDef();

	PARSINGRESULT result = ParseFile(cfg, modelDef, "OrbitalHauler.cfg");
	if (result.HasErrors()) {
		Olog::error((char*)result.GetFormattedErrorsForFile().c_str());
		throw std::runtime_error("Errors in OrbitalHauler config, see log for details!");
	}

	phLO2 = CreatePropellantResource(TUG_LO2TANK_MAXIMUM_MASS);
	phLH2 = CreatePropellantResource(TUG_LH2TANK_MAXIMUM_MASS);

	// Initialise vessel systems
	systems.push_back(mainEngine = new MainEngine(this, config.mainEngineConfig,  phLH2, phLO2));
	systems.push_back(new ReactionControlSystem(config.rcsConfig, this));
	systems.push_back(new DockPort(this));

	for (const auto& it : systems) {
		it->init(eventBroker);
	}

	// Event will be propagated in first clbkPreStep
	eventBroker.publish(EVENTTOPIC::GENERAL, new SimpleEvent(EVENTTYPE::SIMULATIONSTARTEDEVENT));

}


void OrbitalHauler::clbkPreStep(double  simt, double  simdt, double  mjd) {
	
	// Propagate due events.
	// This should always remain at the beginning of clbkPreStep and never be called anywhere else.
	eventBroker.processEvents();

}

MainEngine* OrbitalHauler::Powerplant() const {
	return mainEngine;
}



