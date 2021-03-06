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
#include "qtDLGTrace.h"

#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <QWheelEvent>

using namespace std;

qtDLGTrace *qtDLGTrace::pThis = NULL;

qtDLGTrace::qtDLGTrace(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(horizontalLayout);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	pThis = this;

	tblTraceLog->horizontalHeader()->resizeSection(0,80); //PID
	tblTraceLog->horizontalHeader()->resizeSection(1,80); //TID
	tblTraceLog->horizontalHeader()->resizeSection(2,135); //OFFSET
	tblTraceLog->horizontalHeader()->resizeSection(3,300); //INST.
	//tblTraceLog->horizontalHeader()->resizeSection(4,300); //REG

	connect(tblTraceLog,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	//connect(scrollTrace,SIGNAL(valueChanged(int)),this,SLOT(OnShow(int)));
}

qtDLGTrace::~qtDLGTrace()
{
	pThis = NULL;
}

void qtDLGTrace::addTraceData(DWORD64 dwOffset,DWORD PID,DWORD TID)
{
	if(pThis == NULL) return;

	TraceInfoRow newTraceRow;
	newTraceRow.PID = PID;
	newTraceRow.TID = TID;
	newTraceRow.asmInstruction = QString("");
	newTraceRow.cpuReg = QString("");
	newTraceRow.dwOffset = dwOffset;

	pThis->traceData.insert(dwOffset,newTraceRow);
}

void qtDLGTrace::clearTraceData()
{
	if(pThis == NULL) return;

	pThis->traceData.clear();
	pThis->scrollTrace->setValue(0);
	pThis->scrollTrace->setMaximum(0);
}

void qtDLGTrace::showEvent(QShowEvent * event)
{
	OnShow(0);
}

void qtDLGTrace::OnShow(int delta)
{
	int iLines = NULL,
		iPossibleRowCount = ((tblTraceLog->verticalHeader()->height() + 4) / 12) - 1;
	QMap<DWORD64,TraceInfoRow>::iterator i;

	scrollTrace->setMaximum(traceData.count());

	if(delta < 0 && tblTraceLog->rowCount() > 0)
	{
		i = traceData.find(tblTraceLog->item(0,2)->text().toULongLong(0,16));
		--i;
		scrollTrace->setValue(scrollTrace->value() - 1);
	}
	else if(delta > 0 && tblTraceLog->rowCount() > 1)
	{
		i = traceData.find(tblTraceLog->item(1,2)->text().toULongLong(0,16));
		scrollTrace->setValue(scrollTrace->value() + 1);
	}
	else if(delta == 0)
	{
		i = traceData.begin();
		scrollTrace->setValue(0);
	}
	else
		return;

	if((QMapData::Node *)i == (QMapData::Node *)traceData.end())
		return;

	tblTraceLog->setRowCount(0);
	while(iLines <= iPossibleRowCount)
	{
		if((QMapData::Node *)i == (QMapData::Node *)traceData.end())
			return;

		tblTraceLog->insertRow(tblTraceLog->rowCount());
		
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(i.value().PID,8,16,QChar('0'))));
		
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(i.value().TID,8,16,QChar('0'))));
	
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(i.value().dwOffset,16,16,QChar('0'))));

		if(i.value().asmInstruction.length() <= 0)
		{
			std::wstring *FuncName = new wstring(L""),*ModName = new wstring(L"");
			clsHelperClass::LoadSymbolForAddr(*FuncName,*ModName,i.value().dwOffset,OpenProcess(PROCESS_ALL_ACCESS,false,i.value().PID));

			QString funcName = QString().fromStdWString(*FuncName);
			QString modName = QString().fromStdWString(*FuncName);
			i.value().asmInstruction.append(modName).append(".").append(funcName);
		}
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,3,
			new QTableWidgetItem(i.value().asmInstruction));

		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,4,
			new QTableWidgetItem(i.value().cpuReg));

		++i;iLines++;
	}
}

void qtDLGTrace::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;
		
	OnShow(pWheel->delta() * -1);
}

void qtDLGTrace::resizeEvent(QResizeEvent *event)
{
	OnShow(0);
}

void qtDLGTrace::OnCustomContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblTraceLog->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGTrace::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
		emit OnDisplayDisassembly(tblTraceLog->item(_iSelectedRow,2)->text().toULongLong(0,16));
}