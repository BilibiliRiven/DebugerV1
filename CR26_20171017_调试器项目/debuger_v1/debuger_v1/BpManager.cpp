#include "stdafx.h"
#include "BpManager.h"


CBpManager::CBpManager(HANDLE handle)
{
	if (handle <= 0){
#ifdef DEBUG
		assert(0);
#endif // DEBUG
	}
	m_handle = handle;
}


CBpManager::~CBpManager()
{
}

//BOOL CBpManager::PBM_AddBreadPoint(LPVOID lpvAddr, CBreakPoint::BP_TYPE enBpType){
//	CBreakPoint * pBp = new CBreakPoint();
//	BOOL bRet = pBp->InitBP(m_handle, lpvAddr, enBpType);
//	if (bRet){
//		OutputDebugString(L"添加INT3断点成功\r\n");
//	}
//	else{
//		OutputDebugString(L"添加INT3断点失败\r\n");
//#ifdef DEBUG
//		assert(0);
//#endif // DEBUG
//	}
//	return bRet;
//}