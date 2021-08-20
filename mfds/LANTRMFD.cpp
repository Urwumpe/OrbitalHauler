#include "LANTRMFD.h"
#include <Sketchpad2.h>
#include "core/OrbitalHauler.h"

using namespace std;


LANTRMFD::LANTRMFD(DWORD w, DWORD h, VESSEL* vessel)
	: MFD2(w, h, vessel)
{
	engine = ((OrbitalHauler*)vessel)->Powerplant();
	initStateLabels();
}

int LANTRMFD::ButtonMenu(const MFDBUTTONMENU** menu) const {
	static const MFDBUTTONMENU mnu[5] = {
		{"Stop", 0, 'S'},
		{"Start", 0, 'A'},
		{"NTR", 0, 'N'},
		{"LANTR", 0, 'K'},
		{"SCRAM", 0, 'X'}
	};
	if (menu) *menu = mnu;
	return 5;
}

char* LANTRMFD::ButtonLabel(int bt)
{
	char* label[5] = { "OFF", "ON", "NTR", "LAN", "SCR"};
	return (bt < 5 ? label[bt] : 0);
}

bool LANTRMFD::ConsumeButton(int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[5] = { OAPI_KEY_S, OAPI_KEY_A, OAPI_KEY_N, OAPI_KEY_K, OAPI_KEY_X };
	if (bt < 5) return ConsumeKeyBuffered(btkey[bt]);
	else return false;
}

bool LANTRMFD::ConsumeKeyBuffered(DWORD key)
{

	switch (key) {
	case OAPI_KEY_S:
		//Stop reactor
		engine->setTargetMode(LANTR_MODE_OFF);
		return true;
	case OAPI_KEY_A:
		engine->setTargetMode(LANTR_MODE_ELECTRIC);
		return true;
	case OAPI_KEY_N:
		engine->setTargetMode(LANTR_MODE_NTR);
		return true;
	case OAPI_KEY_K:
		engine->setTargetMode(LANTR_MODE_LANTR);
		return true;
	case OAPI_KEY_X:
		engine->scram("CREW COMMAND");
		return true;
	}
	return false;
}

const string LABEL_STATE_OFF = "STANDBY";
const string LABEL_STATE_ACTIVATE_CONTROLLER = "ACTIVATE_CONTROLLER";
const string LABEL_STATE_CONTROLLER_BITE = "CONTROLLER_BITE";
const string LABEL_STATE_NEUTRONDETECTOR_TEST = "NEUTRON_DETECTOR_TEST";

bool LANTRMFD::Update(oapi::Sketchpad* sketchpad) {
	char buffer[250];
	int baseX2 = GetWidth() / 2;

	Title(sketchpad, "POWERPLANT");
	sketchpad->SetFont(GetDefaultFont(0));
	sketchpad->SetTextColor(GetDefaultColour(0));
	sprintf_s(buffer, 250, "MODE: %s", engine->getModeAsText().c_str());
	sketchpad->Text(5, 20, buffer, strlen(buffer));
	int mode = engine->getCurrentMode();
	if (state_labels.find(mode) != state_labels.end()) {
		sprintf_s(buffer, 250, "STATE: (%04d) %s", mode, state_labels.at(mode).c_str());
	}
	else {
		sprintf_s(buffer, 250, "STATE: (%04d)", mode);
	}
	sketchpad->Text(5, 40, buffer, strlen(buffer));

	sprintf_s(buffer, 250, "THERMAL: %5.1f kW", engine->getThermalPower() / 1000.0);
	sketchpad->Text(5, 80, buffer, strlen(buffer));
	sprintf_s(buffer, 250, "FLUX: %010.2g", engine->getNeutronFlux());
	sketchpad->Text(5, 100, buffer, strlen(buffer));
	sprintf_s(buffer, 250, "TEMP: %06.1f K", engine->getChamberTemperature());
	sketchpad->Text(5, 120, buffer, strlen(buffer));

	sprintf_s(buffer, 250, "IN P: %06.3f kPa", engine->getPrimaryLoopInP() / 1000.0);
	sketchpad->Text(baseX2, 80, buffer, strlen(buffer));
	sprintf_s(buffer, 250, "IN T: %06.3f K", engine->getPrimaryLoopInletT());
	sketchpad->Text(baseX2, 100, buffer, strlen(buffer));


	renderErrorMessages(sketchpad);

	return true;
}

int MJDToDayOfYear(double mjd) {
	double dayPart = 0.0;
	double timePart = modf(mjd, &dayPart);
	return (int)floor(fmod(dayPart - 33282.0, 365));
}

int MJDToGMT(double mjd) {
	
	double dayPart = 0.0;
	double timePart = modf(mjd, &dayPart);
	int hours = (int)floor(timePart * 24);
	int minutes = (int)floor((timePart - hours / 24.0) * 24 * 60);
	return hours * 100 + minutes;
}

void LANTRMFD::renderErrorMessages(oapi::Sketchpad* sketchpad) {
	int baseY = this->GetHeight() - 40;
	int lines = min(4, engine->countErrors());
	char buffer[250];
	
	for (int i = 0; i < lines; i++) {
		REACTOR_ERROR_TYPE error;
		if (engine->getError(i, &error)) {
			sprintf_s(buffer, 250, "%c %03d %04d %20s %c",
				error.type,
				MJDToDayOfYear(error.mjd),
				MJDToGMT(error.mjd),
				error.cause,
				error.confirmed ? 'X' : ' ');
			sketchpad->Text(4, baseY + 20 * i, buffer, strlen(buffer));
		}
	}
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

void LANTRMFD::initStateLabels() {
	state_labels.insert(std::make_pair(LANTR_MODE_OFF, LABEL_STATE_OFF));
	state_labels.insert(std::make_pair(LANTR_MODE_SCRAM, "SCRAM"));

}