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

// FILE: ImagePacker.cpp //////////////////////////////////////////////////////
//
// Project:    ImagePacker
//
// Created:    Colin Day, August 2001
//
// Desc:       Entry point for the image packer.  This program takes
//						 separate image files and combines them into a single
//						 image as close as possible so that we can conserve texture
//						 memory
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <string>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QString>

#include "ImagePacker.h"

// In the original tool this was a full-fledged assert/crash macro provided by
// the game engine.  For the stand-alone Qt tool a plain assert is sufficient.
#define DEBUG_ASSERTCRASH( cond, msg ) assert( cond )

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ImagePacker *TheImagePacker = NULL;

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// hasTgaExtension ============================================================
/** Does this filename end in a (case insensitive) .tga extension? */
//=============================================================================
static Bool hasTgaExtension( const std::string &name )
{
	size_t len = name.size();
	if( len <= 4 )
		return FALSE;

	return name[ len - 4 ] == '.' &&
				 (name[ len - 3 ] == 't' || name[ len - 3 ] == 'T') &&
				 (name[ len - 2 ] == 'g' || name[ len - 2 ] == 'G') &&
				 (name[ len - 1 ] == 'a' || name[ len - 1 ] == 'A');

}  // end hasTgaExtension

// ensureTrailingSlash ========================================================
/** Make sure a directory string ends with a forward slash, the file name
	* construction code below assumes that */
//=============================================================================
static std::string ensureTrailingSlash( const std::string &path )
{
	if( path.empty() )
		return path;

	char last = path[ path.size() - 1 ];
	if( last == '/' || last == '\\' )
		return path;

	return path + "/";

}  // end ensureTrailingSlash

// ImagePacker::createNewTexturePage ==========================================
/** Create a new texture page and add to the list */
//=============================================================================
TexturePage *ImagePacker::createNewTexturePage( void )
{
	TexturePage *page;

	// allocate new page
	page = new TexturePage( getTargetWidth(), getTargetHeight() );

	// link page to list
	page->m_prev = NULL;
	page->m_next = m_pageList;
	if( m_pageList )
		m_pageList->m_prev = page;
	m_pageList = page;

	// add the tail pointer if this is the first page
	if( m_pageTail == NULL )
		m_pageTail = page;

	// we got a new page now
	m_pageCount++;

	// set page id as the current page count
	page->setID( m_pageCount );

	return page;

}  // end createNewTexturePage

// ImagePacker::validateImages ================================================
/** Check all the images in the image list, if any of them cannot be
	* processed we will flag them as so.  If we have some images that can't
	* be processed, we will warn the user of these images and ask them
	* whether or not to proceed.
	*
	* Returns TRUE to proceed
	* Returns FALSE to cancel build
	*/
//=============================================================================
Bool ImagePacker::validateImages( void )
{
	UnsignedInt i;
	ImageInfo *image;
	Bool errors = FALSE;
	Bool proceed = TRUE;

	// loop through all images
	for( i = 0; i < m_imageCount; i++ )
	{

		// get this image
		image = m_imageList[ i ];

		// sanity
		if( image == NULL )
		{

			DEBUG_ASSERTCRASH( image, ("Image in imagelist is NULL") );
			continue;  // should never happen

		}  // end if

		//
		// if this image is too big to fit in the target page size as a whole
		// then there is nothing we can do about it
		//
		if( image->m_size.x > getTargetWidth() ||
				image->m_size.y > getTargetHeight() )
		{

			errors = TRUE;
			BitSet( image->m_status, ImageInfo::TOOBIG );
			BitSet( image->m_status, ImageInfo::CANTPROCESS );

		}  // end if

		//
		// if this image is not the right format we can't process it, at
		// present we only understand 32 and 24 bit images
		//
		if( image->m_colorDepth != 32 && image->m_colorDepth != 24 )
		{

			errors = TRUE;
			BitSet( image->m_status, ImageInfo::INVALIDCOLORDEPTH );
			BitSet( image->m_status, ImageInfo::CANTPROCESS );

		}  // end if

	}  // end for i

	//
	// if we have errors, ask the user (through the host) whether to proceed
	//
	if( errors == TRUE && m_host )
		proceed = m_host->confirmImageErrors( this );

	return proceed;

}  // end validateImages

