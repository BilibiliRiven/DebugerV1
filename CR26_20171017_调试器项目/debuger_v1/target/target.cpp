// target.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MySnapshoot.h"

int _tmain(int argc, _TCHAR* argv[])
{
	MOD_SNAPSHOOT p;
	MyGetSnapshoot(&p);
	MODULE_INFO temp;
	int k = 0;
	do{
		k = MyNextModule(&p, k, &temp);
		wprintf(L"%s\t\t%x\t\t%x\t\n", temp.ModName.c_str(), temp.hMod, temp.dwModSize);
	} while (k >= 0);
	return 0;
}

