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
#ifndef QTDLGWINDOWVIEW_H
#define QTDLGWINDOWVIEW_H

#include "ui_qtDLGWindowView.h"

#include "qtDLGNanomite.h"

class qtDLGWindowView : public QWidget, public Ui_qtDLGWindowViewClass
{
	Q_OBJECT

public:
	static qtDLGWindowView *pThis;
	
	qtDLGWindowView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGWindowView();

private:
	qint32 _iPID;
	int _iForEntry;
	int _iForEnd;

	qtDLGNanomite *myMainWindow;

	private slots:
		void EnumWindow();
};
#endif