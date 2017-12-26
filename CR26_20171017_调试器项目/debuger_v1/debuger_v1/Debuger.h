#pragma once
#include "IMotion.h"
class CDebuger
{
	//CList<IMotion &> m_MotionRecord;
	CString m_FileName;
	
public:
	CDebuger(CString & rstrFileName);
	~CDebuger();
	//BOOL SetInt3(LPVOID lpvAddr, BPINFO& bpinfo);
};

