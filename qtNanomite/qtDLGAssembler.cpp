/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "qtDLGAssembler.h"

#include "clsMemManager.h"

#include <QFile>

qtDLGAssembler::qtDLGAssembler(QWidget *parent, Qt::WFlags flags,
	HANDLE hProc,quint64 InstructionOffset,clsDisassembler *pDisAs,bool Is64Bit)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setFixedSize(this->width(),this->height());

	connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(InsertNewInstructions()));

	_Is64Bit = Is64Bit;
	_pDisAs = pDisAs;
	_InstructionOffset = InstructionOffset;
	_hProc = hProc;
}

qtDLGAssembler::~qtDLGAssembler()
{

}

void qtDLGAssembler::InsertNewInstructions()
{
	if(lineEdit->text().length() <= 0) return;

	QMap<QString,DisAsDataRow>::const_iterator i = _pDisAs->SectionDisAs.constFind(QString("%1").arg(_InstructionOffset,16,16,QChar('0')).toUpper());
	if((QMapData::Node *)i == (QMapData::Node *)_pDisAs->SectionDisAs.constEnd()) return;

	QString oldOpcodes = i.value().OpCodes;
	int oldOpcodeLen = oldOpcodes.replace(" ", "").length() / 2,
		newOpcodeLen = NULL;
		
	QFile tempOutput("nanomite.asm");
	tempOutput.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&tempOutput);

	if(_Is64Bit)
		out << "BITS 64\r\n";
	else
		out << "BITS 32\r\n";
	out << lineEdit->text();
	tempOutput.close();


	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si,sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi,sizeof(pi));
	TCHAR szCommandLine[] = L"nasm.exe -o nanomite.bin nanomite.asm";

	if(!CreateProcess(NULL,szCommandLine,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)) 
    {
        MessageBoxW(NULL,L"Error, unable to launch assembler!",L"Nanomite",MB_OK);
        return;
    }

    WaitForSingleObject(pi.hProcess,INFINITE);
	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	DeleteFile(L"nanomite.asm");

	DWORD	BytesWritten = NULL,
			BytesRead = NULL,
			NewProtection = PAGE_EXECUTE_READWRITE,
			OldProtection = NULL;	

	HANDLE hFile = CreateFileW(L"nanomite.bin",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE) return;

	int iLen = GetFileSize(hFile,NULL);
	LPVOID pFileBuffer = clsMemManager::CAlloc(iLen);
	if(!ReadFile(hFile,pFileBuffer,iLen,&BytesRead,NULL))
	{
		CloseHandle(hFile);
		DeleteFile(L"nanomite.bin");
		clsMemManager::CFree(pFileBuffer);
		MessageBoxW(NULL,L"Error, no valid opcodes found!",L"Nanomite",MB_OK);
		close();
		return;
	}
	CloseHandle(hFile);
	DeleteFile(L"nanomite.bin");


	if(BytesRead <= 0)
	{
		clsMemManager::CFree(pFileBuffer);
		MessageBoxW(NULL,L"Error, no valid opcodes found!",L"Nanomite",MB_OK);
		close();
		return;
	}

	if(oldOpcodeLen >= BytesRead)
		newOpcodeLen = oldOpcodeLen;
	else if(oldOpcodeLen < BytesRead)
	{
		newOpcodeLen = oldOpcodeLen;
		while(newOpcodeLen < BytesRead)
		{
			++i;
			if((QMapData::Node *)i == (QMapData::Node *)_pDisAs->SectionDisAs.constEnd()) return;
			oldOpcodes = i.value().OpCodes;
			newOpcodeLen += oldOpcodes.replace(" ", "").length() / 2;
		}
	}

	LPVOID pBuffer = clsMemManager::CAlloc(newOpcodeLen);
	memset(pBuffer,0x90,newOpcodeLen);
	memcpy(pBuffer,pFileBuffer,BytesRead);

	VirtualProtectEx(_hProc,(LPVOID)_InstructionOffset,newOpcodeLen,NewProtection,&OldProtection);
	if(!WriteProcessMemory(_hProc,(LPVOID)_InstructionOffset,pBuffer,newOpcodeLen,(SIZE_T*)&BytesWritten))
	{
		VirtualProtectEx(_hProc,(LPVOID)_InstructionOffset,newOpcodeLen,OldProtection,&NewProtection);
		MessageBoxW(NULL,L"Error while writing the new Buffer!",L"Nanomite",MB_OK);
		clsMemManager::CFree(pBuffer);
		clsMemManager::CFree(pFileBuffer);
		close();
		return;
	}
	
	VirtualProtectEx(_hProc,(LPVOID)_InstructionOffset,newOpcodeLen,OldProtection,&NewProtection);
	clsMemManager::CFree(pBuffer);
	clsMemManager::CFree(pFileBuffer);
	_pDisAs->SectionDisAs.clear();
	emit OnReloadDebugger();
	lineEdit->clear();
	close();
	return;
}