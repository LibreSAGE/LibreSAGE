/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**  Copyright 2026 Stephan Vedder
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// addplayerdialog.h : the "add a player to the map" dialog (Qt6 port)
//

#pragma once

#include <QDialog>

#include "Common/AsciiString.h"

class QComboBox;

/////////////////////////////////////////////////////////////////////////////
/// AddPlayerDialog - pick a player template (optionally filtered to a side)
/// and add it to the map's sides list.

class AddPlayerDialog : public QDialog
{
	Q_OBJECT

public:
	AddPlayerDialog(AsciiString side, QWidget *parent = NULL);

	AsciiString getAddedSide() const { return m_addedSide; }

protected:
	void accept() override;
	void reject() override;

private:
	AsciiString m_side;
	AsciiString m_addedSide;
	QComboBox  *m_factions;
};