// ImagePacker::packImages ====================================================
/** Pack all the images in the image list, starting from the top and
	* working from there */
//=============================================================================
Bool ImagePacker::packImages( void )
{
	UnsignedInt i;
	TexturePage *page = NULL;
	ImageInfo *image = NULL;

	//
	// first sanity check all images loaded, if there are images that cannot
	// be processed the user will be given a list of these and asked wether
	// or not to proceed
	//
	Bool proceed;
	proceed = validateImages();
	if( proceed == FALSE )
	{

		statusMessage( "Build Cancelled By User." );
		return FALSE;

	}  // end if

	// loop through all images
	for( i = 0; i < m_imageCount; i++ )
	{

		// update status
		snprintf( m_statusBuffer, sizeof(m_statusBuffer), "Fitting Image %d of %d.", i, m_imageCount );
		statusMessage( m_statusBuffer );

		// get this image out of the list
		image = m_imageList[ i ];

		// ignore images that we cannot process
		if( BitTest( image->m_status, ImageInfo::CANTPROCESS) )
			continue;

		// try to put image on each page
		for( page = m_pageTail; page; page = page->m_prev )
		{

			if( page->addImage( image ) == TRUE )
				break;  // page added, stop trying to add into pages

		}  // end for page

		// if image was not able to go on any existing page create a new page for it
		if( page == NULL )
		{

			page = createNewTexturePage();
			if( page == NULL )
				return FALSE;

			// try to add the image to this page
			if( page->addImage( image ) == FALSE )
			{
				char buffer[ _MAX_PATH ];

				snprintf( buffer, sizeof(buffer), "Unable to add image '%s' to a brand new page!\n", image->m_path );
				DEBUG_ASSERTCRASH( 0, (buffer) );
				reportError( "Internal Error", buffer );
				return FALSE;

			}  // end if

		}  // end if

	}  // end for i

	return TRUE;  // success

}  // end packImages

// ImagePacker::writeFinalTextures ============================================
/** Generate and write the final textures to the output directory
	* of the packed images along with a definition file for which images
	* are where on the page */
//=============================================================================
void ImagePacker::writeFinalTextures( void )
{
	TexturePage *page;
	Bool errors = FALSE;
	char buffer[ 128 ];

	//
	// go through each page, let's start from the end of the list since
	// that's where we packed first, but it doesn't matter
	//
	for( page = m_pageTail; page; page = page->m_prev )
	{

		// update status message
		snprintf( buffer, sizeof(buffer), "Generating texture #%d of %d.",
						 page->getID(), m_pageCount );
		statusMessage( buffer );

		// generate the final texture for this page
		if( page->generateTexture() == FALSE )
		{

			errors = TRUE;
			continue;  // could not generate this page, but try to continue

		}  // end if

		//
		// write this page out to a file using the filename given by
		// the user and the texture page ID to keep it unique
		//
		if( page->writeFile( m_outputFile ) == FALSE )
		{

			errors = TRUE;
			continue;  // could not write page, but try to go on

		}  // end if

	}  // end for page

	// check for any errors and notify the user
	if( errors == TRUE && m_host )
		m_host->reportPageErrors( this );

}  // end writeFinalTextures

// sortImageCompare ===========================================================
/** Compare function for qsort
	* -1 item1 less than item2
	*	0 item1 identical to item2
	*	1 item1 greater than item2
	*/
//=============================================================================
static Int sortImageCompare( const void *aa, const void *bb )
{
	const ImageInfo **a = (const ImageInfo **)aa;
	const ImageInfo **b = (const ImageInfo **)bb;

	if( (*a)->m_area < (*b)->m_area )
		return 1;
	else if( (*a)->m_area > (*b)->m_area )
		return -1;
	else
		return 0;

}  // end sortImageCompare

