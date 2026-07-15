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

// ObjectPreview.h : a small W3D-rendered thumbnail of a thing template (Qt6
// port).  Like the MFC original it borrows the single shared WW3D device,
// renders the model into an offscreen render-target texture, reads the pixels
// back to a QImage, and paints that.
//

#pragma once

#include <QImage>
#include <QWidget>

#include "Lib/BaseType.h"

class ThingTemplate;

class ObjectPreview : public QWidget
{
	Q_OBJECT

public:
	ObjectPreview(QWidget *parent = NULL);
	~ObjectPreview() override;

	/// Set the template to preview (NULL clears it).  Regenerates the thumbnail.
	void setThingTemplate(const ThingTemplate *tTempl);

	QSize sizeHint(void) const override;

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QImage renderTemplate(const ThingTemplate *tt);

	const ThingTemplate *m_tTempl;
	QImage				 m_image;	///< cached thumbnail for the current template.
};
