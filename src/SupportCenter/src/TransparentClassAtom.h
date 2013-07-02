#pragma once

#include "AidLib\Strings\tstring.h"

class CTransparentClassAtom
{
public:
	CTransparentClassAtom(HINSTANCE Instance);
	~CTransparentClassAtom(void);
private:
	ATOM CreateWinClass(void);

	tstring className;
	HINSTANCE hInstance;
};
