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

// FILE: ImagePackerWindow.cpp ////////////////////////////////////////////////

#include <QAbstractButton>
#include <QCheckBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QImage>
#include <QIntValidator>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QRadioButton>
#include <QStatusBar>

#include "ImagePackerWindow.h"
#include "PreviewWindow.h"
#include "ui_ImagePackerWindow.h"

ImagePackerWindow::ImagePackerWindow( QWidget *parent )
	: QMainWindow( parent ), m_ui( new Ui::ImagePackerWindow() ), m_packer( nullptr ),
		m_preview( nullptr )
{
	m_ui->setupUi( this );

	// create the packing engine and hook ourselves up as its host
	m_packer = new ImagePacker();
	TheImagePacker = m_packer;
	m_packer->setHost( this );
	m_packer->init();

	// numeric input validators
	m_ui->editWidth->setValidator( new QIntValidator( 0, 4096, this ) );
	m_ui->editHeight->setValidator( new QIntValidator( 0, 4096, this ) );
	m_ui->editGutter->setValidator( new QIntValidator( 0, 1024, this ) );

	// default UI state mirrors the defaults stored on the engine
	m_ui->editFilename->setMaxLength( MAX_OUTPUT_FILE_LEN );
	m_ui->editFilename->setText( "NewImage" );
	m_ui->checkAlpha->setChecked( m_packer->getOutputAlpha() );
	m_ui->checkIni->setChecked( m_packer->createINIFile() );
	m_ui->checkSubFolders->setChecked( m_packer->getUseSubFolders() );
	m_ui->checkCompress->setChecked( m_packer->getCompressTextures() );
	m_ui->radio512->setChecked( true );
	m_ui->editGutter->setText( QString::number( m_packer->getGutter() ) );

	UnsignedInt gap = m_packer->getGapMethod();
	m_ui->checkExtendRgb->setChecked( BitTest( gap, ImagePacker::GAP_METHOD_EXTEND_RGB ) );
	m_ui->checkGutter->setChecked( BitTest( gap, ImagePacker::GAP_METHOD_GUTTER ) );

	updateSizeEnabled();
	updateGutterEnabled();

	// wire up the controls
	connect( m_ui->buttonStart, &QPushButton::clicked, this, &ImagePackerWindow::onStart );
	connect( m_ui->buttonExit, &QPushButton::clicked, this, &ImagePackerWindow::onExit );
	connect( m_ui->buttonAddFolder, &QPushButton::clicked, this, &ImagePackerWindow::onAddFolder );
	connect( m_ui->buttonRemoveFolder, &QPushButton::clicked, this, &ImagePackerWindow::onRemoveFolder );
	connect( m_ui->buttonPreview, &QPushButton::clicked, this, &ImagePackerWindow::onPreviewToggle );
	connect( m_ui->buttonPrev, &QPushButton::clicked, this, &ImagePackerWindow::onPrevPage );
	connect( m_ui->buttonNext, &QPushButton::clicked, this, &ImagePackerWindow::onNextPage );
	connect( m_ui->checkShowTexture, &QCheckBox::toggled, this, &ImagePackerWindow::onShowTextureToggled );
	connect( m_ui->radioOther, &QRadioButton::toggled, this, &ImagePackerWindow::onSizeModeChanged );
	connect( m_ui->checkGutter, &QCheckBox::toggled, this, &ImagePackerWindow::onGutterToggled );
	connect( m_ui->editWidth, &QLineEdit::textChanged, this, &ImagePackerWindow::onWidthChanged );

	statusBar()->showMessage( tr( "Select options and click 'Start'." ) );
}

