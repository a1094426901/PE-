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
#include <string>
#pragma comment(lib,"comctl32.lib")
using namespace std;
HWND hListModule;//ģ�����
HWND hListProcess;//���̿���

//����
INT_PTR   CALLBACK PEInfoPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
);
//�鿴�ļ�����Ϣ
  string filePath ="";
  string fileName = "";
  string exten = "";
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

string TCHAR2STRING(TCHAR* str) {
	std::string strstr;
	try
	{
		int iLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

		char* chRtn = new char[iLen * sizeof(char)];

		WideCharToMultiByte(CP_ACP, 0, str, -1, chRtn, iLen, NULL, NULL);

		strstr = chRtn;
	}
	catch (exception e)
	{
	}

	return strstr;
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
////������
DWORD SelectFileOpen() {
	OPENFILENAME ofn = { 0 };
	TCHAR strFileName[MAX_PATH] = { 0 };	//���ڽ����ļ���
	ofn.lStructSize = sizeof(OPENFILENAME);	//�ṹ���С
	ofn.hwndOwner = NULL;					//ӵ���Ŵ��ھ��
	ofn.lpstrFilter = TEXT("All\0*.*\exe\0*.dll\0\0");	//���ù���
	ofn.nFilterIndex = 1;	//����������
	ofn.lpstrFile = strFileName;	//���շ��ص��ļ�����ע���һ���ַ���ҪΪNULL
	ofn.nMaxFile = sizeof(strFileName);	//����������
	ofn.lpstrInitialDir = NULL;			//��ʼĿ¼ΪĬ��
	ofn.lpstrTitle = TEXT("��ѡ��һ���ļ�"); //���ڱ���
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY; //�ļ���Ŀ¼������ڣ�����ֻ��ѡ��
	//���ļ��Ի���
	if (GetOpenFileName(&ofn)) {
		 filePath = TCHAR2STRING(strFileName);
		int start = filePath.find_last_of('\\');
		int end = filePath.find_last_of('.');
		 fileName = filePath.substr(start + 1, end - start - 1);
		 exten = filePath.substr(end, filePath.length() - end);
	
		
		DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG_PE), NULL, PEInfoPro, 0);

	}
	else {
		MessageBox(NULL, TEXT("��ѡ��һ�ļ�"), NULL, MB_ICONERROR);
	}

	return 0;
}
//ReadFile��ȡfilebuffer
DWORD ReadPEFile(IN LPSTR lpszFile, OUT LPVOID* pFileBuffer) {
	FILE* file = NULL;
	LPVOID pTempFileBuffer = NULL;
	DWORD nFileLen = 0;

	file = fopen(lpszFile, "rb");
	if (file == NULL) {
		printf("���ļ�����!");
		return 0;
	}

	fseek(file, 0, SEEK_END);

	nFileLen = ftell(file);

	printf("The file pointer is at byte  %d \n", nFileLen);
	fseek(file, 0, SEEK_SET);
	//...

	//�����ڴ�
	pTempFileBuffer = malloc(nFileLen);

	//�ж������ڴ�ɹ�û
	if (pTempFileBuffer == NULL) {
		printf("�����ڴ�ʧ��");
		fclose(file);
		return 0;
	}

	//���ļ���ȡ��������
	size_t n = fread(pTempFileBuffer, nFileLen, 1, file);
	if (!n) {
		printf("��ȡ����ʧ��");
		free(pTempFileBuffer);
		fclose(file);
		return 0;
	}

	printf("pTempFileBuffer:%x\n", pTempFileBuffer);

	//�ͷ��ļ�
	*pFileBuffer = pTempFileBuffer;
	pTempFileBuffer = NULL;
	fclose(file);
	return nFileLen;

}

