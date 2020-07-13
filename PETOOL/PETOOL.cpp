/********************************************************************
	created:	2020/07/13   10:29
	filename: 	H:\2020-2021\PECODE\滴水初级\PE\PE\PETOOL\PETOOL.cpp
	file base:	PETOOL
	author:		大海
	
	purpose:	Practice makes perfect.
*********************************************************************/
#define _CRT_SECURE_NO_WARNINGS //省略_CRT_SECURE_NO_WARNINGS这种提示

#include <iostream>
#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <tlhelp32.h>	//进程快照函数头文件
#include <stdlib.h>//itoa 头文件
#include "Toolhelp.h"
#include <strsafe.h>//StringCchPrintf
#include <Shlwapi.h>//StrFormatKBSize
//非通用控件
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")

HWND hListModule;//模块框句柄
HWND hListProcess;//进程框句柄
///////////////////////////////////////////////////////////////////////////////


PVOID GetModulePreferredBaseAddr(DWORD dwProcessId, PVOID pvModuleRemote) {

	PVOID pvModulePreferredBaseAddr = NULL;
	IMAGE_DOS_HEADER idh;
	IMAGE_NT_HEADERS inth;

	// Read the remote module's DOS header
	Toolhelp32ReadProcessMemory(dwProcessId,
		pvModuleRemote, &idh, sizeof(idh), NULL);

	// Verify the DOS image header
	if (idh.e_magic == IMAGE_DOS_SIGNATURE) {
		// Read the remote module's NT header
		Toolhelp32ReadProcessMemory(dwProcessId,
			(PBYTE)pvModuleRemote + idh.e_lfanew, &inth, sizeof(inth), NULL);

		// Verify the NT image header
		if (inth.Signature == IMAGE_NT_SIGNATURE) {
			// This is valid NT header, get the image's preferred base address
			pvModulePreferredBaseAddr = (PVOID)inth.OptionalHeader.ImageBase;
		}
	}
	return(pvModulePreferredBaseAddr);
}


///////////////////////////////////////////////////////////////////////////////
void __cdecl OutputDebugStringF(const char* format, ...)
{
	va_list vlArgs;
	char* strBuffer = (char*)GlobalAlloc(GPTR, 4096);

	va_start(vlArgs, format);
	_vsnprintf_s(strBuffer, 4096 - 1,_TRUNCATE, format, vlArgs);
	va_end(vlArgs);


	OutputDebugStringA(strBuffer);
	GlobalFree(strBuffer);
	return;
}
DWORD GetProcessIdFromName(WCHAR* name,OUT PROCESSENTRY32* peinfo)
{

	HANDLE  hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hsnapshot == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot Error!\n");
		return 0;
	}
	PROCESSENTRY32 pe;
	

	pe.dwSize = sizeof(PROCESSENTRY32);

	int flag = Process32First(hsnapshot, &pe);

	while (flag != 0)
	{
		if (wcscmp(pe.szExeFile, name)== 0)
		{
			*peinfo = pe;
			
			CloseHandle(hsnapshot);

			return pe.th32ProcessID;
		}
		flag = Process32Next(hsnapshot, &pe);
	}
	CloseHandle(hsnapshot);

	return 0;
}

void EnumModules(LPWSTR hListName,DWORD pid) {
	ListView_DeleteItem(hListModule, 0);

	LV_ITEM lv_item;
	memset(&lv_item, 0, sizeof(LV_ITEM));
	lv_item.mask = LVIF_TEXT;//// lvif 是item 

	int countModule = 0;								//当前模块数量计数变量
	PROCESSENTRY32 pe32 = { sizeof(pe32) };;                    //保存进程信息
	MODULEENTRY32 me32 = { sizeof(me32) };                   //保存模块信息
	pe32.dwSize = sizeof(PROCESSENTRY32);//初始化，分配内存空间

	DWORD processId = 1;//GetProcessIdFromName(hListName,  &pe32);
		if (processId==0) {
			OutputDebugStringF("获取进程信息失败!");
		}
		else {
	
				CToolhelp thModules(TH32CS_SNAPMODULE, pid);

				//bool bModule = Module32First(hModule, &me32);      //获取第一个模块信息,即进程相应可执行文件的信息
				int err = GetLastError();
				
				BOOL fOk = thModules.ModuleFirst(&me32);
				for (; fOk; fOk = thModules.ModuleNext(&me32)) {
					OutputDebugStringF("模块:\t%d\t%ls\n", countModule, me32.szExePath);
					if (me32.ProccntUsage == 65535) {
						// Module was implicitly loaded and cannot be unloaded
						OutputDebugStringF("Module was implicitly loaded and cannot be unloaded\n");
					}
					else {
						OutputDebugStringF( "  %5d\n", (me32.ProccntUsage));
					}

					lv_item.pszText = LPWSTR(me32.szModule);
					lv_item.iItem = countModule;
					lv_item.iSubItem = 0;
					SendMessage(hListModule, LVM_INSERTITEM, countModule, (DWORD)&lv_item);

				
			
					OutputDebugStringF("modBaseAddrL:  %x\n", me32.modBaseAddr);
				
					lv_item.pszText = LPWSTR(L"hi");;
					lv_item.iItem = countModule;
					lv_item.iSubItem = 1;
					ListView_SetItem(hListModule, &lv_item);

				/*	lv_item.pszText = LPWSTR("ttttttttt");
					lv_item.iItem = 1;
					lv_item.iSubItem = 2;
					SendMessage(hListModule, LVM_INSERTITEM, countModule, (DWORD)&lv_item);*/


					countModule++;
				}


		}
	

		


}

