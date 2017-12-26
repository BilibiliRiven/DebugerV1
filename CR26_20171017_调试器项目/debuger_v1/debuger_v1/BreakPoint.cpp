#include "stdafx.h"
#include "BreakPoint.h"


//CBreakPoint::CBreakPoint()
//{
//}
//
//
//CBreakPoint::~CBreakPoint()
//{
//}
//
//BOOL CBreakPoint::InitBP(HANDLE hProcess, LPVOID lpvAddr, BP_TYPE enBpType){
//	SIZE_T ReadBytes = 0;
//	BOOL bRet = ReadProcessMemory(hProcess, lpvAddr, &m_OldDate, 1, &ReadBytes);
//	if (!bRet&&ReadBytes==1){
//		OutputDebugString(L"InitBP 中设置断点出现异常\r\n");
//#ifdef DEBUG
//		assert(0);
//#endif // DEBUG
//		return FALSE;
//	}
//	m_BpType = enBpType;
//
//	bRet = FALSE;
//	ReadBytes = 0;
//	BYTE Buff = 0xcc;
//	bRet = WriteProcessMemory(hProcess, lpvAddr, &Buff, 1, &ReadBytes);
//
//	if (!bRet&&ReadBytes == 1){
//		OutputDebugString(L"InitBP 中设置断点出现异常\r\n");
//#ifdef DEBUG
//		assert(0);
//#endif // DEBUG
//		return FALSE;
//	}
//
//	return TRUE;
//}
