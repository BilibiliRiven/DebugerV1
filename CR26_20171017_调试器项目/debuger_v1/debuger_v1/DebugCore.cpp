#include "stdafx.h"
#include "DebugCore.h"


#pragma comment(lib, "capstone.lib")


BYTE _int3 = 0xcc;
CDebugCore::CDebugCore(){
	m_WaitCommand = CreateEvent(NULL, FALSE, FALSE, NULL);

	OutputDebugString(L"����������ʼ��\r\n");
	if (cs_open(CS_ARCH_X86, CS_MODE_32, &m_handle) != CS_ERR_OK){
		OutputDebugString(L"����������ʼ��ʧ��\r\n");
		ExitProcess(-1);
	}

	m_pinsn = NULL;
	m_TempBreakPoint.bIsOnlyOne = FALSE;
	m_IsRecoverHBP = FALSE;
	m_IsRecoverBp = FALSE;
	m_bStepHandle = FALSE;
	m_bStepInto = FALSE;
	m_IsRecoverMBp = FALSE;
	m_IsGo = FALSE;
}


CDebugCore::~CDebugCore(){
	CloseHandle(m_WaitCommand);
	cs_close(&m_handle);
}


BOOL CDebugCore::DbgPeFile(CString & rstrFilePath){
	if (rstrFilePath.GetLength() <= 0){
		OutputDebugString(L"DbgPeFile�� �������ļ�������\r\n");
		return FALSE;
	}
	m_cstrFileName = rstrFilePath;
	HANDLE hThread = CreateThread(FALSE
			, NULL
			, (LPTHREAD_START_ROUTINE)ThreadDebugEventProc
			, this
			, 0
			, NULL);

	return hThread > 0 ? TRUE : (OutputDebugString(_T("DbgPeFile,����ThreadDebugEventProc�߳�ʧ��\r\n")), FALSE);
}


DWORD CDebugCore::ThreadDebugEventProc(LPVOID pThis){

	CDebugCore * pDbgCore = (CDebugCore *)pThis;


	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	BOOL bRet = CreateProcess(pDbgCore->m_cstrFileName
		, NULL
		, NULL
		, FALSE
		, FALSE
		, DEBUG_ONLY_THIS_PROCESS
		, NULL
		, NULL
		, &si
		, &(pDbgCore->m_stProcessInfo));


	DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	for (;;
	ContinueDebugEvent(pDbgCore->m_stDbgEvent.dwProcessId, pDbgCore->m_stDbgEvent.dwThreadId, dwContinueStatus))
	{
		pDbgCore->DbgProcessRun(); // �ڵȴ�������Ϣ֮ǰ�����ϳ��������С�
		WaitForDebugEvent(&(pDbgCore->m_stDbgEvent), INFINITE); // �ȴ������¼���
		if (pDbgCore->m_stDbgEvent.dwProcessId != pDbgCore->m_stProcessInfo.dwProcessId){
			// ���������Ϣ����������Ҫ���߳���Ϣ�����
			continue;
		}
		
		pDbgCore->DbgProcessSuspend(); // �е�����Ϣ������ʱ����Խ��̾ͻᱻ����
		switch (pDbgCore->m_stDbgEvent.dwDebugEventCode)
		{
			case EXCEPTION_DEBUG_EVENT:      /***** �쳣�����¼� ***********/
				dwContinueStatus = pDbgCore->OnExceptionDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case CREATE_THREAD_DEBUG_EVENT:  /***** �̵߳����¼� ***********/
				dwContinueStatus = pDbgCore->OnCreateThreadDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case CREATE_PROCESS_DEBUG_EVENT: /***** ���̵����¼� ***********/
				dwContinueStatus = pDbgCore->OnCreateProcessDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case EXIT_THREAD_DEBUG_EVENT:    /***** �˳��߳��¼� ***********/
				dwContinueStatus = pDbgCore->OnExitThreadDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case EXIT_PROCESS_DEBUG_EVENT:   /***** �˳������¼� ***********/
				dwContinueStatus = pDbgCore->OnExitProcessDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case LOAD_DLL_DEBUG_EVENT:       /***** ӳ��DLL�¼�  ***********/
				dwContinueStatus = pDbgCore->OnLoadDllDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case UNLOAD_DLL_DEBUG_EVENT:     /***** ��ӳ��DLL�¼� **********/
				dwContinueStatus = pDbgCore->OnUnloadDllDebugEvent(&pDbgCore->m_stDbgEvent);
				break;
			case OUTPUT_DEBUG_STRING_EVENT:  /***** �����ַ�������¼� *****/
				dwContinueStatus = pDbgCore->OnOutputDebugStringEvent(&pDbgCore->m_stDbgEvent);
				break;
			case RIP_EVENT:                  /***** RIP�¼�(�ڲ�����) ******/
				dwContinueStatus = pDbgCore->OnRipEvent(&pDbgCore->m_stDbgEvent);
				break;
			default:
				OutputDebugString(L"ThreadDebugEventProc,���յ��쳣�ĵ����¼�\r\n");
				break;
		}
		TRACE(L"%d-%d\r\n", pDbgCore->m_stDbgEvent.dwProcessId, pDbgCore->m_stDbgEvent.dwThreadId);
	}
	return 0;
}


