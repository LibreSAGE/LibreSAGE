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

// MoundOptions.cpp : options panel (Qt6 port of the MFC dialog)

#include <QFormLayout>
#include <QSpinBox>

#include "MoundOptions.h"

#include "MoundTool.h"

MoundOptions *MoundOptions::m_staticThis = NULL;

MoundOptions::MoundOptions(QWidget *parent) :
	QWidget(parent),
	m_updating(false)
{
	QFormLayout *layout = new QFormLayout(this);

	m_widthSpin = new QSpinBox(this);
	m_widthSpin->setRange(MIN_BRUSH_SIZE, MAX_BRUSH_SIZE);
	layout->addRow("Brush width:", m_widthSpin);
	connect(m_widthSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) MoundTool::setWidth(value);
	});

	m_featherSpin = new QSpinBox(this);
	m_featherSpin->setRange(MIN_FEATHER, MAX_FEATHER);
	layout->addRow("Feather:", m_featherSpin);
	connect(m_featherSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) MoundTool::setFeather(value);
	});

	m_heightSpin = new QSpinBox(this);
	m_heightSpin->setRange(MIN_HEIGHT, MAX_HEIGHT);
	layout->addRow("Mound height:", m_heightSpin);
	connect(m_heightSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) MoundTool::setMoundHeight(value);
	});

	m_staticThis = this;
	setWidth(MoundTool::getWidth());
	setFeather(MoundTool::getFeather());
	setHeight(MoundTool::getMoundHeight());
}

MoundOptions::~MoundOptions()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

void MoundOptions::setWidth(Int value)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_widthSpin->setValue(value);
		m_staticThis->m_updating = false;
	}
}

void MoundOptions::setFeather(Int value)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_featherSpin->setValue(value);
		m_staticThis->m_updating = false;
	}
}

void MoundOptions::setHeight(Int value)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_heightSpin->setValue(value);
		m_staticThis->m_updating = false;
	}
}

