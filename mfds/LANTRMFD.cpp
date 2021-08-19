#include "LANTRMFD.h"
#include <Sketchpad2.h>
#include "core/OrbitalHauler.h"

using namespace std;


LANTRMFD::LANTRMFD(DWORD w, DWORD h, VESSEL* vessel)
	: MFD2(w, h, vessel)
{
	engine = ((OrbitalHauler*)vessel)->Powerplant();
}

int LANTRMFD::ButtonMenu(const MFDBUTTONMENU** menu) const {
	//No buttons right now
	return 0;
}

bool LANTRMFD::Update(oapi::Sketchpad* sketchpad) {
	char buffer[250];
	Title(sketchpad, "POWERPLANT");
	char* mode = "#NODATA";
	sprintf_s(buffer, 250, "MODE: %s", mode);
	sketchpad->Text(5, 20, buffer, strlen(buffer));
	sprintf_s(buffer, 250, "THERMAL: %5.1f kW", engine->getThermalPower() / 1000.0);
	sketchpad->Text(5, 40, buffer, strlen(buffer));
	sprintf_s(buffer, 250, "FLUX: %010.2g", engine->getNeutronFlux());
	sketchpad->Text(5, 60, buffer, strlen(buffer));
	return true;
}

int LANTRMFD::MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
		case OAPI_MSG_MFD_OPENEDEX: {
			MFDMODEOPENSPEC* ospec = (MFDMODEOPENSPEC*)wparam;
			return (int)(new LANTRMFD(ospec->w, ospec->h, (VESSEL*)lparam));
		}
	}
	return 0;
}