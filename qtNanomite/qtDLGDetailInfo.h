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
#ifndef QTDLGDETINFO_H
#define QTDLGDETINFO_H

#include "ui_qtDLGDetailInfo.h"

class qtDLGDetailInfo : public QWidget, public Ui_qtDLGDetailInfoClass
{
	Q_OBJECT

public:
	qtDLGDetailInfo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGDetailInfo();

signals:
	void ShowInDisassembler(quint64 Offset);
	void OpenFileInPEManager(std::wstring FileName,int PID);

private:
	int _iSelectedRow;
	int _iSelectedTable;

	quint64 _SelectedOffset;

private slots:
	void MenuCallback(QAction* pAction);
	void OnCustomTIDContextMenu(QPoint qPoint);
	void OnCustomPIDContextMenu(QPoint qPoint);
	void OnCustomExceptionContextMenu(QPoint qPoint);
	void OnCustomModuleContextMenu(QPoint qPoint);

};

#endif
