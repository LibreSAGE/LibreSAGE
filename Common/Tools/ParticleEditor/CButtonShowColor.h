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

// FILE: CButtonShowColor.h /////////////////////////////////////////////////////
//
// Desc:       A push button that paints itself with a solid colour swatch.  Used
//             by the colour keyframe editor to show the colour of each keyframe.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QPushButton>

#include "Lib/BaseType.h"

// BaseType.h defines min/max as macros which collide with Qt/STL templates.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class CButtonShowColor : public QPushButton
{
	Q_OBJECT

public:
	explicit CButtonShowColor( QWidget *parent = nullptr );

	const RGBColor& getColor( void ) const { return m_color; }
	void setColor( Int color ) { m_color.setFromInt( color ); update(); }
	void setColor( const RGBColor& color ) { m_color = color; update(); }

protected:
	void paintEvent( QPaintEvent *event ) override;

private:
	RGBColor m_color = { 0.0f, 0.0f, 0.0f };
};