// ImagePacker::sortImageList =================================================
/** Sort the image list */
//=============================================================================
void ImagePacker::sortImageList( void )
{

	// sort all images so that largest area ones are first
	qsort( (void *)m_imageList, m_imageCount, sizeof( ImageInfo *), sortImageCompare );

}  // end sortImageList

// ImagePacker::addImagesInDirectory ==========================================
/** Add all the images in the specified directory */
//=============================================================================
void ImagePacker::addImagesInDirectory( const char *dir )
{

	// sanity
	if( dir == NULL )
		return;

	QDir directory( QString::fromUtf8( dir ) );
	const QFileInfoList entries = directory.entryInfoList( QDir::Files );
	for( const QFileInfo &entry : entries )
	{

		std::string name = entry.fileName().toStdString();
		if( hasTgaExtension( name ) )
		{
			std::string filePath = std::string( dir ) + name;
			addImage( filePath.c_str() );

		}  // end if

	}  // end for

}  // end addImagesInDirectory

// ImagePacker::checkOutputDirectory ==========================================
/** Verify that there are no files in the output directory ... if there
	* are give the user the option to delete them, cancel the operation,
	* or proceed with possibly overwriting any files there
	*
	* Returns TRUE to proceed with the process, FALSE if the user wants
	* to cancel the process
	*/
//=============================================================================
Bool ImagePacker::checkOutputDirectory( void )
{
	Int fileCount = 0;

	// create the output directory if it does not exist
	QDir().mkpath( QString::fromUtf8( m_outputDirectory ) );

	QDir outputDir( QString::fromUtf8( m_outputDirectory ) );

	// count the files (not directories) in the output directory
	fileCount = outputDir.entryInfoList( QDir::Files ).size();

	if( fileCount != 0 )
	{
		Bool proceed = TRUE;

		// ask the user (through the host) whether to delete the files and continue
		if( m_host )
			proceed = m_host->confirmDeleteFiles( m_outputDirectory, fileCount );

		// if they said no, do not delete the files and abort the pack process
		if( proceed == FALSE )
			return FALSE;

		//
		// they said yes, delete all the files in the output directory
		//
		const QFileInfoList files = outputDir.entryInfoList( QDir::Files );
		for( const QFileInfo &entry : files )
			QFile::remove( entry.absoluteFilePath() );

	}  // end if

	return TRUE;  // proceed

}  // end checkOutputDirectory

// ImagePacker::resetPageList =================================================
/** Clear the page list */
//=============================================================================
void ImagePacker::resetPageList( void )
{
	TexturePage *next;

	while( m_pageList )
	{

		next = m_pageList->m_next;
		delete m_pageList;
		m_pageList = next;

	}  // end while

	m_pageTail = NULL;
	m_pageCount = 0;
	m_targetPreviewPage = 1;

}  // end resetPageList

// ImagePacker::resetImageDirectoryList =======================================
/** Clear the image directory list */
//=============================================================================
void ImagePacker::resetImageDirectoryList( void )
{
	ImageDirectory *next;

	while( m_dirList )
	{

		next = m_dirList->m_next;
		delete m_dirList;
		m_dirList = next;

	}  // end while

	m_dirCount = 0;
	m_imagesInDirs = 0;

}  // end resetImageDirectoryList

// ImagePacker::resetImageList ================================================
/** Clear the image list */
//=============================================================================
void ImagePacker::resetImageList( void )
{

	if( m_imageList )
	{

		for( UnsignedInt i = 0; i < m_imageCount; i++ )
			delete m_imageList[ i ];
		delete [] m_imageList;

	}  // end if
	m_imageList = NULL;
	m_imageCount = 0;

}  // end resetImageList

// ImagePacker::addDirectory ==================================================
/** Add the directory to the directory list, do not add it if it is already
	* in the directory list.  We want to have that sanity check so that
	* we can be assured that each image being added to the image list from
	* each directory will be unique and we therefore don't have to do
	* any further checking for duplicates */
