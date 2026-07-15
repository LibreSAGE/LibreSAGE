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

#include <QApplication>
#include <QFont>
#include <QPixmap>

// SplashScreen.cpp : Qt6 port of the MFC loading splash.  The original was
// the IDD_LOADING popup dialog showing the IDB_WORLDBUILDERSPLASH bitmap with
// a text output area near the bottom.

#include "SplashScreen.h"


//-------------------------------------------------------------------------------------------------
SplashScreen::SplashScreen() :
	QSplashScreen(QPixmap(":/WorldBuilder.bmp"))
{
	setWindowTitle("Loading Worldbuilder");

	QFont splashFont("Arial");
	splashFont.setPixelSize(12);
	setFont(splashFont);
}

//-------------------------------------------------------------------------------------------------
void SplashScreen::outputText(const QString &text)
{
	showMessage(text, Qt::AlignBottom | Qt::AlignLeft, Qt::black);
	// The splash lives through the (blocking) engine bring-up, so pump the
	// event loop to get the message painted.
	QApplication::processEvents();
}
