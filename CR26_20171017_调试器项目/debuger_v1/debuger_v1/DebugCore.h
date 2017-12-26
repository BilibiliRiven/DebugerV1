#pragma once
#include "capstone.h"
#include "BpManager.h"
#include <tlhelp32.h>
#include <string>
#include <vector>
using namespace std;
/*
*  CDebugCore �ǵ��������Ĺ����ࡣ
*  �ṩ�������ĺ���API���ã�Ҳ����
*  MCV�е�M��
*/
extern BYTE _int3;

typedef struct tag_MODULE_INFO{
	wstring ModName;
	HANDLE hMod;
	DWORD dwModSize;
}MODULE_INFO, *PMODULE_INFO;

class CDebugCore
{
	friend CBpManager;
	enum DBGP_STATUS
	{
		DBGP_SUSPEND,
		DBGP_RUN,
		DBGP_EXIT,
		DBGP_NONE
	} m_DbgProcStatus;
	DBGP_STATUS DbgProcessRun(){ return m_DbgProcStatus = DBGP_RUN; }
	DBGP_STATUS DbgProcessSuspend(){ return m_DbgProcStatus = DBGP_SUSPEND; }
	csh		m_handle;														
	cs_insn * m_pinsn;														// capston����������ṩ�ķ������	
	UINT	m_count;														// ����������㷢�ӱ���������

public:
	PROCESS_INFORMATION	m_stProcessInfo;									// ��ű����Խ��̵������Ϣ
	CString m_cstrFileName;													// ��ű����Խ��̵��ļ���
	DEBUG_EVENT m_stDbgEvent;												// ��ű����Խ��̽��̣������ĵ����¼�
	HANDLE	m_WaitCommand;													// һ���¼��ź����������������ȴ�����������
	BOOL	m_bSystemException;												// �ж��Ƿ���ϵͳ�ϵ�
	BOOL	m_bStepHandle;													// �ж��Ƿ������ֹ���ɵĵ���
	BOOL	m_bStepInto;													// �ж��Ƿ��е���
	TEMP_BREAK_POINT m_TempBreakPoint;										// ��ʱ�Ķϵ�
	CList<BREAK_POINT, BREAK_POINT&> m_lstBp;								// �ϵ��
	CList<M_BREAK_POINT, M_BREAK_POINT &> m_lstMBp;							// �ڴ�ϵ��
	CArray<MODULEENTRY32, MODULEENTRY32> m_aryMod;							// ģ��Ϣ��
	CArray<MEMORY_BASIC_INFORMATION, MEMORY_BASIC_INFORMATION> m_aryMem;    // �ڴ������Ϣ��
	BREAK_POINT m_lstHbp[4];												// Ӳ���ϵ��
	BOOL  m_IsRecoverHBP;													// �Ƿ���Ҫ�ָ�Ӳ���ϵ�
	int   m_nrDReg;															// ������Ҫ�ָ��ļĴ������
	BOOL  m_IsRecoverBp;													// �Ƿ���Ҫ�ָ��ϵ�
	BOOL  m_IsRecoverMBp;													// �Ƿ���Ҫ�ָ��ڴ�ϵ�
	BOOL  m_IsGo;															// �ָ��ϵ���Ƿ�Ӧ��ֱ��������
	vector<MODULE_INFO> m_ModList;
public:
	CONTEXT m_stRegContext;													// �����żĴ�������Ϣ
																			// ��DbgUpdateRegContext���Ը��¼Ĵ���
																			// ��Ϣ
	/*
	*  ���ܣ������������Ҫ��ɳ�ʼ������
	*/
	CDebugCore();
	/*
	*  ���ܣ������������Ҫ�����Դ���ͷŹ���
	*/
	~CDebugCore();
	/*
	*	���ܣ��Ե��Է�ʽ����PE�ļ��������߳̾������ֵΪ0ʱ������ʧ��
	*   Parameter1���ļ�·��(�����ļ���)
	*	Return Value���ļ�ִ�гɹ�����TRUE��ʧ�ܷ���FALSE
	*/
	BOOL DbgPeFile(CString & rstrFilePath);
	/*
	*	���ܣ��ֹ�����һ�������i��������
	*	Return Value: ����ִ�гɹ�����TRUE, ����ִ��ʧ�ܷ���FALSE
	*/
	BOOL DbgSetpOver();
	/*
	*  ���ܣ��ֹ�����һ�������i�������
	*  Return Value: ����ִ�гɹ�����TRUE, ����ִ��ʧ�ܷ���FALSE
	*/
	BOOL DbgSetpInto();
	/*
	*  ���ܣ����ֹ�����һ����������ϵ㣬CPUִ��һ��ָ�ͨ�����ڶϵ�ָ�
	*  Return Value: ����ִ�гɹ�����TRUE, ����ִ��ʧ�ܷ���FALSE
	*/
	BOOL DbgSetStep();
	/*
	*  ���ܣ������������û��ʵ��
	*/
	int DbgBPList();

private:
	/*
	*  ���ܣ��̻߳ص�������ѭ�����յ����¼�����������¼�
	*  ��������ǰ������ָ��
	*/
	static DWORD ThreadDebugEventProc(LPVOID pThis);
	/*
	*  ����һ�麯���ǻ�����¼��Ĵ�����
	*/
	int OnExceptionDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnCreateThreadDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnCreateProcessDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnExitProcessDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnExitThreadDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnLoadDllDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnUnloadDllDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnOutputDebugStringEvent(DEBUG_EVENT * pDebugEvent);
	int OnRipEvent(DEBUG_EVENT * pDebugEvent);
public:
	/*
	*  ����3���������Ƕ��쳣�����¼��ķ��ദ����
	*/
	int OnExceptionBreakPoint(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo);
	int OnExceptionAccessViolation(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo);
	int OnExceptionSingleStep(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo);
	/*
	*   ���ܣ��ṩ�����������Ϣ��
	*  ����1������Ҫ��������ݵ��׵�ַ
	*  ����2���Ƿ������ٸ��ֽ�
	*  ����3�������������ɹ�������ָ������
	*  ����4������CapStone�ṩ�ṹ�壬������ȡ��������Ϣ
	*  ����ֵ������ִ��ʧ�ܷ���ֵ��0���ɹ�����ֵ����
	*/
	int DbgGetAsm(LPVOID lpvAddr, UINT uiBuffSize = 256, UINT  * piCount = 0, const cs_insn ** pinsn = 0);
	/*
	*	���ܣ���鱻���Գ����Ƿ����
	*	����ֵ��TURE�����Գ�����𣬿��Խ��в�����FLASE���ܽ��в���
	*/
	BOOL DbgIsSuspend(){ return m_DbgProcStatus == DBGP_SUSPEND ? TRUE : FALSE; }
	/*
	*	���ܣ�����ʧ�ܵ���ʾ��Ϣ����Ҫ���ڵ���
	*/
	void DbgWarningProcRun(){ printf("����ʧ��,���Գ�����������\r\n"); }
private:
	/*
	*	���ܣ����¼Ĵ�����Ϣ
	*	����ΪTRUEʱ�����Ĵ�����Ϣ���µ�m_stRegContext�ṹ����
	*	����ΪFLASEʱ�����ṹ������ݣ���ϸ��CPU�Ĵ�����
	*/
	BOOL DbgUpdateRegContext(BOOL bUpdate);
public:
	/*
	*	���ܣ���д�ڴ��е�����
	*	Parameter1��lpvAddr �޸�λ�õ��׵�ַ
	*	Parameter2��pBuff �����ݵĻ������׵�ַ
	*	Parameter3��uiBuffSize �����ݻ�������С
	*	Return Value����������ִ�гɹ�����TRUE��ʧ�ܷ���FALSE
	*/
	BOOL DbgWriteMemory(LPVOID lpvAddr, BYTE * pBuff, UINT uiBuffSize, DWORD * pReadBytes = NULL);
	/*
	*	���ܣ����ڴ��е�����
	*	Parameter1��lpvAddr ��ȡλ�õ��׵�ַ
	*	Parameter2��pBuff �������׵�ַ
	*	Parameter3��uiBuffSize ��������С
	*	Return Value����������ִ�гɹ�����TRUE��ʧ�ܷ���FALSE
	*/
	BOOL DbgReadMemory(LPVOID lpvAddr, BYTE * pBuff, UINT uiBuffSize, DWORD * pReadBytes = NULL);
	/*
	*	���ܣ���ָ�����ڴ�λ��д��CC
	*	����1��ָ��λ�õ��ڴ��ַ
	*	����ֵ���ɹ����ط���ֵ��ʧ�ܷ���0ֵ
	*/
	BOOL SetInt3(LPVOID lpvAddr);
	/*
	*	���ܣ���ָ�����ڴ�λ�����öϵ�
	*	����1��ָ��λ�õ��ڴ��ַ
	*	����2�����봫�����������ضϵ����Ϣ
	*	����ֵ���ɹ����ط���ֵ��ʧ�ܷ���0ֵ
	*/
	BOOL SetInt3(LPVOID lpvAddr, BREAK_POINT & bpinfo);
	/*
	*	���ܣ��ж��Ƿ�int3�ǵ������ϵ�
	*	Parameter1��lpvAddr �ϵ��ַ
	*	Parameter2��bRet ������TURE����m_lstBpɾ���ϵ㲢�һָ����ǣ�����Ϊ��ֻ�ָ����ǣ���ɾ���ϵ�
	*	Return Value���Ƿ��ضϵ�λ�ã����Ǿͷ���NULL
	*/
	POSITION DbgIsDbgInt3(LPVOID lpvAddr);
	/*
	*	���ܣ�ɾ��Int3�ϵ�
	*	����1���ϵ��ڶϵ��б��е�λ��
	*	����2��TRUE����ɾ��������Ӷϵ��б�����ȫɾ����FALSE��ɾ�����������ʱɾ������û�дӶϵ��б���ɾ��
	*	����ֵ���ɹ�����TURE��ʧ�ܷ���FALSE
	*/
	BOOL DbgDelInt3BP(POSITION pos, BOOL bDel);
	/*
	*	���ܣ�����һ��Int3�ϵ㣬����ͬʱ���öϵ�����
	*	����1��int3�ϵ��λ��
	*	����2���Ƿ�Ϊ��ʱ�ϵ�
	*/
	BOOL DbgSetInt3BreakPoint(LPVOID lpvAddr, BOOL bOnceTime);
	/*
	*	���ܣ�����һ��Int3��ʱ�ϵ㣬����ͬʱ���öϵ�����
	*	����1��int3�ϵ��λ��
	*	����2���Ƿ�Ϊ��ʱ�ϵ�
	*	����ֵ���ɹ�����TRUE��ʧ�ܷ���FALSE
	*/
	BOOL DbgSetTempBreakPoint(LPVOID lpvAddr);
	/*
	*	���ܣ�ɾ��һ��Int3��ʱ�ϵ�
	*	����ֵ���ɹ�����TURE��ʧ�ܷ���FASE
	*/
	BOOL DbgDelTempBreakPoint();
	/*
	*	���ܣ��ֶ����ã��ñ����Գ�����������
	*	����ֵ���ɹ�����TRUE��ʧ�ܷ���FALSE
	*/
	BOOL DbgRun();
	/*
	*	���ܣ�����Ӳ���ϵ�
	*	����1��Ӳ���ϵ��ַ
	*	����2��Ӳ���ϵ�ĳ���
	*	����3��Ӳ���ϵ�ĳ���
	*/
	BOOL DbgSetHBP(LPVOID lpvAddr, int iType, UINT uiLen);
	/*
	*	���ܣ����ʣ��δʹ�õ�Ӳ���ϵ�Ĵ���
	*	return value: ����0 1 2 3 ��Ӧ����ʹ�õ�HBP�Ĵ�����������-1��ʾ�޼Ĵ�������ʹ�á�
	*/
	int DbgGetHbpReg();
private:
	/*
	*	���ܣ�ɨ��Ӳ���ϵ������Ҫ�ָ���Ӳ���ϵ�ָ�
	*	����ֵ���ָ��ɹ�����TURE��ʧ�ܷ���FALSE
	*/
	BOOL DbgRecoverHBP();
public:
	/*
	*	���ܣ�ɨ��INT3�ϵ������Ҫ�ָ���Ӳ���ϵ�ָ�
	*	����ֵ���ָ��ɹ�����TURE��ʧ�ܷ���FALSE
	*/
	BOOL DbgRecoverBP();
	/*
	*	����ֵ���ָ��ɹ�����TURE��ʧ�ܷ���FALSE
	*	���ܣ�ɨ���ڴ�ϵ������Ҫ�ָ����ڴ�ϵ�ָ�
	*/
	BOOL DbgSetMemBP(LPVOID lpvAddr, UINT uLen, BYTE btType);
	/*
	*	���ܣ�����ַ�ĺϷ���
	*	����ֵ���Ϸ�������TURE��ʧ�ܷ���FALSE
	*/
	BOOL DbgIsInvalidAddr(LPVOID lpvAddr);
	/*
	*	���ܣ����ϵ��Ƿ�������
	*	����ֵ���Ϸ�������TURE��ʧ�ܷ���FALSE
	*/
	BOOL DbgIsMemBpCovered(LPVOID lpvAddr, UINT uLen);
	/*
	*	���ܣ������ڴ���Ϣ
	*	����ֵ��
	*/
	BOOL DbgUpdateMem();
	/*
	*   ���ܣ������ڴ�ϵ������ַ�Ƿ������ڴ�ϵ��Ӧ�ķ�ҳ�ڡ�
	*   ����1�������ĵ�ַ
	*	����2��bActiveΪ�棬ֻ��鼤��Ķϵ㡣
	*   ����ֵ�������ַ��û�����ڶϵ��ڣ�����FLASE�����򷵻�TURE��ͬʱͨ��ָ�룬
	*	�����ڴ�ϵ�ԭ���ķ�ҳ��Ϣ
	*/
	int DbgFindBmPage(LPVOID lpvAddr, BOOL bActive, M_BREAK_POINT::PAGE * pPage);
	/*
	*   ���ܣ������ڴ�ϵ������ַ�Ƿ������ڴ�ϵ��Ӧ�ķ�ҳ�ڡ�
	*   ����1�������ĵ�ַ
	*	����2��bActiveΪ�棬ֻ��鼤��Ķϵ㡣
	*   ����ֵ�������ַ��û�����ڶϵ��ڣ�����NUL�����򷵻ضϵ�
	*	�ڶϵ���е�λ�á�ͬʱͨ��ָ�룬�����ڴ�ϵ�Ľṹ����Ϣ
	*/
	POSITION DbgFindBmInPageEX(LPVOID lpvAddr, bool bActive, M_BREAK_POINT::PAGE * pPage = NULL);
	/*
	*   ���ܣ�����ַlpvAddr�Ƿ������ڴ�ϵ�ķ�Χ�ڡ�
	*	����1�������ĵ�ַ
	*   ����2��bActiveΪTRUEҪ��ϵ㱻���bActiveΪFLASEҪ��ϵ�
	*   ����ֵ���ϵ��ڶϵ���е�λ�ã�ʧ�ܷ���NULL
	*/
	POSITION DbgFindMemBp(LPVOID lpvAdd, BOOL bActive);
	/*
	*   ���ܣ��ָ��ڴ�ϵ�
	*   ����ֵ������ִ�гɹ�����TRUE������ִ��ʧ�ܷ���FALSE
	*/
	BOOL DbgRecoverMBp();
	/*
	*   ���ܣ��ָ��ڴ�ϵ�
	*   ����ֵ������ִ�гɹ�����TRUE������ִ��ʧ�ܷ���FALSE
	*/
	void DbgUpdateBmPage();
	/*
	*	���ܣ������ڴ���Ϣ������ƶ���ַ�Ƿ����ڴ��ҳ��
	*   ����1����ѯ�ĵ�ַ
	*	����2�����봫����������ѯ��ַ���ڷ�ҳ�ķ�ҳ��Ϣ
	*/
	BOOL CDebugCore::DbgFindPage(LPVOID lpvAddr, MEMORY_BASIC_INFORMATION * pMbi);
	/*
	*	���ܣ����üĴ�������Ϣ
	*   ����1�����봫�����������������Ĵ�������Ϣ
	*	����ֵ������ִ�гɹ�����TURE��ִ��ʧ�ܷ���FALSE
	*/
	BOOL DbgGetRegInfo(CONTEXT * pContext);
	/*
	*	���ܣ����FS�йؼĴ�������Ϣ
	*   ����ֵ��FS�Ĵ���ֵ
	*/
	DWORD DbgGetFs();
	/*
	*   ���ܣ���öϵ�Ļ�����Ϣ
	*	����1�����봫��������ָ��һ��ָ�����飬������Ŷϵ�ĵ�ַ
	*   ����2�����봫����������������ϵ�
	*   �ɹ�������TURE��ʧ�ܷ���FALSE
	*/
	BOOL DbgGetBpInfo(LPVOID * pAddr, DWORD * pdwCount);
	/*
	*	���ܣ����ݶϵ��ţ�ɾ���ϵ�
	*	����1���ϵ���
	*/
	BOOL DbgDelBp(int k);
	/*
	*	���ܣ����ݶϵ��ţ�ɾ���ڴ�ϵ�
	*	����1���ϵ���
	*/
	BOOL DbgDelBm(int k);
	/*
	*	���ܣ����ݶϵ��ţ�ɾ��Ӳ���ϵ�
	*	����1���ϵ���
	*/
	BOOL DbgDelBh(int k);
	/*
	*	���ܣ�����ģ����Ϣ
	*	����1�����ģ�����
	*/
	BOOL UpdateModule(int k);
};

