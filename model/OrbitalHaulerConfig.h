#pragma once

struct OrbitalHaulerConfig
{
	LANTRConfig mainEngineConfig;
	ThrusterConfig rcsConfig;

	Oparse::OpModelDef GetModelDef();

};

