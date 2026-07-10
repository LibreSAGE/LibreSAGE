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

// FILE: ImagePackerWindow.h //////////////////////////////////////////////////
//
// Project:    ImagePacker
//
// Desc:       Qt main window for the image packer tool.  This is the host
//             that drives the platform independent ImagePacker engine.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMainWindow>

#include "ImagePacker.h"

// BaseType.h (pulled in by ImagePacker.h) defines min/max as macros which
// collide with Qt/STL templates used downstream; get rid of them here.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class QImage;
class PreviewWindow;

namespace Ui
{
	class ImagePackerWindow;
}

class ImagePackerWindow : public QMainWindow, public ImagePackerHost
{
	Q_OBJECT

public:

	explicit ImagePackerWindow( QWidget *parent = nullptr );
	~ImagePackerWindow() override;

	// ImagePackerHost interface ------------------------------------------------
	void onStatus( const char *message ) override;
	void onError( const char *title, const char *message ) override;
	Bool confirmImageErrors( ImagePacker *packer ) override;
	Bool confirmDeleteFiles( const char *directory, Int count ) override;
	void reportPageErrors( ImagePacker *packer ) override;
	void onProcessComplete( ImagePacker *packer ) override;

private slots:

	void onStart();
	void onExit();
	void onAddFolder();
	void onRemoveFolder();
	void onPreviewToggle();
	void onPrevPage();
	void onNextPage();
	void onShowTextureToggled( bool checked );
	void onSizeModeChanged();
	void onGutterToggled();
	void onWidthChanged( const QString &text );

private:

	bool gatherSettings();
	void updateSizeEnabled();
	void updateGutterEnabled();
	void updatePreview();
	QImage buildPreviewImage();

	Ui::ImagePackerWindow *m_ui;
	ImagePacker *m_packer;
	PreviewWindow *m_preview;
};
