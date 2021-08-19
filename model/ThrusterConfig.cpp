#include "OpStdLibs.h"
#include "Oparse.h"

#include "ThrusterConfig.h"

using namespace Oparse;

OpModelDef ThrusterConfig::GetModelDef() {
	return OpModelDef() = {
		{ "isp", { _Param(isp), { _REQUIRED() } } },
		{ "thrust", { _Param(thrust), { _REQUIRED() } } }
	};
}

OpModelDef LANTRConfig::GetModelDef() {
	return OpModelDef() = {
		{ "enrichment", { _Param(initialFuelEnrichment), { _REQUIRED() } } },
		{ "absorption", { _Param(controlDrumAbsorptionEffect), { _REQUIRED() } } }
	};
}