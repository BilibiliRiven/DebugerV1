#pragma once
#include <windows.h>
class CDebugCore
{
	
public:
	CDebugCore();
	~CDebugCore();
	BOOL DbgPE(CString & cstrPath);
};