int CDebugCore::OnExceptionDebugEvent(DEBUG_EVENT * pDebugEvent){

	LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo = &(pDebugEvent->u.Exception);
#ifdef DEBUG
	TRACE(L"���յ��쳣���쳣λ��%p\r\n", pExceptionDebugInfo->ExceptionRecord.ExceptionAddress);
#endif // DEBUG
	
	// �����쳣���봦���쳣
	switch (pExceptionDebugInfo->ExceptionRecord.ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:      // �Ƿ������쳣(�ڴ�ϵ�)
		return OnExceptionAccessViolation(pExceptionDebugInfo);
		break;
	case EXCEPTION_BREAKPOINT:            // �ϵ��쳣
		return OnExceptionBreakPoint(pExceptionDebugInfo);
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT: // �ڴ�����쳣
		MessageBox(NULL, L"�ڴ�����쳣", NULL, MB_OK);
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:   // ��Чָ��
		MessageBox(NULL, L"��Чָ��", NULL, MB_OK);
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:    // ��0����
		MessageBox(NULL, L"��0����", NULL, MB_OK);
		break;
	case EXCEPTION_PRIV_INSTRUCTION:      // ָ�֧�ֵ�ǰģʽ
		MessageBox(NULL ,L"ָ�֧�ֵ�ǰģʽ", NULL, MB_OK);
		break;
	case EXCEPTION_SINGLE_STEP:           // ������Ӳ���ϵ��쳣
		return OnExceptionSingleStep(pExceptionDebugInfo);
		break;
	default:
		break;
	}
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnCreateThreadDebugEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnCreateThreadDebugEvent\r\n");
	return DBG_EXCEPTION_HANDLED;
}


int CDebugCore::OnCreateProcessDebugEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnCreateProcessDebugEvent\r\n");
	
	DbgSetTempBreakPoint(pDebugEvent->u.CreateProcessInfo.lpStartAddress);
	return DBG_EXCEPTION_HANDLED;
}