//PE info��ʾ
void PEInfo(HWND  hwnd) {
	//��ʾ�򿪵��ļ�·��
	HWND hnd=GetDlgItem(hwnd, IDC_STATIC_FilePath);
	SetWindowTextA(hnd,filePath.c_str());

	//��ʾ��ڵ����Ϣ
	//imageͷ
	LPVOID pTempFileBuffer = NULL;
	//PE
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;

	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	//ǿת
	LPSTR filepath = const_cast<char*>(filePath.c_str());
	PVOID FileBuffer = NULL;
	DWORD fileLenth = ReadPEFile(filepath,&FileBuffer);

	//��ֵ
	pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)FileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	
	//��ڵ�
	DWORD temp = pOptionHeader->AddressOfEntryPoint;
	char buf[100];
	sprintf(buf, "%X", temp);
	 hnd = GetDlgItem(hwnd, IDC_EDIT1_RUKOUDIAN);
	SetWindowTextA(hnd, buf);
	//�����ַ
	temp = pOptionHeader->ImageBase;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT2_JINGXIANGJIZHI);
	SetWindowTextA(hnd, buf);
	//�����С
	temp = pOptionHeader->ImageBase;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT3_JINGXIANGDAXIAO);
	SetWindowTextA(hnd, buf);
	//�����ַ
	temp = pOptionHeader->BaseOfCode;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT4_DAIMAJIZHI);
	SetWindowTextA(hnd, buf);
	//���ݻ�ַ
	temp = pOptionHeader->BaseOfData;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT5_SHUJUJIZHI);
	SetWindowTextA(hnd, buf);
	//�ڴ����
	temp = pOptionHeader->SectionAlignment;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT6_NEICUNDUIQI);
	SetWindowTextA(hnd, buf);
	//�ļ�����
	temp = pOptionHeader->FileAlignment;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT7_WENJIANDUIQI);
	SetWindowTextA(hnd, buf);
	//��־��
	temp = pNTHeader->Signature;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT8_BIAOZHIZI);
	SetWindowTextA(hnd, buf);
	//��ϵͳ
	temp = pOptionHeader->Subsystem;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT9_ZIXITONG);
	SetWindowTextA(hnd, buf);
	//������Ŀ
	temp = pPEHeader->NumberOfSections;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT10_QUDUANSHUMU);
	SetWindowTextA(hnd, buf);
	//ʱ���
	temp = pPEHeader->TimeDateStamp;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT11_SHIJIANCHUO);
	SetWindowTextA(hnd, buf);
	//PEͷ��С
	temp = pOptionHeader->SizeOfHeaders;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT12_PETOUDAXIAO);
	SetWindowTextA(hnd, buf);
	//����ֵ
	temp = pPEHeader->Characteristics;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT13_TEZHENGZHI);
	SetWindowTextA(hnd, buf);
	//У���
	temp = pOptionHeader->CheckSum;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT14_JIAOYANHE);
	SetWindowTextA(hnd, buf);
	//��ѡPEͷ
	temp = pPEHeader->SizeOfOptionalHeader;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT15_KEXUANPETOU);
	SetWindowTextA(hnd, buf);
	//Ŀ¼����Ŀ
	temp = pOptionHeader->NumberOfRvaAndSizes;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT16_MULUXIANGSHUMU);
	SetWindowTextA(hnd, buf);
}


INT_PTR   CALLBACK SectionPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
	switch (uMsg) {

	case WM_CLOSE:
	{
		//�رնԻ���
		EndDialog(hwnd, uMsg);
		return 1;
	}
	}

	return 0;
}
INT_PTR   CALLBACK DirPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
	switch (uMsg) {

		case WM_CLOSE:
		{
			//�رնԻ���
			EndDialog(hwnd, uMsg);
			return 1;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BUTTON2_GUANBI:
			{
				//�رնԻ���
				EndDialog(hwnd, uMsg);
				return 1;
			}
	
			}
		}
	}

	return 0;
}

INT_PTR   CALLBACK PEInfoPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
	switch (uMsg) {

		case WM_CLOSE:
		{
			//�رնԻ���
			EndDialog(hwnd, uMsg);
			return 1;
		}

		//��ʼ��ʱ��
		case WM_INITDIALOG: {
			PEInfo(hwnd);
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BUTTON_GUANBI:
			{
				//�رնԻ���
				EndDialog(hwnd, uMsg);
				return 1;
			}
			case IDC_BUTTON1_QUDUAN:
			{
				DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG_SectionTable), NULL, SectionPro, 0);
				return 1;
			}
			case IDC_BUTTON3_MULU:
			{
				DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG1_MULU), NULL, DirPro, 0);
				return 1;
			}
			}
		}
		return 1;
	}

	return 0;
}
INT_PTR   CALLBACK AboutPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
	switch (uMsg) {

	case WM_CLOSE:
	{
		//�رնԻ���
		EndDialog(hwnd, uMsg);
		return 1;
	}
	}

	return 0;
}
//������..
INT_PTR   CALLBACK MainDlgProc(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
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
		case IDC_BUTTON_About:
			///���ع��ڽ���
			DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG_ABOUTAIALOG), NULL, AboutPro, 0);
		
			return 1;
		case IDC_BUTTON_PE:
			SelectFileOpen();
			
			return 1;
		}
	case IDC_BUTTON_Exit://�˳���ť
		PostQuitMessage(0);
		return 1;
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



