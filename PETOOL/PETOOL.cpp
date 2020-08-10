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
#include <string>
#pragma comment(lib,"comctl32.lib")
using namespace std;
HWND hListModule;//模块框句柄
HWND hListProcess;//进程框句柄
HWND hListSection;//节表句柄

//声明
INT_PTR   CALLBACK PEInfoPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
);
//查看文件的信息
  string filePath ="";
  string fileName = "";
  string exten = "";
  PIMAGE_OPTIONAL_HEADER32 pOptionHeader = NULL;
  PIMAGE_FILE_HEADER  pPEHeader = NULL;

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
////处理方法
DWORD SelectFileOpen() {
	OPENFILENAME ofn = { 0 };
	TCHAR strFileName[MAX_PATH] = { 0 };	//用于接收文件名
	ofn.lStructSize = sizeof(OPENFILENAME);	//结构体大小
	ofn.hwndOwner = NULL;					//拥有着窗口句柄
	ofn.lpstrFilter = TEXT("All\0*.*\exe\0*.dll\0\0");	//设置过滤
	ofn.nFilterIndex = 1;	//过滤器索引
	ofn.lpstrFile = strFileName;	//接收返回的文件名，注意第一个字符需要为NULL
	ofn.nMaxFile = sizeof(strFileName);	//缓冲区长度
	ofn.lpstrInitialDir = NULL;			//初始目录为默认
	ofn.lpstrTitle = TEXT("请选择一个文件"); //窗口标题
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY; //文件、目录必须存在，隐藏只读选项
	//打开文件对话框
	if (GetOpenFileName(&ofn)) {
		 filePath = TCHAR2STRING(strFileName);
		int start = filePath.find_last_of('\\');
		int end = filePath.find_last_of('.');
		 fileName = filePath.substr(start + 1, end - start - 1);
		 exten = filePath.substr(end, filePath.length() - end);
	
		
		DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG_PE), NULL, PEInfoPro, 0);

	}
	else {
		MessageBox(NULL, TEXT("请选择一文件"), NULL, MB_ICONERROR);
	}

	return 0;
}
//ReadFile读取filebuffer
DWORD ReadPEFile(IN LPSTR lpszFile, OUT LPVOID* pFileBuffer) {
	FILE* file = NULL;
	LPVOID pTempFileBuffer = NULL;
	DWORD nFileLen = 0;

	file = fopen(lpszFile, "rb");
	if (file == NULL) {
		printf("打开文件错误!");
		return 0;
	}

	fseek(file, 0, SEEK_END);

	nFileLen = ftell(file);

	printf("The file pointer is at byte  %d \n", nFileLen);
	fseek(file, 0, SEEK_SET);
	//...

	//申请内存
	pTempFileBuffer = malloc(nFileLen);

	//判断申请内存成功没
	if (pTempFileBuffer == NULL) {
		printf("申请内存失败");
		fclose(file);
		return 0;
	}

	//将文件读取到缓冲区
	size_t n = fread(pTempFileBuffer, nFileLen, 1, file);
	if (!n) {
		printf("读取数据失败");
		free(pTempFileBuffer);
		fclose(file);
		return 0;
	}

	printf("pTempFileBuffer:%x\n", pTempFileBuffer);

	//释放文件
	*pFileBuffer = pTempFileBuffer;
	pTempFileBuffer = NULL;
	fclose(file);
	return nFileLen;

}

