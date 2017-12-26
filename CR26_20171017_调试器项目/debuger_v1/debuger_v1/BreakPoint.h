#pragma once

typedef struct _BREAK_POINT /**����ϵ����Ϣ**/
{
	BYTE   Type;       // �ϵ�����[01:����ϵ�/02:Ӳ���ϵ�/03:�ڴ�ϵ�]
	LPVOID lpAddress;  // �ϵ����ڵ�ַ
	BOOL   bIsOnlyOne; // �Ƿ�Ϊһ���Զϵ�
	BOOL   bActinve;   // �Ƿ񼤻�
	BOOL   PointLen;   // �ϵ㳤��
	BOOL   bRecover;
	DWORD  OldAttrbuit;
	union TYPE_BACKUP
	{
		DWORD BreakTyep;	// �ж����ͣ���Ӳ�����ڴ�ϵ㣩	
		BYTE  Backup;		// �������öϵ�ǰ�����ݣ�������ϵ㣩	
		DWORD dwOldProtect; // �ϵ㴦��ԭ�б�������
	}TB;

}BREAK_POINT, *PBREAK_POINT, TEMP_BREAK_POINT, *PTEMP_BREAK_POINT;


typedef struct tagM_BREAK_POINT
{
	LPVOID dwAddr;
	struct PAGE
	{
		DWORD dwBaseAddr;
		DWORD dwSize;
		DWORD dwProtect;
	}page;
	WORD  wSize;
	WORD  wState;
	BOOL   bIsOnlyOne; // �Ƿ�Ϊһ���Զϵ�
	BOOL   bActive;   // �Ƿ񼤻�
	BOOL   bRecover;
}M_BREAK_POINT;

#define BM_WRITE    0x0001
#define BM_ENABLE   0x0002
#define BM_REPAIR   0x0004

enum BREAK_TYPE /** �ж����� **/
{
	BREAK_TYPE_EXE,  // ִ�жϵ�
	BREAK_TYPE_WIT,  // д��ϵ�
	BREAK_TYPE_I_O,  // I/O��д�ж�
	BREAK_TYPE_RED   // ��ȡ�ϵ�
};

enum BP_TYPE    /** �ϵ����� **/
{
	BP_TYPE_INT3 = 1,  // ����ϵ�
	BP_TYPE_HARD = 2,  // Ӳ���ϵ�
	BP_TYPE_MEM = 3,  // �ڴ�ϵ�
	BP_TYPE_STEP = 4   // �����ϵ�
};

