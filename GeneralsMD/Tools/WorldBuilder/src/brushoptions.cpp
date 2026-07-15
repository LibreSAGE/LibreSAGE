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

#include <QFormLayout>
#include <QSpinBox>

// brushoptions.cpp : the height-brush options panel (Qt6 port of the
// IDD_BRUSH_OPTIONS dialog: brush width, feather and height).

#include "brushoptions.h"

#include "BrushTool.h"


BrushOptions *BrushOptions::m_staticThis = NULL;

BrushOptions::BrushOptions(QWidget *parent) :
	QWidget(parent),
	m_updating(false)
{
	QFormLayout *layout = new QFormLayout(this);

	m_widthSpin = new QSpinBox(this);
	m_widthSpin->setRange(MIN_BRUSH_SIZE, MAX_BRUSH_SIZE);
	layout->addRow("Brush width:", m_widthSpin);

	m_featherSpin = new QSpinBox(this);
	m_featherSpin->setRange(MIN_FEATHER, MAX_FEATHER);
	layout->addRow("Feather:", m_featherSpin);

	m_heightSpin = new QSpinBox(this);
	m_heightSpin->setRange(MIN_HEIGHT, MAX_HEIGHT);
	layout->addRow("Height:", m_heightSpin);

	connect(m_widthSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) BrushTool::setWidth(value);
	});
	connect(m_featherSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) BrushTool::setFeather(value);
	});
	connect(m_heightSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) BrushTool::setHeight(value);
	});

	m_staticThis = this;

	// Pick up the tool's current values.
	setWidth(BrushTool::getWidth());
	setFeather(BrushTool::getFeather());
	setHeight(BrushTool::getHeight());
}

BrushOptions::~BrushOptions()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

void BrushOptions::setWidth(Int width)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_widthSpin->setValue(width);
		m_staticThis->m_updating = false;
	}
}

void BrushOptions::setFeather(Int feather)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_featherSpin->setValue(feather);
		m_staticThis->m_updating = false;
	}
}

void BrushOptions::setHeight(Int height)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_heightSpin->setValue(height);
		m_staticThis->m_updating = false;
	}
}
