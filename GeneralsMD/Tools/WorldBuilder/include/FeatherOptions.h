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

// FeatherOptions.h : options panel (Qt6 port)

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class QSpinBox;

class FeatherOptions : public QWidget
{
	Q_OBJECT

public:
	enum {MIN_FEATHER_SIZE=2, MAX_FEATHER_SIZE=51, MIN_RATE=1, MAX_RATE=10, MIN_RADIUS=1, MAX_RADIUS=5};

	FeatherOptions(QWidget *parent = NULL);
	~FeatherOptions() override;

	static void setFeather(Int value);
	static void setRate(Int value);
	static void setRadius(Int value);

protected:
	QSpinBox *m_featherSpin;
	QSpinBox *m_rateSpin;
	QSpinBox *m_radiusSpin;
	Bool m_updating;

	static FeatherOptions *m_staticThis;
};
