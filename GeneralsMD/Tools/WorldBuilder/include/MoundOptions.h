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

// MoundOptions.h : options panel (Qt6 port)

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class QSpinBox;

class MoundOptions : public QWidget
{
	Q_OBJECT

public:
	enum {MIN_BRUSH_SIZE=1, MAX_BRUSH_SIZE=51, MIN_FEATHER=0, MAX_FEATHER=20, MIN_HEIGHT=1, MAX_HEIGHT=64};

	MoundOptions(QWidget *parent = NULL);
	~MoundOptions() override;

	static void setWidth(Int value);
	static void setFeather(Int value);
	static void setHeight(Int value);

protected:
	QSpinBox *m_widthSpin;
	QSpinBox *m_featherSpin;
	QSpinBox *m_heightSpin;
	Bool m_updating;

	static MoundOptions *m_staticThis;
};
