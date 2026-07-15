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

// FeatherOptions.cpp : options panel (Qt6 port of the MFC dialog)

#include <QFormLayout>
#include <QSpinBox>

#include "FeatherOptions.h"

#include "FeatherTool.h"

FeatherOptions *FeatherOptions::m_staticThis = NULL;

FeatherOptions::FeatherOptions(QWidget *parent) :
	QWidget(parent),
	m_updating(false)
{
	QFormLayout *layout = new QFormLayout(this);

	m_featherSpin = new QSpinBox(this);
	m_featherSpin->setRange(MIN_FEATHER_SIZE, MAX_FEATHER_SIZE);
	layout->addRow("Feather size:", m_featherSpin);
	connect(m_featherSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) FeatherTool::setFeather(value);
	});

	m_rateSpin = new QSpinBox(this);
	m_rateSpin->setRange(MIN_RATE, MAX_RATE);
	layout->addRow("Rate:", m_rateSpin);
	connect(m_rateSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) FeatherTool::setRate(value);
	});

	m_radiusSpin = new QSpinBox(this);
	m_radiusSpin->setRange(MIN_RADIUS, MAX_RADIUS);
	layout->addRow("Radius:", m_radiusSpin);
	connect(m_radiusSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) FeatherTool::setRadius(value);
	});

	m_staticThis = this;
	setFeather(FeatherTool::getFeather());
	setRate(FeatherTool::getRate());
	setRadius(FeatherTool::getRadius());
}

FeatherOptions::~FeatherOptions()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

void FeatherOptions::setFeather(Int value)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_featherSpin->setValue(value);
		m_staticThis->m_updating = false;
	}
}

void FeatherOptions::setRate(Int value)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_rateSpin->setValue(value);
		m_staticThis->m_updating = false;
	}
}

void FeatherOptions::setRadius(Int value)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_radiusSpin->setValue(value);
		m_staticThis->m_updating = false;
	}
}

