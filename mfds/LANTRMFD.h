#pragma once
#include <MFDAPI.h>
#include "systems/mainengine/MainEngine.h"
#include <map>
#include <string>

class LANTRMFD : public MFD2
{
	MainEngine* engine;
	map<int, string> state_labels;
	void initStateLabels();
	void renderErrorMessages(oapi::Sketchpad* sketchpad);
public:
	LANTRMFD(DWORD w, DWORD h, VESSEL* vessel);

	virtual int ButtonMenu(const MFDBUTTONMENU** menu) const;
	virtual char* ButtonLabel(int bt);
	virtual bool ConsumeButton(int bt, int event);
	virtual bool ConsumeKeyBuffered(DWORD key);
	virtual bool Update(oapi::Sketchpad* sketchpad);

	static int MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
};