int CDebugCore::OnExitProcessDebugEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnExitProcessDebugEvent\r\n");
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnExitThreadDebugEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnExitThreadDebugEvent\r\n");
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnLoadDllDebugEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnLoadDllDebugEvent\r\n");
	//GetFilePathByHandle(lpDebugInfo->hFile, lpDebugInfo->lpBaseOfDll, szOutPut);
	//pOutputDlg->m_OutPutListBox.AddString(szOutPut);
	//pOutputDlg->m_OutPutListBox.SetTopIndex(pOutputDlg->m_OutPutListBox.GetCount() - 1);
	//if (m_bStepInto){
	//	m_bStepInto = FALSE;
	//	WaitForSingleObject(m_WaitCommand, INFINITE);
	//}
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnUnloadDllDebugEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnUnloadDllDebugEvent\r\n");
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnOutputDebugStringEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnOutputDebugStringEvent\r\n");
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnRipEvent(DEBUG_EVENT * pDebugEvent){
	OutputDebugString(L"OnRipEvent\r\n");
	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnExceptionBreakPoint(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo){
	OutputDebugString(L"�յ��ϵ��쳣\r\n");
	// �ж�����֪�ϵ�
	do{

		// 0.�ж��Ƿ�����ʱ�ϵ�
		if (m_TempBreakPoint.bIsOnlyOne&&m_TempBreakPoint.lpAddress == pExceptionDebugInfo->ExceptionRecord.ExceptionAddress){
			// �ָ�ԭ����ָ��
			DbgDelTempBreakPoint();
			break;
		}

		// 1.�ж��Ƿ��ǵ������ϵ�
		if (m_lstBp.IsEmpty()){
			return DBG_EXCEPTION_NOT_HANDLED;
		}

		// 2.���Ҷϵ��б��ж��Ƿ��ڶϵ��б���
		POSITION pos = NULL;
		pos = DbgIsDbgInt3(pExceptionDebugInfo->ExceptionRecord.ExceptionAddress);
		if (pos == NULL){
			return DBG_EXCEPTION_NOT_HANDLED;
		}

		// 3.���ִ�е�����˵���ǵ������ϵ㣬Ӧ������Ӧ����
			
			// �ж��Ƿ�Ϊһ���Զϵ�
			// �ǣ����ϵ��m_lstBp��ɾ�������һָ���CC���ǵ�λ�á�
			// �񣺲��һָ���CC���ǵ�λ�á���Ҫ���õ���
			// �ڵ��������лָ��ϵ�
		if ((m_lstBp.GetAt(pos)).bIsOnlyOne){
			// ɾ��һ���Զϵ㣬���һָ�CC����
			DbgDelInt3BP(pos, TRUE);
			break;
		}
		else{
			// ����ʱ�ָ����ڲ����ûָ�״̬
			DbgDelInt3BP(pos, FALSE);
			DbgSetStep();
			break;
		}

		// ����һ���Զϵ�
	} while (0);

	/*
	* ���ڴ���INT3��ʹEIP����ȷ��ַ��1��EIP��0xCC���棩��������Ҫ��EIP��1
	* ������ʲô���͵Ķϵ㶼��Ҫ��һ��
	*/
	DbgUpdateRegContext(TRUE);
	m_stRegContext.Eip--;
	DbgUpdateRegContext(FALSE);
	DbgGetAsm(pExceptionDebugInfo->ExceptionRecord.ExceptionAddress, 32);
	WaitForSingleObject(m_WaitCommand, INFINITE);
	return DBG_EXCEPTION_HANDLED;
}


int CDebugCore::OnExceptionAccessViolation(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo)
{
	OutputDebugString(L"OnExceptionAccessViolation\r\n");

	// �����ʱ�ϵ�
	DbgDelTempBreakPoint();

	DbgUpdateBmPage();

	//  ��������Ҫ���жϣ������쳣�Ƿ�����Ϊ�����ڴ�ϵ�������
	POSITION pos = DbgFindMemBp((LPVOID)pExceptionDebugInfo->ExceptionRecord.ExceptionInformation[1], TRUE);
	if (pos){
		M_BREAK_POINT & mbp = m_lstMBp.GetAt(pos);
		// �Ƚ��Ƿ���ͬ����Ȩ�޵��쳣
		if (pExceptionDebugInfo->ExceptionRecord.ExceptionInformation[0] == (mbp.wState & BM_WRITE) ? 1 : 0){
			// ���ڴ�ϵ㴥�����쳣����ʱӦ�ûָ��ϵ㡣
			mbp.bRecover = TRUE;
			m_IsRecoverMBp = TRUE;

			// ��ԭ��ҳ����
			DWORD flOldProtect = 0;
			if (!VirtualProtectEx(m_stProcessInfo.hProcess, (LPVOID)mbp.page.dwBaseAddr, mbp.page.dwSize, mbp.page.dwProtect, &flOldProtect))
			{
				OutputDebugString(L"��ԭ�ڴ������������󣬲����ҵĹ�\r\n");
				return FALSE;
			}

			DbgSetStep();
			DbgGetAsm(pExceptionDebugInfo->ExceptionRecord.ExceptionAddress, 32);
			WaitForSingleObject(m_WaitCommand, INFINITE);
			return DBG_EXCEPTION_HANDLED;
		}
	}

	// �����жϴ����Ƿ������ڣ����ڴ�ϵ㴦��һ����ҳ��������쳣,Ӧ�÷Ź�ȥ
	pos = DbgFindBmInPageEX((LPVOID)pExceptionDebugInfo->ExceptionRecord.ExceptionInformation[1], TRUE);
	if (pos){

		M_BREAK_POINT & mbp = m_lstMBp.GetAt(pos);
		// ��ԭ��ҳ����
		DWORD flOldProtect = 0;
		if (!VirtualProtectEx(m_stProcessInfo.hProcess, (LPVOID)mbp.page.dwBaseAddr, mbp.page.dwSize, mbp.page.dwProtect, &flOldProtect))
		{

			OutputDebugString(L"��ԭ�ڴ������������󣬲����ҵĹ�\r\n");
			return FALSE;
		}
		mbp.bRecover = TRUE;
		m_IsRecoverMBp = TRUE;
		m_IsGo = TRUE;
		DbgSetStep();
		return DBG_EXCEPTION_HANDLED;

	}

	return DBG_EXCEPTION_NOT_HANDLED;
}


int CDebugCore::OnExceptionSingleStep(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo)
{
	OutputDebugString(L"OnExceptionSingleStep\r\n");
	/*
	* 1. ��ȡ�Ĵ�����Ϣ
	*/
	if (!DbgUpdateRegContext(TRUE)) 	// ���¼Ĵ����ṹ�������
	{
		OutputDebugString(L"OnExceptionSingleStep�����쳣���쳣����if (DbgUpdateRegContext(TRUE)) \r\n");
		return DBG_EXCEPTION_NOT_HANDLED;
	}

	/*
	* 2. �жϾ�����쳣����ԭ��
	*	 �������ɹ���break����ѭ��
	*    ���������ֱ�ӷ���
	*/

	// 2. �жϾ�����쳣����ԭ��
	PDBG_REG6 pReg6 = (PDBG_REG6)&m_stRegContext.Dr6;
	PDBG_REG7 pReg7 = (PDBG_REG7)&m_stRegContext.Dr7;
	PEFLAGS   pEFlags = (PEFLAGS)&m_stRegContext.EFlags;

	do{
		// 3.   �����Ӳ�������Ķϵ㣬�������Ӧ����
		if ((pReg6->B0 && pReg7->L0) || (pReg6->B1 && pReg7->L1)
			|| (pReg6->B2 && pReg7->L2) || (pReg6->B3 && pReg7->L3)){
			if ((DWORD)pExceptionDebugInfo->ExceptionRecord.ExceptionAddress == m_stRegContext.Dr0)
			{
				// �ж��Ƿ��� ִ����Ӳ���ϵ�
				if (pReg7->RW0 == BREAK_TYPE_EXE)
				{
					// �ж��ж����������Ӳ��ִ���ж�
					// ����Ҫ��ȡ��Ӳ���ж�ִ��������Ȼ�����õ���				
					pReg7->L0 = false;
					// ����ָ��ϵ�
					m_IsRecoverHBP = TRUE;	//�Ƿ���Ҫ�ָ�Ӳ���ϵ�
					m_nrDReg = 0;			//������Ҫ�ָ��ļĴ������
				}
			}
			else if ((DWORD)pExceptionDebugInfo->ExceptionRecord.ExceptionAddress == m_stRegContext.Dr1)
			{

				if (pReg7->RW1 == BREAK_TYPE_EXE)
				{
					pReg7->L1 = false;
					m_IsRecoverHBP = TRUE;
					m_nrDReg = 1;
				}
			}
			else if ((DWORD)pExceptionDebugInfo->ExceptionRecord.ExceptionAddress == m_stRegContext.Dr2)
			{

				if (pReg7->RW2 == BREAK_TYPE_EXE)
				{
					pReg7->L2 = false;
					m_IsRecoverHBP = TRUE;
					m_nrDReg = 2;
				}
			}
			else if ((DWORD)pExceptionDebugInfo->ExceptionRecord.ExceptionAddress == m_stRegContext.Dr3)
			{

				if (pReg7->RW3 == BREAK_TYPE_EXE)
				{
					pReg7->L3 = false;
					m_IsRecoverHBP = TRUE;
					m_nrDReg = 3;
				}
			}
			else{
				break;
			}
			DbgUpdateRegContext(FALSE);
			DbgSetStep();
			break;
		}

		// �����Ӳ���ϵ���Ҫ�ָ����Ͱ����ָ�
		if (m_IsRecoverHBP){

			DbgRecoverHBP();
			m_IsRecoverHBP = FALSE;
			m_nrDReg = -1;
			if (m_bStepInto){
				m_bStepInto = FALSE;
				break;
			}
			return DBG_CONTINUE;
		}

		// ����жϵ���Ҫ�ָ����Ͱ����ָ�
		if (m_IsRecoverBp){
			DbgRecoverBP();
			m_IsRecoverBp = FALSE;
			if (m_bStepInto){
				m_bStepInto = FALSE;
				break;
			}
			return DBG_CONTINUE;
		}

		if (m_IsRecoverMBp){
			if (DbgRecoverMBp()){
				OutputDebugString(L"�ɹ��ָ��ڴ�ϵ�\r\n");
			}
			m_IsRecoverMBp = FALSE;

			if (m_bStepInto){
				m_bStepInto = FALSE;
				break;
			}
			
			DbgGetAsm(pExceptionDebugInfo->ExceptionRecord.ExceptionAddress, 32);
			return DBG_CONTINUE;
		}
	} while (0);
	if (m_bStepInto){
		m_bStepInto = FALSE;
	}
	DbgGetAsm(pExceptionDebugInfo->ExceptionRecord.ExceptionAddress, 32);
	WaitForSingleObject(m_WaitCommand, INFINITE);
	return DBG_EXCEPTION_HANDLED;
}


int CDebugCore::DbgGetAsm(LPVOID lpvAddr, UINT uiBuffSize, UINT  * piCount, const cs_insn ** ppinsn)
{
	if (m_pinsn){
		cs_free(m_pinsn, m_count);
		m_pinsn = NULL;
		m_count = 0;
	}

	BYTE * pBuffer = new BYTE[uiBuffSize];
	if (!pBuffer){
		OutputDebugString(L"GetAsm���뻺����ʧ��\r\n");
		return 0;
	}
	SIZE_T	iReadBytes = 0;

	ReadProcessMemory(m_stProcessInfo.hProcess, lpvAddr, pBuffer, uiBuffSize, &iReadBytes);
	
	m_count = cs_disasm(m_handle, (const uint8_t *)pBuffer, iReadBytes, (uint64_t)lpvAddr, 0, &m_pinsn);

	system("cls");
	for (size_t j = 0; j < m_count; ++j){
		printf("0lx%"PRIx64":\t%s\t\t%s\r", m_pinsn[j].address, m_pinsn[j].mnemonic, m_pinsn[j].op_str);

		printf("\n");
	}
	
	if (piCount&&ppinsn){
		*piCount = m_count;
		*ppinsn = m_pinsn;
	}

	if (pBuffer){
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return m_count;
}



BOOL CDebugCore::DbgSetpInto(){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	if (!DbgUpdateRegContext(TRUE)) 	// ���¼Ĵ����ṹ�������
	{
		return FALSE;
	}

	OutputDebugString(L"���õ����ϵ㣬�޸ļĴ���EFlages\r\n");
	m_stRegContext.EFlags |= 0x100;
	m_bStepInto = TRUE;
	if (!DbgUpdateRegContext(FALSE))  // ���Ĵ����ṹ������ݣ����µ�ʵ�ʵ�CPU�Ĵ�����
	{
		return FALSE;
	}
	SetEvent(m_WaitCommand);  // ����������ȴ��������Բ���
	return TRUE;
}


BOOL CDebugCore::DbgSetStep(){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	
	if (!DbgUpdateRegContext(TRUE)) 	// ���¼Ĵ����ṹ�������
	{
		return FALSE;
	}

	OutputDebugString(L"���õ����ϵ㣬�޸ļĴ���EFlages\r\n");
	m_stRegContext.EFlags |= 0x100;
	m_bStepHandle = TRUE;
	if (!DbgUpdateRegContext(FALSE))  // ���Ĵ����ṹ������ݣ����µ�ʵ�ʵ�CPU�Ĵ�����
	{
		return FALSE;
	}
	return TRUE;
}


BOOL CDebugCore::DbgUpdateRegContext(BOOL bUpdate)
{
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	if (bUpdate){
		m_stRegContext.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
		BOOL bRet = GetThreadContext(m_stProcessInfo.hThread, &m_stRegContext);
		if (!bRet){
			OutputDebugString(L"DbgSetpInto �޷���üĴ���Context\r\n");
		}
		return bRet;
	}
	else{
		BOOL bRet = SetThreadContext(m_stProcessInfo.hThread, &m_stRegContext);
		if (!bRet){
			OutputDebugString(L"DbgSetpInto �޷����¼Ĵ���Context\r\n");
		}
		return bRet;
	}
	return FALSE;
}


BOOL CDebugCore::DbgWriteMemory(LPVOID lpvAddr, BYTE * pBuff, UINT uiBuffSize, DWORD * pReadBytes){

	_ASSERT(m_stProcessInfo.hProcess);
	return WriteProcessMemory(m_stProcessInfo.hProcess, lpvAddr, pBuff, uiBuffSize, pReadBytes);
}

BOOL CDebugCore::DbgReadMemory(LPVOID lpvAddr, BYTE * pBuff, UINT uiBuffSize, DWORD * pReadBytes){
	_ASSERT(m_stProcessInfo.hProcess);
	BOOL bRet = ReadProcessMemory(m_stProcessInfo.hProcess, lpvAddr, pBuff, uiBuffSize, pReadBytes);
	if (!bRet){
		return FALSE;
	}
	POSITION pos = m_lstBp.GetHeadPosition();
	while (pos)
	{
		BREAK_POINT & bp = m_lstBp.GetNext(pos);
		if (bp.lpAddress >= lpvAddr && (UINT)bp.lpAddress < (UINT)lpvAddr + *pReadBytes)
		{
			LPBYTE pCode = (LPBYTE)pBuff + ((LPBYTE)bp.lpAddress - lpvAddr);
			*pCode = bp.TB.Backup;
		}
	}
	return TRUE;
}


BOOL CDebugCore::SetInt3(LPVOID lpvAddr, BREAK_POINT & bpinfo)
{
	// ��д�ϵ��ַ��Ϣ
	bpinfo.lpAddress = lpvAddr;

	// ��¼ԭ��λ�õ���Ϣ
	if (!DbgReadMemory(lpvAddr, &bpinfo.TB.Backup, sizeof(bpinfo.TB.Backup)))
	{
		return FALSE;
	}

	// д��int3
	return DbgWriteMemory(lpvAddr, &_int3, sizeof(_int3));
}

BOOL CDebugCore::SetInt3(LPVOID lpvAddr)
{

	return DbgWriteMemory(lpvAddr, &_int3, sizeof(_int3));
}



POSITION CDebugCore::DbgIsDbgInt3(LPVOID lpvAddr)
{
	POSITION pos = NULL;
	POSITION pre = NULL;

	pos = m_lstBp.GetHeadPosition();
	BREAK_POINT bp = { 0 };
	
	pre = pos;
	while (pos != NULL){
		bp = m_lstBp.GetNext(pos);
		if (bp.lpAddress == lpvAddr){
			return pre;
		}
		pre = pos;
	}
	return pre;
}


BOOL CDebugCore::DbgDelInt3BP(POSITION pos, BOOL bDel)
{
	BREAK_POINT bp = { 0 };
	bp = m_lstBp.GetAt(pos);
	BOOL bRet = DbgWriteMemory(bp.lpAddress, &(bp.TB.Backup), sizeof(bp.TB.Backup));
	if (!bRet){
		assert(bRet);
	}
	
	m_IsRecoverBp = TRUE;
	m_lstBp.GetAt(pos).bRecover = TRUE;  // ����ϵ�����Ҫ�ָ��ġ�

	// �ָ��ɹ� �� bDelΪ���ִ��ɾ��
	if (bRet&&bDel){
		m_lstBp.RemoveAt(pos);
	}
	return bRet;
}


BOOL CDebugCore::DbgSetInt3BreakPoint(LPVOID lpvAddr, BOOL bOnceTime){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	BREAK_POINT bp = { 0 };
	SetInt3(lpvAddr, bp);
	bp.bIsOnlyOne = bOnceTime;
	m_lstBp.AddTail(bp);
	return TRUE;
}


BOOL CDebugCore::DbgSetTempBreakPoint(LPVOID lpvAddr)
{
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	if (!SetInt3(lpvAddr, m_TempBreakPoint))  // ����INT3�ϵ�
	{
		MessageBox(NULL, L"�޷�������ʱ�ϵ�", NULL, NULL);
		return FALSE;
	}

	m_TempBreakPoint.bIsOnlyOne = TRUE; // ��д�ϵ㸽������,����ʹ�õ�����ʱ�ϵ����ԣ���仰����˼������ʱ�ϵ㱻���
	return TRUE;
}


BOOL CDebugCore::DbgDelTempBreakPoint()
{
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}

	if (m_TempBreakPoint.lpAddress){
		BOOL bRet = WriteProcessMemory(m_stProcessInfo.hProcess
			, m_TempBreakPoint.lpAddress
			, &m_TempBreakPoint.TB.Backup
			, sizeof(m_TempBreakPoint.TB.Backup)
			, 0);
		m_TempBreakPoint.lpAddress = 0;
		return bRet;
	}
	else{
		return FALSE;
	}
}


BOOL CDebugCore::DbgRun()
{

	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	SetEvent(m_WaitCommand);
	return TRUE;
}


BOOL CDebugCore::DbgSetpOver(){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	DbgUpdateRegContext(TRUE);

	DbgGetAsm((LPVOID)m_stRegContext.Eip, 32);

	if (!strcmp(m_pinsn[0].mnemonic, "call")){
		POSITION pos = DbgIsDbgInt3((LPVOID)(m_stRegContext.Eip + m_pinsn[0].size));
		if (!pos){
			DbgSetInt3BreakPoint((LPVOID)(m_stRegContext.Eip + m_pinsn[0].size), TRUE);
		}
		else{
			// ����ҵ��ϵ��� ����Ҫ���¶ϵ�
		}
		SetEvent(m_WaitCommand);
		return TRUE;
	}

	DbgSetpInto();
	return TRUE;
}

BOOL CDebugCore::DbgSetHBP(LPVOID lpvAddr, int iType, UINT uiLen){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}

	// ����ж�������ִ�жϵ㣬��ϵ㳤��ǿ�Ƹ�Ϊ0
	if (iType == BREAK_TYPE_EXE){
		uiLen = 0;
	}
	DbgUpdateRegContext(TRUE);

	// ���ҿ���ʹ�õ�Ӳ���Ĵ���
	DWORD nFreeReg = DbgGetHbpReg();
	if (nFreeReg < 0){
		printf("û�п���ʹ�õļĴ���\r\n");
		return FALSE;
	}

	PDBG_REG7 pReg7 = (PDBG_REG7)&(m_stRegContext.Dr7);

	switch (nFreeReg)
	{
	case 0:
		m_stRegContext.Dr0 = (DWORD)lpvAddr;
		pReg7->L0 = 1;
		pReg7->LEN0 = uiLen;
		pReg7->RW0 = iType;
		break;
	case 1:
		m_stRegContext.Dr1 = (DWORD)lpvAddr;
		pReg7->L1 = 1;
		pReg7->LEN1 = uiLen;
		pReg7->RW1 = iType;
		break;
	case 2:
		m_stRegContext.Dr2 = (DWORD)lpvAddr;
		pReg7->L2 = 1;
		pReg7->LEN2 = uiLen;
		pReg7->RW2 = iType;
		break;
	case 3:
		m_stRegContext.Dr3 = (DWORD)lpvAddr;
		pReg7->L3 = 1;
		pReg7->LEN3 = uiLen;
		pReg7->RW3 = iType;
		break;
	}
	pReg7->GE = 1;
	pReg7->GD = 1;
	m_lstHbp[nFreeReg].Type = BP_TYPE_HARD;
	m_lstHbp[nFreeReg].lpAddress = lpvAddr;
	m_lstHbp[nFreeReg].PointLen = uiLen;
	m_lstHbp[nFreeReg].TB.BreakTyep = iType;
	DbgUpdateRegContext(FALSE);
	return TRUE;
}


