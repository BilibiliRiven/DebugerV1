#include "stdafx.h"
#include "MySnapshoot.h"

int MyGetSnapshoot(MOD_SNAPSHOOT * pSnapShoot){
	PPEB pPEB = 0;
	__asm{
		mov eax, fs:[0x30]
		mov pPEB, eax
	}
	if (! pPEB){
		assert(0);
	}

	if (!pPEB){
		assert(0);
	}

	PPEB_LDR_DATA pLdrData = pPEB->Ldr;
	PLDR_MODULE pLdrMod = (PLDR_MODULE)(pLdrData->InLoadOrderModuleList.Blink);
	PLDR_MODULE pLdrModEnd = pLdrMod;
	int k = 0;
	do{
		MODULE_INFO temp;
		wstring wstr;
		temp.dwModSize = pLdrMod->SizeOfImage;
		temp.hMod = pLdrMod->BaseAddress;
		wstr = (WCHAR *)(pLdrMod->BaseDllName.pUnicode != 0 ? pLdrMod->BaseDllName.pUnicode: "NULL");
		temp.ModName = wstr;
		(*pSnapShoot).push_back(temp);
		k++;
		pLdrMod = (PLDR_MODULE)(pLdrMod->InLoadOrderModuleList.Blink);
	} while (pLdrModEnd != pLdrMod||pLdrMod <= 0);
}

int MyNextModule(MOD_SNAPSHOOT * pSnapShoot, int k, MODULE_INFO * pMode){
	if (k > 0 && k >= (*pSnapShoot).size() && (*pSnapShoot).size() >0){
		return -1;
	}
	*pMode = ((*pSnapShoot)[k]);
	return ++k;
}