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

// FILE: PreviewWindow.cpp ////////////////////////////////////////////////////

#include "PreviewWindow.h"

#include <QCloseEvent>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

PreviewWindow::PreviewWindow( QWidget *parent )
	: QWidget( parent, Qt::Window )
{
	m_label = new QLabel( this );
	m_label->setAlignment( Qt::AlignCenter );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( m_label );

	setWindowTitle( tr( "Preview" ) );
}

void PreviewWindow::setPreview( const QImage &image, const QString &title )
{
	m_label->setPixmap( QPixmap::fromImage( image ) );
	setWindowTitle( title );
	adjustSize();
}

void PreviewWindow::closeEvent( QCloseEvent *event )
{
	emit closed();
	QWidget::closeEvent( event );
}