void EnumProcess(HWND hListProcess) {
	LV_ITEM lv_item;
	memset(&lv_item, 0, sizeof(LV_ITEM));
	lv_item.mask = LVIF_TEXT;//// lvif 是item 

	int countProcess = 0;									//当前进程数量计数变量
	PROCESSENTRY32 currentProcess;						//存放快照进程信息的一个结构体
	currentProcess.dwSize = sizeof(currentProcess);		//在使用这个结构之前，先设置它的大小
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//给系统内的所有进程拍一个快照

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot()调用失败!\n");

	}
	else {
		bool bMore = Process32First(hProcess, &currentProcess);	//获取第一个进程信息
		while (bMore)
		{
			printf("PID=%5u    PName= %ls\n", currentProcess.th32ProcessID, currentProcess.szExeFile);	//遍历进程快照，轮流显示每个进程信息
			lv_item.pszText = (currentProcess.szExeFile);
			lv_item.iItem = countProcess;
			lv_item.iSubItem = 0;
			SendMessage(hListProcess, LVM_INSERTITEM, 0, (DWORD)&lv_item);

			WCHAR szBuffer[25] = L"";
			
			wsprintfW(szBuffer, LPWSTR(L"%d"), currentProcess.th32ProcessID);//itoa(currentProcess.th32ProcessID, str, 10); 十进制转字符串

			lv_item.pszText = szBuffer;
			lv_item.iItem = countProcess;
			lv_item.iSubItem = 1;
			ListView_SetItem(hListProcess, &lv_item);

			lv_item.pszText = LPWSTR("");
			lv_item.iItem = countProcess;
			lv_item.iSubItem = 2;
			ListView_SetItem(hListProcess, &lv_item);

			bMore = Process32Next(hProcess, &currentProcess);	//遍历下一个
			countProcess++;
		}

		CloseHandle(hProcess);	//清除hProcess句柄

	}

}

void InitProcessListView() {
	LV_COLUMN lv;
	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));

	///设置整行选中
	SendMessage(hListProcess, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = LPWSTR((L"进程名"));
	lv.cx = 150;
	lv.iSubItem = 0;
	SendMessage(hListProcess, LVM_INSERTCOLUMN, 0, (DWORD)&lv);

	//第二列
	lv.pszText = LPWSTR(L"PID");
	lv.cx = 50;
	lv.iSubItem = 1;
	SendMessage(hListProcess, LVM_INSERTCOLUMN, 1, (DWORD)&lv);

	//第三列
	lv.pszText = LPWSTR(L"镜像地址");
	lv.cx = 100;
	lv.iSubItem = 2;
	ListView_InsertColumn(hListProcess, 2, &lv);

	//第四列
	lv.pszText = LPWSTR(L"镜像大小");
	lv.cx = 100;
	lv.iSubItem = 3;
	ListView_InsertColumn(hListProcess, 3, &lv);

	EnumProcess(hListProcess);

}
void InitModuleListView() {
	LV_COLUMN lv;
	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));

	///设置整行选中
	SendMessage(hListModule, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = LPWSTR(L"模块名");
	lv.cx = 250;
	lv.iSubItem = 0;
	SendMessage(hListModule, LVM_INSERTCOLUMN, 0, (DWORD)&lv);

	//第二列
	lv.pszText = LPWSTR(L"模块位置");
	lv.cx = 100;
	lv.iSubItem = 1;
	SendMessage(hListModule, LVM_INSERTCOLUMN, 1, (DWORD)&lv);

	//第三列
	lv.pszText = LPWSTR(L"模块大小");
	lv.cx = 100;
	lv.iSubItem = 2;
	SendMessage(hListModule, LVM_INSERTCOLUMN, 1, (DWORD)&lv);

}
INT_PTR   CALLBACK MainDlgProc(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter

)
{
	BOOL bRet = true;
	switch (uMsg) {
	case WM_DESTROY:

		PostQuitMessage(0);
		return 1;
	case WM_MOVE:
	{

		return 1;
	}
	case WM_INITDIALOG:
	{
		///获取句柄
		hListProcess = GetDlgItem(hwnd, IDC_LIST_PRO);
		hListModule= GetDlgItem(hwnd, IDC_LIST_MODULE);

		InitProcessListView();
		InitModuleListView();
		return 1;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 1;
	}
	case WM_COMMAND:
	{
		OutputDebugStringF("WM_COMMAND...");
		switch (LOWORD(wParam)) {
		case 111:
			OutputDebugStringF("Button...");
			return 1;
		}
	}
	case WM_NOTIFY:
	{
		NMHDR* pNHMDR = (NMHDR*)lParam;
		if (wParam == IDC_LIST_PRO && pNHMDR->code == NM_CLICK) {
			////模块情况
			
			WCHAR szBuffer[MAX_PATH];
			int line = ListView_GetSelectionMark(hListProcess);	 //点选的行数
			ListView_GetItemText(hListProcess, line, 0, szBuffer, MAX_PATH);
			WCHAR PID[10];
			memset(PID, 0, 10);
			DWORD dpid=0;
			ListView_GetItemText(hListProcess, line, 1, PID, 10);
			swscanf(PID,L"%d",&dpid);//wchar 转换成整形
			OutputDebugStringF("%ls %d", szBuffer, dpid);
			EnumModules(szBuffer, dpid);

		}

		return 1;
	}
	}

	return 0;
}
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	//加载全部非通用控件
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);



	///加载主界面
	DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc,0);

	return 0;
}