ImagePackerWindow::~ImagePackerWindow()
{
	delete m_packer;
	TheImagePacker = nullptr;
	delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////
// UI helpers /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ImagePackerWindow::updateSizeEnabled()
{
	// the user defined width is only editable when the "Other" radio is on, the
	// height always follows the width (the output must be square)
	bool other = m_ui->radioOther->isChecked();
	m_ui->editWidth->setEnabled( other );
	m_ui->labelX->setEnabled( other );
}

void ImagePackerWindow::updateGutterEnabled()
{
	bool on = m_ui->checkGutter->isChecked();
	m_ui->editGutter->setEnabled( on );
	m_ui->labelGutter->setEnabled( on );
}

void ImagePackerWindow::onSizeModeChanged()
{
	updateSizeEnabled();
}

void ImagePackerWindow::onGutterToggled()
{
	updateGutterEnabled();
}

void ImagePackerWindow::onWidthChanged( const QString &text )
{
	// the output image must be square, mirror the width into the height
	m_ui->editHeight->setText( text );
}

void ImagePackerWindow::onShowTextureToggled( bool checked )
{
	m_packer->setUseTexturePreview( checked );
	updatePreview();
}

void ImagePackerWindow::onExit()
{
	close();
}

void ImagePackerWindow::onAddFolder()
{
	QString dir = QFileDialog::getExistingDirectory( this, tr( "Select Folder" ) );
	if( dir.isEmpty() )
		return;

	// do not add a folder that is already in the list
	if( !m_ui->listFolders->findItems( dir, Qt::MatchFixedString ).isEmpty() )
	{
		QMessageBox::information( this, tr( "Folder Already In List" ),
														 tr( "Ignoring folder '%1', already in list." ).arg( dir ) );
		return;
	}

	m_ui->listFolders->addItem( dir );
}

void ImagePackerWindow::onRemoveFolder()
{
	QList<QListWidgetItem *> selected = m_ui->listFolders->selectedItems();
	if( selected.isEmpty() )
	{
		QMessageBox::information( this, tr( "Select Folder First" ),
														 tr( "You must first select a folder to remove it" ) );
		return;
	}

	qDeleteAll( selected );
}

void ImagePackerWindow::onPreviewToggle()
{
	if( m_preview && m_preview->isVisible() )
	{
		m_preview->close();  // closeEvent() resets the button text
		return;
	}

	if( m_preview == nullptr )
	{
		m_preview = new PreviewWindow( this );
		connect( m_preview, &PreviewWindow::closed, this,
						 [this]() { m_ui->buttonPreview->setText( tr( "Open Preview" ) ); } );
	}

	m_ui->buttonPreview->setText( tr( "Close Preview" ) );
	m_preview->show();
	updatePreview();
}

void ImagePackerWindow::onPrevPage()
{
	Int page = m_packer->getTargetPreviewPage();
	if( page > 1 )
	{
		m_packer->setTargetPreviewPage( page - 1 );
		updatePreview();
	}
}

void ImagePackerWindow::onNextPage()
{
	Int page = m_packer->getTargetPreviewPage();
	if( page < (Int)m_packer->getPageCount() )
	{
		m_packer->setTargetPreviewPage( page + 1 );
		updatePreview();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Preview ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QImage ImagePackerWindow::buildPreviewImage()
{
	int w = m_packer->getTargetWidth();
	int h = m_packer->getTargetHeight();

	QImage image( w, h, QImage::Format_RGB32 );
	image.fill( Qt::black );

	// find the target texture page
	TexturePage *page = nullptr;
	for( TexturePage *p = m_packer->getFirstTexturePage(); p; p = p->m_next )
		if( p->getID() == m_packer->getTargetPreviewPage() )
		{
			page = p;
			break;
		}

	if( page == nullptr )
		return image;

	if( m_packer->getUseTexturePreview() )
	{
		// draw the actual generated texture pixel by pixel
		for( int y = 0; y < h; y++ )
			for( int x = 0; x < w; x++ )
			{
				Byte r = 0, g = 0, b = 0;
				page->getPixel( x, y, &r, &g, &b );
				image.setPixel( x, y, qRgb( (unsigned char)r, (unsigned char)g, (unsigned char)b ) );
			}
	}
	else
	{
		// draw a silhouette of the packed image rectangles
		QPainter painter( &image );
		for( ImageInfo *img = page->getFirstImage(); img; img = img->m_nextPageImage )
		{
			QRect rect( img->m_pagePos.lo.x, img->m_pagePos.lo.y,
									img->m_pagePos.hi.x - img->m_pagePos.lo.x + 1,
									img->m_pagePos.hi.y - img->m_pagePos.lo.y + 1 );
			painter.fillRect( rect, Qt::white );
		}
	}

	return image;
}

void ImagePackerWindow::updatePreview()
{
	if( m_preview == nullptr || !m_preview->isVisible() )
		return;

	QString title = tr( "Page #%1 of %2" )
										.arg( m_packer->getTargetPreviewPage() )
										.arg( m_packer->getPageCount() );
	m_preview->setPreview( buildPreviewImage(), title );
}

///////////////////////////////////////////////////////////////////////////////
// Settings + run /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ImagePackerWindow::gatherSettings()
{
	// target image size, the user defined size must be a power of 2
	if( m_ui->radioOther->isChecked() )
	{
		int size = m_ui->editWidth->text().toInt();

		int bitCount = 0;
		for( unsigned int v = (unsigned int)size; v; v >>= 1 )
			if( v & 0x1 )
				bitCount++;

		if( bitCount != 1 )
		{
			QMessageBox::critical( this, tr( "Must Be Power Of 2" ),
														 tr( "The target image size must be a power of 2." ) );
			return false;
		}

		m_packer->setTargetSize( size, size );
	}
	else if( m_ui->radio128->isChecked() )
		m_packer->setTargetSize( 128, 128 );
	else if( m_ui->radio256->isChecked() )
		m_packer->setTargetSize( 256, 256 );
	else if( m_ui->radio512->isChecked() )
		m_packer->setTargetSize( 512, 512 );
	else
	{
		QMessageBox::critical( this, tr( "Error" ), tr( "Internal Error. Target Size Unknown." ) );
		return false;
	}

	// simple options
	m_packer->setOutputAlpha( m_ui->checkAlpha->isChecked() );
	m_packer->setINICreate( m_ui->checkIni->isChecked() );
	m_packer->setUseTexturePreview( m_ui->checkShowTexture->isChecked() );
	m_packer->setCompressTextures( m_ui->checkCompress->isChecked() );

	// gap method options
	m_packer->clearGapMethod( ImagePacker::GAP_METHOD_EXTEND_RGB );
	if( m_ui->checkExtendRgb->isChecked() )
		m_packer->setGapMethod( ImagePacker::GAP_METHOD_EXTEND_RGB );
	m_packer->clearGapMethod( ImagePacker::GAP_METHOD_GUTTER );
	if( m_ui->checkGutter->isChecked() )
		m_packer->setGapMethod( ImagePacker::GAP_METHOD_GUTTER );

	// gutter size
	int gutter = m_ui->editGutter->text().toInt();
	if( gutter < 0 )
		gutter = 0;
	m_packer->setGutter( gutter );

	// output filename, reject illegal characters
	QString outName = m_ui->editFilename->text();
	const QString illegal = "/\\:*?<>|";
	for( QChar c : outName )
		if( illegal.contains( c ) )
		{
			QMessageBox::critical( this, tr( "Illegal Filename" ),
														 tr( "Output filename '%1' contains one or more of the "
																 "following illegal characters:\n\n%2" )
															 .arg( outName, illegal ) );
			return false;
		}
	m_packer->setOutputFileName( outName.toUtf8().constData() );

	// work on sub-folders option
	bool useSub = m_ui->checkSubFolders->isChecked();
	m_packer->setUseSubFolders( useSub );

	// add all the image directories specified in the folder list
	m_packer->clearDirectories();
	statusBar()->showMessage( tr( "Gathering Directory Information, Please Wait ..." ) );
	for( int i = 0; i < m_ui->listFolders->count(); i++ )
	{
		QString path = m_ui->listFolders->item( i )->text();
		m_packer->addSourceDirectory( path.toUtf8().constData(), useSub );
	}

	return true;
}

void ImagePackerWindow::onStart()
{
	if( !gatherSettings() )
		return;

	m_packer->process();
}

///////////////////////////////////////////////////////////////////////////////
// ImagePackerHost interface //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ImagePackerWindow::onStatus( const char *message )
{
	statusBar()->showMessage( QString::fromUtf8( message ) );

	// keep the UI responsive while the (synchronous) packing process runs
	QCoreApplication::processEvents();
}

void ImagePackerWindow::onError( const char *title, const char *message )
{
	QMessageBox::critical( this, QString::fromUtf8( title ), QString::fromUtf8( message ) );
}

Bool ImagePackerWindow::confirmDeleteFiles( const char *directory, Int count )
{
	QString text = tr( "The output directory (%1) must be empty before proceeding.  "
										 "Delete '%2' files and continue with build process?" )
									 .arg( QString::fromUtf8( directory ) )
									 .arg( count );

	QMessageBox::StandardButton response =
		QMessageBox::warning( this, tr( "Delete files to continue?" ), text,
													QMessageBox::Yes | QMessageBox::No );

	return response == QMessageBox::Yes;
}

Bool ImagePackerWindow::confirmImageErrors( ImagePacker *packer )
{
	QStringList lines;
	UnsignedInt count = packer->getImageCount();
	for( UnsignedInt i = 0; i < count; i++ )
	{
		ImageInfo *image = packer->getImage( i );
		if( image == nullptr )
			continue;

		if( BitTest( image->m_status, ImageInfo::CANTPROCESS ) )
		{
			QString reason;
			if( BitTest( image->m_status, ImageInfo::TOOBIG ) )
				reason = tr( "Too Big" );
			else if( BitTest( image->m_status, ImageInfo::INVALIDCOLORDEPTH ) )
				reason = tr( "Unsupported Color Depth" );
			else
				reason = tr( "Unknown Reason" );

			lines << QString( "%1: (%2x%3x%4) %5" )
								 .arg( reason )
								 .arg( image->m_size.x )
								 .arg( image->m_size.y )
								 .arg( image->m_colorDepth )
								 .arg( QString::fromUtf8( image->m_path ) );
		}
	}

	QMessageBox box( this );
	box.setIcon( QMessageBox::Warning );
	box.setWindowTitle( tr( "Processing Errors" ) );
	box.setText( tr( "The following images cannot be processed.  Proceed anyway?" ) );
	box.setDetailedText( lines.join( "\n" ) );
	box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
	box.button( QMessageBox::Yes )->setText( tr( "Proceed Anyway" ) );
	box.button( QMessageBox::No )->setText( tr( "Cancel Build" ) );

	return box.exec() == QMessageBox::Yes;
}

void ImagePackerWindow::reportPageErrors( ImagePacker *packer )
{
	QStringList lines;
	for( TexturePage *page = packer->getFirstTexturePage(); page; page = page->m_next )
	{
		if( BitTest( page->m_status, TexturePage::PAGE_ERROR ) )
		{
			QString reason;
			if( BitTest( page->m_status, TexturePage::CANT_ALLOCATE_PACKED_IMAGE ) )
				reason = tr( "Can't allocate image memory" );
			else if( BitTest( page->m_status, TexturePage::CANT_ADD_IMAGE_DATA ) )
				reason = tr( "Can't add image(s) data" );
			else if( BitTest( page->m_status, TexturePage::NO_TEXTURE_DATA ) )
				reason = tr( "No texture data to write" );
			else if( BitTest( page->m_status, TexturePage::ERROR_DURING_SAVE ) )
				reason = tr( "Error writing texture file" );
			else
				reason = tr( "Unknown Reason" );

			lines << QString( "%1: (%2x%3) %4%5" )
								 .arg( reason )
								 .arg( page->getWidth() )
								 .arg( page->getHeight() )
								 .arg( QString::fromUtf8( packer->getOutputFile() ) )
								 .arg( page->getID() );
		}
	}

	QMessageBox box( this );
	box.setIcon( QMessageBox::Warning );
	box.setWindowTitle( tr( "Texture Page Errors" ) );
	box.setText( tr( "The following texture pages could not be generated and were ignored" ) );
	box.setDetailedText( lines.join( "\n" ) );
	box.exec();
}

void ImagePackerWindow::onProcessComplete( ImagePacker *packer )
{
	Q_UNUSED( packer );
	updatePreview();
}