//=============================================================================
void ImagePacker::addDirectory( const char *path, Bool subDirs )
{

	// santiy
	if( path == NULL )
		return;

	// always work with a trailing slash
	std::string dirPath = ensureTrailingSlash( path );

	// check to see if path is already in list
	ImageDirectory *dir;
	for( dir = m_dirList; dir; dir = dir->m_next )
		if( QString::fromUtf8( dir->m_path ).compare(
					QString::fromUtf8( dirPath.c_str() ), Qt::CaseInsensitive ) == 0 )
			return;  // already in list

	// the directory must exist
	QDir directory( QString::fromUtf8( dirPath.c_str() ) );
	if( !directory.exists() )
		return;  // directory does not exist

	// image is not in list, make a new entry and link to the list
	dir = new ImageDirectory;

	// allocate space for the path
	Int len = dirPath.size();
	dir->m_path = new char[ len + 1 ];
	strncpy( dir->m_path, dirPath.c_str(), len ); dir->m_path[len] = '\0';

	// tie to list
	dir->m_prev = NULL;
	dir->m_next = m_dirList;
	if( m_dirList )
		m_dirList->m_prev = dir;
	m_dirList = dir;

	// increase our directory count
	m_dirCount++;

	// update status
	snprintf( m_statusBuffer, sizeof(m_statusBuffer), "Folder Added: %d.", m_dirCount );
	statusMessage( m_statusBuffer );

	// count how many image files are in this directory
	const QFileInfoList files = directory.entryInfoList( QDir::Files );
	for( const QFileInfo &entry : files )
		if( hasTgaExtension( entry.fileName().toStdString() ) )
			dir->m_imageCount++;

	// add the image count of this directory to the total image count
	m_imagesInDirs += dir->m_imageCount;

	// if we are adding subdirectories add them all
	if( subDirs )
	{

		const QFileInfoList subdirs =
			directory.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
		for( const QFileInfo &entry : subdirs )
		{
			std::string subDir = dirPath + entry.fileName().toStdString() + "/";
			addDirectory( subDir.c_str(), subDirs );

		}  // end for

	}  // end if

}  // end addDirectory

// ImagePacker::addImage ======================================================
/** Add the image to the image list */
//=============================================================================
void ImagePacker::addImage( const char *path )
{

	// sanity
	if( path == NULL )
		return;

	// allocate a new entry
	ImageInfo *info = new ImageInfo;

	// allocate space for the path
	Int len = strlen( path );
	info->m_path = new char[ len + 1 ];
	strncpy( info->m_path, path, len ); info->m_path[len] = '\0';

	// load just the header information from the targa
	m_targa->Load( info->m_path, 0, TRUE );

	// get the data we need out of the targa header
	info->m_colorDepth = m_targa->Header.PixelDepth;
	info->m_size.x = m_targa->Header.Width;
	info->m_size.y = m_targa->Header.Height;
	info->m_area = info->m_size.x * info->m_size.y;

	// save the filename only without path (handle both path separators)
	QFileInfo fileInfo( QString::fromUtf8( path ) );
	std::string filenameOnly = fileInfo.fileName().toStdString();
	info->m_filenameOnly = new char[ filenameOnly.size() + 1 ];
	strncpy( info->m_filenameOnly, filenameOnly.c_str(), filenameOnly.size() ); info->m_filenameOnly[filenameOnly.size()] = '\0';

	// save the filename without the extension
	std::string filenameNoExt = fileInfo.completeBaseName().toStdString();
	info->m_filenameOnlyNoExt = new char[ filenameNoExt.size() + 1 ];
	strncpy( info->m_filenameOnlyNoExt, filenameNoExt.c_str(), filenameNoExt.size() ); info->m_filenameOnlyNoExt[filenameNoExt.size()] = '\0';

	// assign to array
	m_imageList[ m_imageCount++ ] = info;

	// update status
	snprintf( m_statusBuffer, sizeof(m_statusBuffer), "Loading Image %d of %d.",
					 m_imageCount, m_imagesInDirs );
	statusMessage( m_statusBuffer );

}  // end addImage

