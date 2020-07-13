/********************************************************************
	created:	2020/07/13   10:29
	filename: 	H:\2020-2021\PECODE\��ˮ����\PE\PE\PETOOL\PETOOL.cpp
	file base:	PETOOL
	author:		��
	
	purpose:	Practice makes perfect.
*********************************************************************/
#define _CRT_SECURE_NO_WARNINGS //ʡ��_CRT_SECURE_NO_WARNINGS������ʾ

#include <iostream>
#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <tlhelp32.h>	//���̿��պ���ͷ�ļ�
#include <stdlib.h>//itoa ͷ�ļ�
#include "Toolhelp.h"
#include <strsafe.h>//StringCchPrintf
#include <Shlwapi.h>//StrFormatKBSize
//��ͨ�ÿؼ�
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")

HWND hListModule;//ģ�����
HWND hListProcess;//���̿���
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
	lv_item.mask = LVIF_TEXT;//// lvif ��item 

	int countModule = 0;								//��ǰģ��������������
	PROCESSENTRY32 pe32 = { sizeof(pe32) };;                    //���������Ϣ
	MODULEENTRY32 me32 = { sizeof(me32) };                   //����ģ����Ϣ
	pe32.dwSize = sizeof(PROCESSENTRY32);//��ʼ���������ڴ�ռ�

	DWORD processId = 1;//GetProcessIdFromName(hListName,  &pe32);
		if (processId==0) {
			OutputDebugStringF("��ȡ������Ϣʧ��!");
		}
		else {
	
				CToolhelp thModules(TH32CS_SNAPMODULE, pid);

				//bool bModule = Module32First(hModule, &me32);      //��ȡ��һ��ģ����Ϣ,��������Ӧ��ִ���ļ�����Ϣ
				int err = GetLastError();
				
				BOOL fOk = thModules.ModuleFirst(&me32);
				for (; fOk; fOk = thModules.ModuleNext(&me32)) {
					OutputDebugStringF("ģ��:\t%d\t%ls\n", countModule, me32.szExePath);
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
	lv_item.mask = LVIF_TEXT;//// lvif ��item 

	int countProcess = 0;									//��ǰ����������������
	PROCESSENTRY32 currentProcess;						//��ſ��ս�����Ϣ��һ���ṹ��
	currentProcess.dwSize = sizeof(currentProcess);		//��ʹ������ṹ֮ǰ�����������Ĵ�С
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//��ϵͳ�ڵ����н�����һ������

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot()����ʧ��!\n");

	}
	else {
		bool bMore = Process32First(hProcess, &currentProcess);	//��ȡ��һ��������Ϣ
		while (bMore)
		{
			printf("PID=%5u    PName= %ls\n", currentProcess.th32ProcessID, currentProcess.szExeFile);	//�������̿��գ�������ʾÿ��������Ϣ
			lv_item.pszText = (currentProcess.szExeFile);
			lv_item.iItem = countProcess;
			lv_item.iSubItem = 0;
			SendMessage(hListProcess, LVM_INSERTITEM, 0, (DWORD)&lv_item);

			WCHAR szBuffer[25] = L"";
			
			wsprintfW(szBuffer, LPWSTR(L"%d"), currentProcess.th32ProcessID);//itoa(currentProcess.th32ProcessID, str, 10); ʮ����ת�ַ���

			lv_item.pszText = szBuffer;
			lv_item.iItem = countProcess;
			lv_item.iSubItem = 1;
			ListView_SetItem(hListProcess, &lv_item);

			lv_item.pszText = LPWSTR("");
			lv_item.iItem = countProcess;
			lv_item.iSubItem = 2;
			ListView_SetItem(hListProcess, &lv_item);

			bMore = Process32Next(hProcess, &currentProcess);	//������һ��
			countProcess++;
		}

		CloseHandle(hProcess);	//���hProcess���

	}

}

void InitProcessListView() {
	LV_COLUMN lv;
	//��ʼ��
	memset(&lv, 0, sizeof(LV_COLUMN));

	///��������ѡ��
	SendMessage(hListProcess, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	//��һ��
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = LPWSTR((L"������"));
	lv.cx = 150;
	lv.iSubItem = 0;
	SendMessage(hListProcess, LVM_INSERTCOLUMN, 0, (DWORD)&lv);

	//�ڶ���
	lv.pszText = LPWSTR(L"PID");
	lv.cx = 50;
	lv.iSubItem = 1;
	SendMessage(hListProcess, LVM_INSERTCOLUMN, 1, (DWORD)&lv);

	//������
	lv.pszText = LPWSTR(L"�����ַ");
	lv.cx = 100;
	lv.iSubItem = 2;
	ListView_InsertColumn(hListProcess, 2, &lv);

	//������
	lv.pszText = LPWSTR(L"�����С");
	lv.cx = 100;
	lv.iSubItem = 3;
	ListView_InsertColumn(hListProcess, 3, &lv);

	EnumProcess(hListProcess);

}
void InitModuleListView() {
	LV_COLUMN lv;
	//��ʼ��
	memset(&lv, 0, sizeof(LV_COLUMN));

	///��������ѡ��
	SendMessage(hListModule, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	//��һ��
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = LPWSTR(L"ģ����");
	lv.cx = 250;
	lv.iSubItem = 0;
	SendMessage(hListModule, LVM_INSERTCOLUMN, 0, (DWORD)&lv);

	//�ڶ���
	lv.pszText = LPWSTR(L"ģ��λ��");
	lv.cx = 100;
	lv.iSubItem = 1;
	SendMessage(hListModule, LVM_INSERTCOLUMN, 1, (DWORD)&lv);

	//������
	lv.pszText = LPWSTR(L"ģ���С");
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
		///��ȡ���
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
			////ģ�����
			
			WCHAR szBuffer[MAX_PATH];
			int line = ListView_GetSelectionMark(hListProcess);	 //��ѡ������
			ListView_GetItemText(hListProcess, line, 0, szBuffer, MAX_PATH);
			WCHAR PID[10];
			memset(PID, 0, 10);
			DWORD dpid=0;
			ListView_GetItemText(hListProcess, line, 1, PID, 10);
			swscanf(PID,L"%d",&dpid);//wchar ת��������
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
	//����ȫ����ͨ�ÿؼ�
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);



	///����������
	DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc,0);

	return 0;
}



