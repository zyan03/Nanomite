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
#include "qtDLGDetailInfo.h"
#include "qtDLGPEEditor.h"

#include "clsMemManager.h"
#include "clsHelperClass.h"
#include "clsPEManager.h"

#include <QtCore>
#include <QMenu>

qtDLGDetailInfo::qtDLGDetailInfo(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);

	// List DetInfo  Processes
	tblPIDs->horizontalHeader()->resizeSection(0,135);
	tblPIDs->horizontalHeader()->resizeSection(1,135);
	tblPIDs->horizontalHeader()->resizeSection(2,135);

	// List DetInfo  Threads
	tblTIDs->horizontalHeader()->resizeSection(0,135);
	tblTIDs->horizontalHeader()->resizeSection(1,135);
	tblTIDs->horizontalHeader()->resizeSection(2,135);
	tblTIDs->horizontalHeader()->resizeSection(3,135);

	// List DetInfo  Exceptions
	tblExceptions->horizontalHeader()->resizeSection(0,135);
	tblExceptions->horizontalHeader()->resizeSection(1,135);
	tblExceptions->horizontalHeader()->resizeSection(2,140);

	// List DetInfo  Modules
	tblModules->horizontalHeader()->resizeSection(0,135);
	tblModules->horizontalHeader()->resizeSection(1,135);
	tblModules->horizontalHeader()->resizeSection(2,135);

	connect(tblTIDs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomTIDContextMenu(QPoint)));
	connect(tblPIDs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomPIDContextMenu(QPoint)));
	connect(tblExceptions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomExceptionContextMenu(QPoint)));
	connect(tblModules,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomModuleContextMenu(QPoint)));
}

qtDLGDetailInfo::~qtDLGDetailInfo()
{

}

void qtDLGDetailInfo::OnCustomPIDContextMenu(QPoint qPoint)
{
	if(tblPIDs->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblPIDs->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblPIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomTIDContextMenu(QPoint qPoint)
{
	if(tblTIDs->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblTIDs->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblTIDs->item(_iSelectedRow,2)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	menu.addAction(new QAction("Suspend",this));
	menu.addAction(new QAction("Resume",this));
	QMenu *submenu = menu.addMenu("Set Thread Priority");
	submenu->addAction(new QAction("Highest",this));
	submenu->addAction(new QAction("Above Normal",this));
	submenu->addAction(new QAction("Normal",this));
	submenu->addAction(new QAction("Below Normal",this));
	submenu->addAction(new QAction("Lowest",this));

	connect(submenu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomExceptionContextMenu(QPoint qPoint)
{
	if(tblExceptions->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblExceptions->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblExceptions->item(_iSelectedRow,0)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomModuleContextMenu(QPoint qPoint)
{
	if(tblModules->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblModules->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblModules->item(_iSelectedRow,1)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	menu.addAction(new QAction("Open Module in PE View",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Show Offset in disassembler") == 0)
	{
		if(_SelectedOffset >= 0)
		{
			emit ShowInDisassembler(_SelectedOffset);
			_SelectedOffset = NULL;
		}
	}
	else if(QString().compare(pAction->text(),"Open Module in PE View") == 0)
	{
		std::wstring *temp = new std::wstring(tblModules->item(_iSelectedRow,3)->text().toStdWString());
		emit OpenFileInPEManager(*temp,-1);
		qtDLGPEEditor *dlgPEEditor = new qtDLGPEEditor(clsPEManager::GetInstance(),this,Qt::Window,
			-1,
			*temp);
		dlgPEEditor->show();
	}
	else if(QString().compare(pAction->text(),"Suspend") == 0)
	{
		DWORD ThreadID = tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME,false,ThreadID);
		if(hThread == INVALID_HANDLE_VALUE) 
		{
			MessageBoxW(NULL,L"ERROR, have not been able to open this Thread!",L"Nanomite",MB_OK);
			return;
		}

		if(SuspendThread(hThread) != -1)
			tblTIDs->item(_iSelectedRow,4)->setText("Suspended");
		else
			MessageBoxW(NULL,L"ERROR, have not been able to suspend this Thread!",L"Nanomite",MB_OK);

		CloseHandle(hThread);
	}
	else if(QString().compare(pAction->text(),"Resume") == 0)
	{
		DWORD ThreadID = tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME,false,ThreadID);
		if(hThread == INVALID_HANDLE_VALUE) 
		{
			MessageBoxW(NULL,L"ERROR, have not been able to open this Thread!",L"Nanomite",MB_OK);
			return;
		}

		if(ResumeThread(hThread) != -1)
			tblTIDs->item(_iSelectedRow,4)->setText("Running");
		else
			MessageBoxW(NULL,L"ERROR, have not been able to resume this Thread!",L"Nanomite",MB_OK);

		CloseHandle(hThread);
	}
	else if(QString().compare(pAction->text(),"Highest") == 0)
		clsHelperClass::SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_HIGHEST);
	else if(QString().compare(pAction->text(),"Above Normal") == 0)
		clsHelperClass::SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_ABOVE_NORMAL);
	else if(QString().compare(pAction->text(),"Normal") == 0)
		clsHelperClass::SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_NORMAL);
	else if(QString().compare(pAction->text(),"Below Normal") == 0)
		clsHelperClass::SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_BELOW_NORMAL);
	else if(QString().compare(pAction->text(),"Lowest") == 0)
		clsHelperClass::SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_LOWEST);
}