int CDebugCore::DbgGetHbpReg(){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	DbgUpdateRegContext(TRUE);

	if (!m_stRegContext.Dr0){

		return 0;
	}
	if (!m_stRegContext.Dr1){

		return 1;
	}
	if (!m_stRegContext.Dr2){

		return 2;
	}
	if (!m_stRegContext.Dr3){

		return 3;
	}
	
	return -1;
}


BOOL CDebugCore::DbgRecoverHBP(){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	DbgUpdateRegContext(TRUE);
	PDBG_REG7 pReg7 = (PDBG_REG7)&m_stRegContext.Dr7;

	m_IsRecoverHBP = FALSE;
	if (m_nrDReg == -1)
	{
		return FALSE;
	}
	switch (m_nrDReg)
	{
	case 0:
		pReg7->L0 = true;
		break;
	case 1:
		pReg7->L1 = true;
		break;
	case 2:
		pReg7->L2 = true;
		break;
	case 3:
		pReg7->L3 = true;
		break;
	default:
		return FALSE;
	}
	DbgUpdateRegContext(FALSE);
	return TRUE;
}


BOOL CDebugCore::DbgRecoverBP(){
	if (m_lstBp.IsEmpty()){
		return FALSE;
	}
	POSITION pos = m_lstBp.GetHeadPosition();
	POSITION pre = pos;
	while (pos){
		BREAK_POINT & bp = m_lstBp.GetNext(pos);
		if (bp.bRecover){
			SetInt3(bp.lpAddress);
			m_lstBp.GetAt(pre).bRecover = FALSE;
		}
		pre = pos;
	}
	return TRUE;
}


