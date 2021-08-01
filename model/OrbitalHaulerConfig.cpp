#include "OpStdLibs.h"
#include "Oparse.h"

#include "Models.h"


using namespace Oparse;

OpModelDef OrbitalHaulerConfig::GetModelDef() {
	return OpModelDef() = {
		{"lantr", { _Model<LANTRConfig>(mainEngineConfig), { _REQUIRED() } } },
		{"rcs_power", { _Model<ThrusterConfig>(rcsConfig), { _REQUIRED() } } }
	};
}
