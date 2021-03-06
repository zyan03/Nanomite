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
#include "qtDLGNanomite.h"
#include "clsCrashHandler.h"
#include "clsMemManager.h"
#include <WinBase.h>

#include <QtGui/QApplication>
//#include <QDebug>

BOOL IsUserAdmin()
{
    BOOL _isUserAdmin = false;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    if (AllocateAndInitializeSid(&NtAuthority,2,SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&AdministratorsGroup))
    {
        CheckTokenMembership( NULL, AdministratorsGroup, &_isUserAdmin);
        
        FreeSid(AdministratorsGroup);
    }    

    return _isUserAdmin;
}

int main(int argc, char *argv[])
{
	AddVectoredExceptionHandler(1,clsCrashHandler::ErrorReporter);

    if (!IsUserAdmin())
    {
        MessageBoxW(NULL,L"You didn�t start the debugger with admin rights!\r\nThis could cause problems with some features!",L"Nanomite",MB_OK);
    }

	clsMemManager clsMManage = clsMemManager();
	//Tests - 500bytes, 100000 rounds
	//Test using malloc and free:  8750 
	//Test using clsMemManager:  31
	//
	//Test - 1014bytes, 100000 rounds
	//Test using malloc and free:  9187 
	//Test using clsMemManager:  31

	//DWORD dwStartTick = GetTickCount();

	//DWORD pMem[100000];
	//for(int i = 0; i < 100000; i++)
	//{
	//	pMem[i] = (DWORD)malloc(512);
	//}

	//for(int i = 0; i < 100000; i++)
	//{
	//	free((void*)pMem[i]);
	//}
	//qDebug() << "Test using malloc and free: " << GetTickCount() - dwStartTick;

	//
	//dwStartTick = GetTickCount();
	//for(int i = 0; i < 100000; i++)
	//{
	//	pMem[i] = (DWORD)clsMemManager::CAlloc(512);
	//}
	//
	//for(int i = 0; i < 100000; i++)
	//{
	//	clsMemManager::CFree((void*)pMem[i]);
	//}
	//qDebug() << "Test using clsMemManager: " << GetTickCount() - dwStartTick;

	
	QApplication a(argc, argv);
	qtDLGNanomite w;
	w.show();

#ifdef _DEBUG
	return a.exec(); 
#else
	// ugly workaround for cruntime crash caused by new override!
	TerminateProcess(GetCurrentProcess(),a.exec());
#endif
}
