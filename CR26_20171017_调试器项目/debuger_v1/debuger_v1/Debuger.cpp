#include "stdafx.h"
#include "Debuger.h"


CDebuger::CDebuger(CString & rstrFileName)
{
	if (rstrFileName.GetLength()>0){
		OutputDebugString(L"��������ʧ��\r\n");
		ExitProcess(-1);
	}
}


CDebuger::~CDebuger()
{
}


//BOOL CDebuger::SetInt3(LPVOID lpvAddr, BPINFO& bpinfo)
//{
//	return 0;
//}
