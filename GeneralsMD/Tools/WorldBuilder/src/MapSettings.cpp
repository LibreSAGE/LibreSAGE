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

// MapSettings.cpp : the map settings dialog (Qt6 port).  The TimeOfDayNames /
// WeatherNames arrays are defined by the game engine (GlobalData.cpp), so this
// file just uses the externs - it must not define them again.

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>

#include "MapSettings.h"

#include "Common/GlobalData.h"
#include "Common/GameType.h"
#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"
#include "Compression.h"

MapSettings::MapSettings(QWidget *parent) :
	QDialog(parent),
	m_mapTitle(NULL),
	m_timeOfDay(NULL),
	m_weather(NULL),
	m_compression(NULL)
{
	setWindowTitle("Map Settings");
	setModal(true);

	QFormLayout *form = new QFormLayout(this);

	m_mapTitle = new QLineEdit(this);
	form->addRow("Map Title:", m_mapTitle);

	m_timeOfDay = new QComboBox(this);
	for (Int i = TIME_OF_DAY_FIRST; i < TIME_OF_DAY_COUNT; i++) {
		m_timeOfDay->addItem(QString::fromUtf8(TimeOfDayNames[i]));
	}
	form->addRow("Time of Day:", m_timeOfDay);

	m_weather = new QComboBox(this);
	for (Int i = 0; i < WEATHER_COUNT; i++) {
		m_weather->addItem(QString::fromUtf8(WeatherNames[i]));
	}
	form->addRow("Weather:", m_weather);

	m_compression = new QComboBox(this);
	for (Int i = COMPRESSION_MIN; i <= COMPRESSION_MAX; i++) {
		m_compression->addItem(QString::fromUtf8(
			CompressionManager::getCompressionNameByType((CompressionType)i)));
	}
	form->addRow("Compression:", m_compression);

	// Populate from the current global data / world dict.
	m_timeOfDay->setCurrentIndex(TheGlobalData->m_timeOfDay - TIME_OF_DAY_FIRST);
	m_weather->setCurrentIndex(TheGlobalData->m_weather);

	Dict *worldDict = MapObject::getWorldDict();
	Bool exists = false;
	if (worldDict) {
		AsciiString mapName = worldDict->getAsciiString(TheKey_mapName, &exists);
		if (exists) m_mapTitle->setText(QString::fromUtf8(mapName.str()));

		exists = false;
		Int index = worldDict->getInt(TheKey_compression, &exists);
		if (!exists || index < COMPRESSION_MIN || index > COMPRESSION_MAX)
			index = CompressionManager::getPreferredCompression();
		m_compression->setCurrentIndex(index - COMPRESSION_MIN);
	}

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &MapSettings::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &MapSettings::reject);
	form->addRow(buttons);
}

void MapSettings::accept()
{
	TimeOfDay tod = (TimeOfDay)(m_timeOfDay->currentIndex() + TIME_OF_DAY_FIRST);
	Weather theWeather = (Weather)m_weather->currentIndex();

	TheWritableGlobalData->setTimeOfDay(tod);
	TheWritableGlobalData->m_weather = theWeather;

	Dict *worldDict = MapObject::getWorldDict();
	if (worldDict) {
		worldDict->setAsciiString(TheKey_mapName,
			AsciiString(m_mapTitle->text().toUtf8().constData()));
		CompressionType compType = (CompressionType)(m_compression->currentIndex() + COMPRESSION_MIN);
		worldDict->setInt(TheKey_compression, compType);
	}

	QDialog::accept();
}