BOOL CDebugCore::DbgSetMemBP(LPVOID lpvAddr, UINT uLen, BYTE btType){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}

	// 0. ��ѯ�Ƿ񳬹���ҳ
	//if ((((UINT)lpvAddr + 0x1000)&(~(0x1000))) - (UINT)lpvAddr < uLen){
	//	return FALSE;
	//}
	DbgUpdateBmPage();

	// 1.�ж϶ϵ��ַ�Ƿ�������
	if (DbgIsMemBpCovered(lpvAddr, uLen)){
		return FALSE;
	}

	// �����Ƿ���Ѿ����õĶϵ���ͬһ����ҳ

	M_BREAK_POINT bp = { 0 };		//�ϵ���Ϣ�ṹ��
	bp.dwAddr = lpvAddr;
	bp.wSize = uLen;
	bp.wState = btType;

	M_BREAK_POINT::PAGE page = { 0 };
	// �Ƿ����֪�ϵ���һ����ҳ��
	if (!DbgFindBmPage(bp.dwAddr, FALSE, &page)){
		// �������һ����ҳ��
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		
		// ��ȡ��ַ��Ӧ�ķ�ҳ��Ϣ
		if (!DbgFindPage(bp.dwAddr, &mbi))
		{
			return FALSE;
		}
		page.dwBaseAddr = (DWORD)mbi.BaseAddress;
		page.dwSize = mbi.RegionSize;
		if (!VirtualProtectEx(m_stProcessInfo.hProcess, (LPVOID)(page.dwBaseAddr), 1, PAGE_NOACCESS, &page.dwProtect))
		{
			return FALSE;
		}
	}
	bp.page = page;
	bp.bActive = TRUE;
	m_lstMBp.AddTail(bp);
	return TRUE;
}


