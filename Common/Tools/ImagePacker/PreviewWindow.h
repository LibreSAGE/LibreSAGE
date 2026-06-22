/*
**	Command & Conquer Generals(tm)
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

// FILE: PreviewWindow.h //////////////////////////////////////////////////////
//
// Project:    ImagePacker
//
// Desc:       Small top level window that displays a packed texture page.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>

class QLabel;

class PreviewWindow : public QWidget
{
	Q_OBJECT

public:

	explicit PreviewWindow( QWidget *parent = nullptr );

	void setPreview( const QImage &image, const QString &title );

signals:

	void closed();

protected:

	void closeEvent( QCloseEvent *event ) override;

private:

	QLabel *m_label;
};