// ImagePacker::generateINIFile ===============================================
/** Generate the INI image file definition for the final packed images */
//=============================================================================
Bool ImagePacker::generateINIFile( void )
{
	FILE *fp;
	char filename[ _MAX_PATH ];

	// construct filename we'll use
	snprintf( filename, sizeof(filename), "%s%s.INI", m_outputDirectory, m_outputFile );

	// open the file
	fp = fopen( filename, "w" );
	if( fp == NULL )
	{
		char buffer[ _MAX_PATH + 64 ];

		snprintf( buffer, sizeof(buffer), "Cannot open INI file '%s' for writing.", filename );
		reportError( "Error Opening File", buffer );
		return FALSE;

	}  // end if

	// print header for file
	fprintf( fp, "; ------------------------------------------------------------\n" );
	fprintf( fp, "; Do NOT edit by hand, ImagePacker auto generated INI file\n" );
	fprintf( fp, "; ------------------------------------------------------------\n\n" );

	//
	// loop through all the pages so that we write image definitions that
	// are on the same page close together in the file, note we're
	// going backwards through the page list because page 1 is at the
	// tail and I want them to print out in number order, but it
	// doesn't really matter
	//
	TexturePage *page;
	ImageInfo *image;
	for( page = m_pageTail; page; page = page->m_prev )
	{

		// ignore texture pages that generated errors
		if( BitTest( page->m_status, TexturePage::PAGE_ERROR ) )
			continue;

		// go through each image on this page
		for( image = page->getFirstImage();
				 image;
				 image = image->m_nextPageImage )
		{

			//
			// write the item definition, note when we output the texture coords
			// we add on to the right and bottom to include that pixel in the
			// texture calculations ... need to do this since we are using a zero
			// based region for the "filled regions" in the image packer
			//
			fprintf( fp, "MappedImage %s\n", image->m_filenameOnlyNoExt );
			fprintf( fp, "  Texture = %s_%03d.tga\n", m_outputFile, page->getID() );
			fprintf( fp, "  TextureWidth = %d\n", page->getWidth() );
			fprintf( fp, "  TextureHeight = %d\n", page->getHeight() );
			fprintf( fp, "  Coords = Left:%d Top:%d Right:%d Bottom:%d\n",
							 image->m_pagePos.lo.x, image->m_pagePos.lo.y,
							 image->m_pagePos.hi.x + 1, image->m_pagePos.hi.y + 1 );
			fprintf( fp, "  Status = %s\n",
							 BitTest( image->m_status, ImageInfo::ROTATED90C ) ?
												"ROTATED_90_CLOCKWISE" : "NONE" );
			fprintf( fp, "End\n\n" );

		}  // end for image

	}  // end for page

	// close the file
	fclose( fp );

	return TRUE;  // success

}  // end generateINIFile

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ImagePacker::reportError ===================================================
/** Route an error message to the host, if any */
//=============================================================================
void ImagePacker::reportError( const char *title, const char *message )
{

	if( m_host )
		m_host->onError( title, message );
	else
		fprintf( stderr, "%s: %s\n", title, message );

}  // end reportError

// ImagePacker::setOutputFileName =============================================
/** Set the base name used for the output files */
//=============================================================================
void ImagePacker::setOutputFileName( const char *name )
{

	if( name == NULL )
	{
		m_outputFile[ 0 ] = '\0';
		return;
	}

	strncpy( m_outputFile, name, MAX_OUTPUT_FILE_LEN - 1 );
	m_outputFile[ MAX_OUTPUT_FILE_LEN - 1 ] = '\0';

}  // end setOutputFileName

// ImagePacker::clearDirectories ==============================================
/** Clear the list of source directories */
//=============================================================================
void ImagePacker::clearDirectories( void )
{

	resetImageDirectoryList();

}  // end clearDirectories

// ImagePacker::addSourceDirectory ============================================
/** Add a source directory (and optionally its sub-directories) to scan */
//=============================================================================
void ImagePacker::addSourceDirectory( const char *path, Bool subDirs )
{

	addDirectory( path, subDirs );

}  // end addSourceDirectory