/************************************************************************/
/* DR7�Ĵ�����λ�ṹ                                                    */
/************************************************************************/
typedef struct _DBG_REG7
{
	/*
	// �ֲ��ϵ�(L0~3)��ȫ�ֶϵ�(G0~3)�ı��λ
	*/
	unsigned L0 : 1;  // ��Dr0����ĵ�ַ���� �ֲ��ϵ�
	unsigned G0 : 1;  // ��Dr0����ĵ�ַ���� ȫ�ֶϵ�
	unsigned L1 : 1;  // ��Dr1����ĵ�ַ���� �ֲ��ϵ�
	unsigned G1 : 1;  // ��Dr1����ĵ�ַ���� ȫ�ֶϵ�
	unsigned L2 : 1;  // ��Dr2����ĵ�ַ���� �ֲ��ϵ�
	unsigned G2 : 1;  // ��Dr2����ĵ�ַ���� ȫ�ֶϵ�
	unsigned L3 : 1;  // ��Dr3����ĵ�ַ���� �ֲ��ϵ�
	unsigned G3 : 1;  // ��Dr3����ĵ�ַ���� ȫ�ֶϵ�
	/*
	// �������á����ڽ���CPUƵ�ʣ��Է���׼ȷ���ϵ��쳣
	*/
	unsigned LE : 1;
	unsigned GE : 1;
	/*
	// �����ֶ�
	*/
	unsigned Reserve1 : 3;
	/*
	// �������ԼĴ�����־λ�������λΪ1������ָ���޸����ǼĴ���ʱ�ᴥ���쳣
	*/
	unsigned GD : 1;
	/*
	// �����ֶ�
	*/
	unsigned Reserve2 : 2;
	/*
	// ����Dr0~Dr3��ַ��ָ��λ�õĶϵ�����(RW0~3)��ϵ㳤��(LEN0~3)��״̬�������£�
	// �ϵ�����(RW0~3)��
	//     00 : ִ�жϵ㣬ִ�е�ָ����ַʱ�ж�
	//     01 : д��ϵ㣬��ָ����ַд������ʱ�ж�
	//     10 : I/O��д�жϣ�ĿǰCPU��֧��
	//     11 : ��ȡ�ϵ㣬��ָ����ַ��ȡ����ʱ�ж�
	// �ϵ㳤��(LEN0~3)��
	//     00 : 1�ֽ�
	//     01 : 2�ֽ�
	//     10 : 8�ֽ�
	//     11 : 4�ֽ�
	*/
	unsigned RW0 : 2;  // �趨Dr0ָ���ַ�Ķϵ�����
	unsigned LEN0 : 2;  // �趨Dr0ָ���ַ�Ķϵ㳤��
	unsigned RW1 : 2;  // �趨Dr1ָ���ַ�Ķϵ�����
	unsigned LEN1 : 2;  // �趨Dr1ָ���ַ�Ķϵ㳤��
	unsigned RW2 : 2;  // �趨Dr2ָ���ַ�Ķϵ�����
	unsigned LEN2 : 2;  // �趨Dr2ָ���ַ�Ķϵ㳤��
	unsigned RW3 : 2;  // �趨Dr3ָ���ַ�Ķϵ�����
	unsigned LEN3 : 2;  // �趨Dr3ָ���ַ�Ķϵ㳤��
}DBG_REG7, *PDBG_REG7;


/************************************************************************/
/* DR6�Ĵ�����λ�ṹ														*/
/************************************************************************/
typedef struct _DBG_REG6
{
	/*
	//     �ϵ����б�־λ�����λ��DR0~3��ĳ���ϵ㱻���У�������쳣����ǰ����Ӧ
	// ��B0~3�ͻᱻ��Ϊ1��
	*/
	unsigned B0 : 1;  // Dr0�ϵ㴥����λ
	unsigned B1 : 1;  // Dr1�ϵ㴥����λ
	unsigned B2 : 1;  // Dr2�ϵ㴥����λ
	unsigned B3 : 1;  // Dr3�ϵ㴥����λ
	/*
	// �����ֶ�
	*/
	unsigned Reserve1 : 9;
	/*
	// ����״̬�ֶ�
	*/
	unsigned BD : 1;  // ���ƼĴ����������ϵ�󣬴�λ����Ϊ1
	unsigned BS : 1;  // �����쳣����������Ҫ���־�Ĵ���(EFLAGS)��TF����ʹ��
	unsigned BT : 1;  // �˱����TSS��T��־����ʹ�ã����ڽ���CPU�����л��쳣
	/*
	// �����ֶ�
	*/
	unsigned Reserve2 : 16;
}DBG_REG6, *PDBG_REG6;


/************************************************************************/
/* EFLAGS�Ĵ�����λ�ṹ                                                 */
/************************************************************************/
//��־�Ĵ���
typedef struct _tagEFlags
{
	DWORD dwCF : 1;  //32    0
	DWORD UnUse3 : 1;  //31    1
	DWORD dwPF : 1;  //30    2
	DWORD UnUse2 : 1;  //29    3
	DWORD dwAF : 1;  //28    4
	DWORD UnUse1 : 1;  //27    5
	DWORD dwZF : 1;  //26    6
	DWORD dwSF : 1;  //25    7
	DWORD dwTF : 1;  //24    8
	DWORD dwIF : 1;  //23    9
	DWORD dwDF : 1;  //22    10
	DWORD dwOF : 1;  //21    11
	DWORD UnUse : 20; //20    12

}EFLAGS, *PEFLAGS;