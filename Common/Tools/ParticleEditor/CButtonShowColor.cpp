/*
**	Command & Conquer Generals(tm)
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

#include "CButtonShowColor.h"

#include <QPainter>

CButtonShowColor::CButtonShowColor( QWidget *parent )
	: QPushButton( parent )
{
}

void CButtonShowColor::paintEvent( QPaintEvent * /*event*/ )
{
	QPainter painter( this );

	const QColor swatch(
		static_cast<int>( m_color.red   * 255.0f ),
		static_cast<int>( m_color.green * 255.0f ),
		static_cast<int>( m_color.blue  * 255.0f ) );

	QRect r = rect().adjusted( 0, 0, -1, -1 );
	painter.fillRect( r, swatch );
	painter.setPen( QColor( 0, 0, 0 ) );
	painter.drawRect( r );
}