BOOL CDebugCore::DbgIsInvalidAddr(LPVOID lpvAddr){
	// δʵ��
	return 0;
}


BOOL CDebugCore::DbgIsMemBpCovered(LPVOID lpvAddr, UINT uLen){
	if (m_lstMBp.IsEmpty()){
		return FALSE;
	}

	POSITION pos, pre;
	pos = m_lstMBp.GetHeadPosition();
	M_BREAK_POINT mbp;

	BOOL bRet = FALSE;
	while (pos){
		mbp = m_lstMBp.GetNext(pos);
		if (mbp.dwAddr > ((char *)lpvAddr + uLen)
			|| lpvAddr > ((char *)mbp.dwAddr + mbp.wSize)){
			continue;
		}
		else{
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}

/*
*  �Ƴ��ɵķ�ҳ��Ϣ
*  �����ڴ��ҳ��Ϣ��
*/
BOOL CDebugCore::DbgUpdateMem(){
	// �����е��ڴ��ҳ��Ϣ�Ƴ�
	m_aryMem.RemoveAll();

	// ��0x10000����ʼ�����ڴ�
	LPBYTE pAddr = (LPBYTE)0x10000;
	while (pAddr < (LPBYTE)0x80000000){
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		// ��ȡ��ǰ�ڴ����Ϣ
		if (!VirtualQueryEx(m_stProcessInfo.hProcess, pAddr, &mbi, sizeof(mbi))){
			return FALSE;
		}
		if (mbi.State == MEM_COMMIT){
			if (mbi.Protect == PAGE_NOACCESS){
				M_BREAK_POINT::PAGE page = { 0 };
				if (DbgFindBmPage((LPVOID)mbi.BaseAddress, FALSE, &page)){
					mbi.Protect = page.dwProtect;
				}
			}
			m_aryMem.Add(mbi);
		}
		pAddr += mbi.RegionSize;
	}
	return 0;
}


int CDebugCore::DbgFindBmPage(LPVOID lpvAddr, BOOL bActive, M_BREAK_POINT::PAGE * pPage){
	POSITION pos = m_lstMBp.GetHeadPosition();
	while (pos){
		POSITION _pos = pos;
		M_BREAK_POINT mbp = m_lstMBp.GetNext(pos);
		if (bActive?(mbp.bActive):TRUE
			&& (UINT)(lpvAddr) >= mbp.page.dwBaseAddr
			&& (UINT)(lpvAddr) < mbp.page.dwBaseAddr + mbp.page.dwSize){

			if (pPage){
				*pPage = mbp.page;
			}
			return mbp.page.dwBaseAddr;
		}
	}
	return 0;
}


POSITION CDebugCore::DbgFindMemBp(LPVOID lpvAdd, BOOL bActive){
	POSITION pos = m_lstMBp.GetHeadPosition();
	while (pos){
		POSITION _pos = pos;
		M_BREAK_POINT & bm = m_lstMBp.GetNext(pos);
		if ((bActive?(bm.bActive):TRUE)
			&& lpvAdd >= bm.dwAddr && (DWORD)lpvAdd < (DWORD)bm.dwAddr + bm.wSize){
			return _pos;
		}
	}
	return NULL;
}


BOOL CDebugCore::DbgFindPage(LPVOID lpvAddr, MEMORY_BASIC_INFORMATION * pMbi){
	for (int i = 0; i < m_aryMem.GetSize(); ++i){
		MEMORY_BASIC_INFORMATION &mbi = m_aryMem[i];
		if (lpvAddr >= mbi.BaseAddress && (DWORD)lpvAddr < (DWORD)mbi.BaseAddress + mbi.RegionSize){
			if (pMbi)
			{
				*pMbi = mbi;
			}
			return TRUE;
		}
	}
	return FALSE;
}


POSITION CDebugCore::DbgFindBmInPageEX(LPVOID lpvAddr, bool bActive, M_BREAK_POINT::PAGE * pPage){
	POSITION pos = m_lstMBp.GetHeadPosition();
	while (pos){
		POSITION _pos = pos;
		M_BREAK_POINT mbp = m_lstMBp.GetNext(pos);
		if (bActive ? (mbp.bActive) : TRUE
			&& (UINT)(lpvAddr) >= mbp.page.dwBaseAddr
			&& (UINT)(lpvAddr) < mbp.page.dwBaseAddr + mbp.page.dwSize){
			if (pPage){
				*pPage = mbp.page;
			}
			return _pos;
		}
	}
	return NULL;
}


BOOL CDebugCore::DbgRecoverMBp(){
	POSITION pos = m_lstMBp.GetHeadPosition();
	while (pos){
		POSITION _pos = pos;
		M_BREAK_POINT & mbp = m_lstMBp.GetNext(pos);
		if (mbp.bRecover){
			mbp.bRecover = FALSE;
			DWORD flOldProtect = 0;
			if (!VirtualProtectEx(m_stProcessInfo.hProcess, mbp.dwAddr, mbp.wSize, PAGE_NOACCESS, &flOldProtect))
			{
				return FALSE;
			}

			return TRUE;
		}
	}
	return FALSE;
}


void CDebugCore::DbgUpdateBmPage(){
	DbgUpdateMem();
	POSITION pos = m_lstMBp.GetHeadPosition();
	while (pos)
	{
		POSITION _pos = pos;
		M_BREAK_POINT & bm = m_lstMBp.GetNext(pos);
		if (!DbgFindPage(bm.dwAddr, 0))
		{
			m_lstMBp.RemoveAt(_pos);
		}
	}
}


BOOL CDebugCore::DbgGetRegInfo(CONTEXT * pContext){
	BOOL bRet = DbgUpdateRegContext(TRUE);
	if (bRet){
		*pContext = m_stRegContext;
		return TRUE;
	}
	else{
		return FALSE;
	}
	return FALSE;
}


union _LDT{
	DWORD dwVal;
	struct
	{
		WORD BaseLow;
		BYTE BaseMid;
		BYTE BaseHi;
	};
};


DWORD CDebugCore::DbgGetFs(){
	CONTEXT reg = { 0 };
	if (!DbgUpdateRegContext(TRUE))
	{
		return 0;
	}
	LDT_ENTRY ldt = { 0 };
	if (!GetThreadSelectorEntry(m_stProcessInfo.hThread, reg.SegFs, &ldt))
	{
		return 0;
	}
	_LDT _ldt = { 0 };
	_ldt.BaseLow = ldt.BaseLow;
	_ldt.BaseMid = ldt.HighWord.Bytes.BaseMid;
	_ldt.BaseHi = ldt.HighWord.Bytes.BaseHi;

	return _ldt.dwVal;
}


BOOL CDebugCore::DbgGetBpInfo(LPVOID * pAddr, DWORD * pdwCount){
	if (pAddr){
		POSITION pos = m_lstBp.GetHeadPosition();
		int i = 0;
		while (pos)
		{
			BREAK_POINT & bp = m_lstBp.GetNext(pos);
			*(pAddr + i) = bp.lpAddress;
			++i;
		}
	}
	else{
		*pdwCount = m_lstBp.GetCount();
	}
	return false;
}


BOOL CDebugCore::DbgDelBp(int k){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	
	POSITION pos = m_lstBp.GetHeadPosition();
	int i = 0;

	while (pos){
		POSITION _pos = pos;
		BREAK_POINT & bp = m_lstBp.GetNext(pos);
		if (i == k){
			DbgDelInt3BP(_pos, TRUE);
			return TRUE;
		}
		++i;
	}

	return FALSE;
}


BOOL CDebugCore::DbgDelBm(int k){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	POSITION pos = m_lstMBp.GetHeadPosition();
	int i = 0;
	while (pos){
		POSITION _pos = pos;
		M_BREAK_POINT bp = m_lstMBp.GetNext(pos);
		if (i == k){
			m_lstMBp.RemoveAt(_pos);
			if (!DbgFindBmInPageEX(bp.dwAddr, TRUE)){
				DWORD lpflOldProtect = 0;
				VirtualProtectEx(m_stProcessInfo.hProcess, (LPVOID)bp.page.dwBaseAddr, 1, bp.page.dwProtect, &lpflOldProtect);
			}
			return TRUE;
		}
		++i;
	}
	return FALSE;
}


BOOL CDebugCore::DbgDelBh(int k){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}

	LPVOID lpvAddr = 0;
	UINT uiLen = 0;
	int iType = 0;

	DbgUpdateRegContext(TRUE);

	PDBG_REG7 pReg7 = (PDBG_REG7)&(m_stRegContext.Dr7);
	
	switch (k)
	{
	case 0:
		m_stRegContext.Dr0 = (DWORD)lpvAddr;
		pReg7->L0 = 0;
		pReg7->LEN0 = uiLen;
		pReg7->RW0 = iType;
		break;
	case 1:
		m_stRegContext.Dr1 = (DWORD)lpvAddr;
		pReg7->L1 = 0;
		pReg7->LEN1 = uiLen;
		pReg7->RW1 = iType;
		break;
	case 2:
		m_stRegContext.Dr2 = (DWORD)lpvAddr;
		pReg7->L2 = 0;
		pReg7->LEN2 = uiLen;
		pReg7->RW2 = iType;
		break;
	case 3:
		m_stRegContext.Dr3 = (DWORD)lpvAddr;
		pReg7->L3 = 0;
		pReg7->LEN3 = uiLen;
		pReg7->RW3 = iType;
		break;
	}

	pReg7->GE = 0;
	pReg7->GD = 0;
	m_lstHbp[k].Type = BP_TYPE_HARD;
	m_lstHbp[k].lpAddress = lpvAddr;
	m_lstHbp[k].PointLen = uiLen;
	m_lstHbp[k].TB.BreakTyep = iType;
		
	return DbgUpdateRegContext(FALSE);
}


BOOL CDebugCore::UpdateModule(int k){
	if (!DbgIsSuspend()){
		DbgWarningProcRun();
		return FALSE;
	}
	DbgUpdateRegContext(TRUE);
}