//PE info显示
void PEInfo(HWND  hwnd) {
	//显示打开的文件路径
	HWND hnd=GetDlgItem(hwnd, IDC_STATIC_FilePath);
	SetWindowTextA(hnd,filePath.c_str());

	//显示入口点等信息
	//image头
	LPVOID pTempFileBuffer = NULL;
	//PE
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	

	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	//强转
	LPSTR filepath = const_cast<char*>(filePath.c_str());
	PVOID FileBuffer = NULL;
	DWORD fileLenth = ReadPEFile(filepath,&FileBuffer);

	//赋值
	pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)FileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	
	//入口点
	DWORD temp = pOptionHeader->AddressOfEntryPoint;
	char buf[100];
	sprintf(buf, "%X", temp);
	 hnd = GetDlgItem(hwnd, IDC_EDIT1_RUKOUDIAN);
	SetWindowTextA(hnd, buf);
	//镜像基址
	temp = pOptionHeader->ImageBase;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT2_JINGXIANGJIZHI);
	SetWindowTextA(hnd, buf);
	//镜像大小
	temp = pOptionHeader->ImageBase;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT3_JINGXIANGDAXIAO);
	SetWindowTextA(hnd, buf);
	//代码基址
	temp = pOptionHeader->BaseOfCode;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT4_DAIMAJIZHI);
	SetWindowTextA(hnd, buf);
	//数据基址
	temp = pOptionHeader->BaseOfData;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT5_SHUJUJIZHI);
	SetWindowTextA(hnd, buf);
	//内存对齐
	temp = pOptionHeader->SectionAlignment;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT6_NEICUNDUIQI);
	SetWindowTextA(hnd, buf);
	//文件对齐
	temp = pOptionHeader->FileAlignment;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT7_WENJIANDUIQI);
	SetWindowTextA(hnd, buf);
	//标志字
	temp = pNTHeader->Signature;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT8_BIAOZHIZI);
	SetWindowTextA(hnd, buf);
	//子系统
	temp = pOptionHeader->Subsystem;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT9_ZIXITONG);
	SetWindowTextA(hnd, buf);
	//区段数目
	temp = pPEHeader->NumberOfSections;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT10_QUDUANSHUMU);
	SetWindowTextA(hnd, buf);
	//时间戳
	temp = pPEHeader->TimeDateStamp;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT11_SHIJIANCHUO);
	SetWindowTextA(hnd, buf);
	//PE头大小
	temp = pOptionHeader->SizeOfHeaders;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT12_PETOUDAXIAO);
	SetWindowTextA(hnd, buf);
	//特征值
	temp = pPEHeader->Characteristics;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT13_TEZHENGZHI);
	SetWindowTextA(hnd, buf);
	//校验和
	temp = pOptionHeader->CheckSum;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT14_JIAOYANHE);
	SetWindowTextA(hnd, buf);
	//可选PE头
	temp = pPEHeader->SizeOfOptionalHeader;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT15_KEXUANPETOU);
	SetWindowTextA(hnd, buf);
	//目录项数目
	temp = pOptionHeader->NumberOfRvaAndSizes;
	sprintf(buf, "%X", temp);
	hnd = GetDlgItem(hwnd, IDC_EDIT16_MULUXIANGSHUMU);
	SetWindowTextA(hnd, buf);
}
//显示16个数据项
void ShowDataDirectory(HWND hwnd) {
	PDWORD DATA = (PDWORD)((DWORD)pOptionHeader + 96);

	char buf[100];
	//开始输出可选头信息
	printf("pe可选投： %x\n", *(PWORD)pOptionHeader);
	printf("目录表结构开始%X \n", *(PDWORD)DATA);
	printf("导出表地址:%x ,  导出表大小:%x \n", pOptionHeader->DataDirectory[0].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[0].VirtualAddress);
	HWND hnd = GetDlgItem(hwnd, IDC_EDIT1_SHUCHURVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[0].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT2_SHUCHUSIZE);
	SetWindowTextA(hnd, buf);
	printf("导入表地址:%x ,  导入表大小:%x \n", pOptionHeader->DataDirectory[1].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[1].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT3_SHURURVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[1].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT4_SHUCHUSIZE);
	SetWindowTextA(hnd, buf);

	printf("资源表地址:%x ,  资源表大小:%x \n", pOptionHeader->DataDirectory[2].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[2].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT5_ZIYUANRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[2].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT6_ZIYUANSIZE);
	SetWindowTextA(hnd, buf);
	printf("异常表地址:%x ,  异常表大小:%x \n", pOptionHeader->DataDirectory[3].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[3].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT7_YICHANGRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[3].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT8_YICHANGSIZE);
	SetWindowTextA(hnd, buf);
	printf("安全证书表地址:%x , 安全证书表大小:%x \n", pOptionHeader->DataDirectory[4].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[4].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT9_ANQUANRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[4].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT10_ANQUANSIZE);
	SetWindowTextA(hnd, buf);
	printf("重定位表地址:%x ,  重定位表大小:%x \n", pOptionHeader->DataDirectory[5].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[5].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT11_CONGDINGWEIRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[5].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT12_CHONGDINGWEISIZE);
	SetWindowTextA(hnd, buf);
	printf("调试信息地址:%x ,  调试信息表大小:%x \n", pOptionHeader->DataDirectory[6].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[6].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT13_TIAOSHIRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[6].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT14_TIAOSHISIZE);
	SetWindowTextA(hnd, buf);
	printf("版权所有地址:%x ,  版权所有表大小:%x \n", pOptionHeader->DataDirectory[7].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[7].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT15_BANQUANRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[7].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT16_BANQUANSIZE);
	SetWindowTextA(hnd, buf);
	printf("全局指针表地址:%x ,  全局指针表大小:%x \n", pOptionHeader->DataDirectory[8].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[8].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT17_QUANJUZHIZHENRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[8].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT18_QUANJUZHIZHENSIZE);
	SetWindowTextA(hnd, buf);
	printf("TLS表地址:%x ,  TLS表表大小:%x \n", pOptionHeader->DataDirectory[9].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[9].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT19_TLSRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[9].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT20_TLSSIZE);
	SetWindowTextA(hnd, buf);
	printf("加载配置表地址:%x ,  加载配置表大小:%x \n", pOptionHeader->DataDirectory[10].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[10].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT21_DAORUPEIZHIRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[10].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT22_DAORUPEIZHISIZE);
	SetWindowTextA(hnd, buf);
	printf("绑定导入表地址:%x , 绑定导入表大小:%x \n", pOptionHeader->DataDirectory[11].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[11].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT23_BANGDINGDAORURVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[11].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT24_BANGDINGDAORUSIZE);
	SetWindowTextA(hnd, buf);
	printf("IAT表地址:%x ,  IAT表大小:%x \n", pOptionHeader->DataDirectory[12].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[12].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT25_IATRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[12].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT26_IATSIZE);
	SetWindowTextA(hnd, buf);
	printf("延迟导入表地址:%x ,  延迟导入表大小:%x \n", pOptionHeader->DataDirectory[13].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[13].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT27_YANCHIDAORURVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[13].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT28_YANCHIDAORUSIZE);
	SetWindowTextA(hnd, buf);
	printf("COM表地址:%x ,  COM表大小:%x \n", pOptionHeader->DataDirectory[14].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[14].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT29_COMRVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[14].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT30_COMSIZE);
	SetWindowTextA(hnd, buf);
	printf("保留表地址:%x ,  保留表大小:%x \n", pOptionHeader->DataDirectory[15].VirtualAddress, pOptionHeader->DataDirectory[0].Size);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[15].VirtualAddress);
	 hnd = GetDlgItem(hwnd, IDC_EDIT31_BAOLIURVA);
	SetWindowTextA(hnd, buf);
	sprintf(buf, "%X", pOptionHeader->DataDirectory[15].Size);
	hnd = GetDlgItem(hwnd, IDC_EDIT32_BAOLIUSIZE);
	SetWindowTextA(hnd, buf);

}
/******************************************************************************************
Function:        ConvertCharToLPWSTR
Description:     const char *转LPWSTR
Input:           str:待转化的const char *类型字符串
Return:          转化后的LPWSTR类型字符串
*******************************************************************************************/
LPWSTR ConvertCharToLPWSTR(const char* szString)
{
	int dwLen = strlen(szString) + 1;
	int nwLen = MultiByteToWideChar(CP_ACP, 0, szString, dwLen, NULL, 0);//算出合适的长度
	LPWSTR lpszPath = new WCHAR[dwLen];
	MultiByteToWideChar(CP_ACP, 0, szString, dwLen, lpszPath, nwLen);
	return lpszPath;
}
void EnumSectionInfo() {


	PIMAGE_SECTION_HEADER	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	DWORD sectionnum = (DWORD)pPEHeader->NumberOfSections;

	for (int sec=0;sec<sectionnum;sec++	)
	{
		LV_ITEM LvItem;
		memset(&LvItem, 0, sizeof(LV_ITEM));
		char Temp[100];
		LvItem.mask = LVIF_TEXT;   // Text Style
		LvItem.cchTextMax = 256; // Max size of test
		LvItem.iItem = sec;          // choose item  
		//LvItem.iSubItem = sec;       // Put in first coluom
		
		LvItem.pszText = ConvertCharToLPWSTR((CHAR*)pSectionHeader->Name); // Text to display (can be from a char variable) (Items)

		SendMessage(hListSection, LVM_INSERTITEM, 0, (LPARAM)&LvItem); // Send info to the Listview
	
	
			LvItem.iSubItem = 1;
			sprintf(Temp, "%x", pSectionHeader->VirtualAddress);
			LvItem.pszText = ConvertCharToLPWSTR(Temp);
			SendMessage(hListSection, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems
	

			LvItem.iSubItem = 2;
			sprintf(Temp, "%x", pSectionHeader->Misc.VirtualSize);
			LvItem.pszText = ConvertCharToLPWSTR(Temp);
			SendMessage(hListSection, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			LvItem.iSubItem = 3;
			sprintf(Temp, "%x", pSectionHeader->PointerToRawData);
			LvItem.pszText = ConvertCharToLPWSTR(Temp);
			SendMessage(hListSection, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			LvItem.iSubItem = 4;
			sprintf(Temp, "%x", pSectionHeader->SizeOfRawData);
			LvItem.pszText = ConvertCharToLPWSTR(Temp);
			SendMessage(hListSection, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

			LvItem.iSubItem = 5;
			sprintf(Temp, "%x", pSectionHeader->Characteristics);
			LvItem.pszText = ConvertCharToLPWSTR(Temp);
			SendMessage(hListSection, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems


			pSectionHeader = pSectionHeader + 1;
	}
		

	/*	lv_item.pszText = LPWSTR(pSectionHeader->Misc.VirtualSize);
		lv_item.iItem = sec;
		lv_item.iSubItem = 2;
		ListView_SetItem(hListSection, &lv_item);

		lv_item.pszText = LPWSTR(pSectionHeader->PointerToRawData);
		lv_item.iItem = sec;
		lv_item.iSubItem =3;
		ListView_SetItem(hListSection, &lv_item);

		lv_item.pszText = LPWSTR(pSectionHeader->SizeOfRawData);
		lv_item.iItem = sec;
		lv_item.iSubItem = 4;
		ListView_SetItem(hListSection, &lv_item);

		lv_item.pszText = LPWSTR(pSectionHeader->Characteristics);
		lv_item.iItem = sec;
		lv_item.iSubItem = 5;
		ListView_SetItem(hListSection, &lv_item);*/



		//

}
//初始化显示节表信息
void InitSectionInfo(HWND hwnd) {
	LV_COLUMN lv;
	//初始化
	memset(&lv, 0, sizeof(LV_COLUMN));

	///设置整行选中
	SendMessage(hListSection, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	//第一列
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lv.pszText = LPWSTR(L"名称");
	lv.cx = 100;
	lv.iSubItem = 0;
	
	SendMessage(hListSection, LVM_INSERTCOLUMN, 0, (DWORD)&lv);

	//第二列
	lv.pszText = LPWSTR(L"V.内存中偏移");
	lv.cx = 100;
	lv.iSubItem = 1;
	SendMessage(hListSection, LVM_INSERTCOLUMN, 1, (DWORD)&lv);

	//第三列
	lv.pszText = LPWSTR(L"V.内存中大小");
	lv.cx = 100;
	lv.iSubItem = 2;
	SendMessage(hListSection, LVM_INSERTCOLUMN, 2, (DWORD)&lv);
		
	//第四列
	lv.pszText = LPWSTR(L"R.文件中偏移");
	lv.cx = 100;
	lv.iSubItem = 3;
	SendMessage(hListSection, LVM_INSERTCOLUMN, 3, (DWORD)&lv);

	//第五列
	lv.pszText = LPWSTR(L"R.文件中大小");
	lv.cx = 100;
	lv.iSubItem = 4;
	SendMessage(hListSection, LVM_INSERTCOLUMN, 4, (DWORD)&lv);
	//第六列
	lv.pszText = LPWSTR(L"标志");
	lv.cx = 100;
	lv.iSubItem =5;
	SendMessage(hListSection, LVM_INSERTCOLUMN, 5, (DWORD)&lv);


	//数据插入
	EnumSectionInfo();
}


INT_PTR   CALLBACK SectionPro(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
	switch (uMsg) {
		//初始化时候
		case WM_INITDIALOG: {
		hListSection= GetDlgItem(hwnd, IDC_LIST2_SectionTable);
		InitSectionInfo(hwnd);
		return 1;

	}
		case WM_CLOSE:
		{
			//关闭对话框
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

		//初始化时候
		case WM_INITDIALOG: {
			ShowDataDirectory(hwnd);
			return 1;

		}

		case WM_CLOSE:
		{
			//关闭对话框
			EndDialog(hwnd, uMsg);
			return 1;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BUTTON2_GUANBI:
			{
				//关闭对话框
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
			//关闭对话框
			EndDialog(hwnd, uMsg);
			return 1;
		}

		//初始化时候
		case WM_INITDIALOG: {
			PEInfo(hwnd);
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_BUTTON_GUANBI:
			{
				//关闭对话框
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
				//显示目录


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
		//关闭对话框
		EndDialog(hwnd, uMsg);
		return 1;
	}
	}

	return 0;
}
//主窗口..
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
		case IDC_BUTTON_About:
			///加载关于界面
			DialogBoxW(NULL, MAKEINTRESOURCE(IDD_DIALOG_ABOUTAIALOG), NULL, AboutPro, 0);
		
			return 1;
		case IDC_BUTTON_PE:
			SelectFileOpen();
			
			return 1;
		}
	case IDC_BUTTON_Exit://退出按钮
		PostQuitMessage(0);
		return 1;
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



