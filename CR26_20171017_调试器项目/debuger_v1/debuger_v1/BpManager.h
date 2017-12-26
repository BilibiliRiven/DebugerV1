#pragma once
#include "BreakPoint.h"
class CBpManager
{
	HANDLE m_handle;
	//CList<CBreakPoint *, CBreakPoint *> m_List;
public:
	CBpManager(HANDLE handle);
	//BOOL PBM_AddBreadPoint(LPVOID lpvAddr, CBreakPoint::BP_TYPE enBpType);
	~CBpManager();
};