// ImagePacker::ImagePacker ===================================================
ImagePacker::ImagePacker( void )
{

	m_host = NULL;
	m_targetSize.x = DEFAULT_TARGET_SIZE;
	m_targetSize.y = DEFAULT_TARGET_SIZE;
	m_useSubFolders = TRUE;
	strncpy( m_outputFile, "", sizeof(m_outputFile) ); m_outputFile[sizeof(m_outputFile)-1] = '\0';
	strncpy( m_outputDirectory, "", sizeof(m_outputDirectory) ); m_outputDirectory[sizeof(m_outputDirectory)-1] = '\0';
	m_dirList = NULL;
	m_dirCount = 0;
	m_imagesInDirs = 0;
	m_imageList = NULL;
	m_imageCount = 0;
	strncpy( m_statusBuffer, "", sizeof(m_statusBuffer) ); m_statusBuffer[sizeof(m_statusBuffer)-1] = '\0';
	m_pageList = NULL;
	m_pageTail = NULL;
	m_pageCount = 0;
	m_gapMethod = GAP_METHOD_EXTEND_RGB;
	m_gutterSize = 1;
	m_outputAlpha = TRUE;
	m_createINI = TRUE;

	m_targetPreviewPage = 1;
	m_showTextureInPreview = FALSE;

	m_targa = NULL;
	m_compressTextures = FALSE;

}  // end ImagePacker

// ImagePacker::~ImagePacker ==================================================
ImagePacker::~ImagePacker( void )
{

	// delete our lists
	resetImageDirectoryList();
	resetImageList();
	resetPageList();

	// delete our targa header loader
	if( m_targa )
		delete m_targa;

}  // end ~ImagePacker

// ImagePacker::init ==========================================================
/** Initialize the image packer system */
//=============================================================================
Bool ImagePacker::init( void )
{

	// allocate a targa to read the headers for the images
	m_targa = new Targa;

	return TRUE;

}  // end init

// ImagePacker::statusMessage =================================================
/** Status message for the program */
//=============================================================================
void ImagePacker::statusMessage( const char *message )
{

	if( m_host )
		m_host->onStatus( message );

}  // end statusMessage

// ImagePacker::process =======================================================
/** Run the packing process */
//=============================================================================
Bool ImagePacker::process( void )
{
	// build the output directory based on the base name of the output images
	QString base = QDir( QDir::currentPath() ).filePath( "ImagePackerOutput" );
	QDir().mkpath( base );

	// subdir of output directory based on output image name, keep a trailing
	// slash since the file name construction code assumes that
	QString outPath = QDir( base ).filePath( QString::fromUtf8( m_outputFile ) );
	std::string outDir = ensureTrailingSlash( outPath.toStdString() );
	strncpy( m_outputDirectory, outDir.c_str(), _MAX_PATH - 1 );
	m_outputDirectory[ _MAX_PATH - 1 ] = '\0';

	//
	// check for existing images in the output directory ... if we have
	// some then ask the user if they want to delete them or proceed with
	// a possible overwrite
	//
	if( checkOutputDirectory() == FALSE )
	{

		statusMessage( "Build Process Cancelled." );
		return FALSE;

	}  // end if

	// reset the contents of our image list and existing textures
	resetImageList();
	resetPageList();

	// set a status message
	statusMessage( "Gathering Image Information, Please Wait ..." );

	// allocate an array to hold all the images
	m_imageList = new ImageInfo *[ m_imagesInDirs ];

	// load our image list with all the art files from the specified directories
	ImageDirectory *dir;
	for( dir = m_dirList; dir; dir = dir->m_next )
		addImagesInDirectory( dir->m_path );

	// sort the images with the largest biggest images at the top of the list
	sortImageList();

	// pack all images
	if( packImages() )
	{

		// generate the actual final textures and write them out to the file
		writeFinalTextures();

		// generate the INI definition file if requested
		if( createINIFile() == TRUE )
			generateINIFile();

		// update preview window
		if( m_host )
			m_host->onProcessComplete( this );

		// all done
		snprintf( m_statusBuffer, sizeof(m_statusBuffer), "Image Packing Complete: '%d' Texture Pages Generated from '%d' Images in '%d' Folder(s)",
						 m_pageCount, m_imageCount, m_dirCount );
		statusMessage( m_statusBuffer );

	}  // end if

	return TRUE;

}  // end process
