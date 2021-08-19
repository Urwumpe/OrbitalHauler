#pragma once
#include <MFDAPI.h>
#include "systems/mainengine/MainEngine.h"

class LANTRMFD : public MFD2
{
	MainEngine* engine;
public:
	LANTRMFD(DWORD w, DWORD h, VESSEL* vessel);

	virtual int ButtonMenu(const MFDBUTTONMENU** menu) const;
	bool Update(oapi::Sketchpad* sketchpad);

	static int MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);
};

