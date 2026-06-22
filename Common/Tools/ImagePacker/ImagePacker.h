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

// FILE: imagepacker.h ////////////////////////////////////////////////////////
//
// Project:    ImagePacker
//
// Created:    Colin Day, August 2001
//
// Desc:       Image packer tool.  This is the platform independent packing
//             engine; all user interaction is routed through ImagePackerHost
//             so that the Qt front end can drive it.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lib/BaseType.h"
#include "targa.h"
#include "ImageDirectory.h"
#include "ImageInfo.h"
#include "TexturePage.h"

// TYPE DEFINES ///////////////////////////////////////////////////////////////
#define MAX_OUTPUT_FILE_LEN 128
#define DEFAULT_TARGET_SIZE 512

class ImagePacker;

// ImagePackerHost ------------------------------------------------------------
/** Abstract interface the packing engine uses to talk back to whatever UI
	* is driving it.  The Qt main window implements this. */
//-----------------------------------------------------------------------------
class ImagePackerHost
{

public:

	virtual ~ImagePackerHost() {}

	/// update the status text shown to the user
	virtual void onStatus( const char *message ) {}

	/// report an error condition to the user
	virtual void onError( const char *title, const char *message ) {}

	/// some source images can't be processed, return true to proceed anyway
	virtual Bool confirmImageErrors( ImagePacker *packer ) { return TRUE; }

	/// the output directory has 'count' files in it, return true to delete and proceed
	virtual Bool confirmDeleteFiles( const char *directory, Int count ) { return TRUE; }

	/// one or more texture pages failed to generate
	virtual void reportPageErrors( ImagePacker *packer ) {}

	/// a packing run has finished, refresh any preview
	virtual void onProcessComplete( ImagePacker *packer ) {}

};

// ImagePacker ----------------------------------------------------------------
// Class interface for running the image packer */
//-----------------------------------------------------------------------------
class ImagePacker
{

public:

	enum
	{
		GAP_METHOD_EXTEND_RGB	= 0x00000001,  ///< extend RGB (no alpha) of image on all sides
		GAP_METHOD_GUTTER			= 0x00000002,  ///< put transparent gutter on right and bottom side of image
	};

public:

	ImagePacker( void );
	virtual ~ImagePacker( void );

	Bool init( void );  ///< initialize the system
	Bool process( void );  ///< run the process

	void setHost( ImagePackerHost *host );  ///< set the UI host
	ImagePackerHost *getHost( void );  ///< get the UI host

	void reportError( const char *title, const char *message );  ///< route an error to the host

	ICoord2D *getTargetSize( void );  ///< get target size
	Int getTargetWidth( void );  ///< get target width
	Int getTargetHeight( void );  ///< bet target height
	void setTargetSize( Int width, Int height );  ///< set the size of the output target image

	void statusMessage( const char *message );  ///< set a status message

	UnsignedInt getImageCount( void );  ///< get image count
	ImageInfo *getImage( Int index );  ///< get image
	TexturePage *getFirstTexturePage( void );  ///< get first texture page

	UnsignedInt getPageCount( void );  ///< get the count of texutre pages

	void setTargetPreviewPage( Int page );  ///< set the target preview page to view
	Int getTargetPreviewPage( void );  ///< get the target preview page to view

	void setGutter( UnsignedInt size );  ///< set gutter size in pixels
	UnsignedInt getGutter( void );  ///< get gutter size in pixels
	void setGapMethod( UnsignedInt methodBit );  ///< set gap method option
	void clearGapMethod( UnsignedInt methodBit );  ///< clear gap method option
	UnsignedInt getGapMethod( void );  ///< get gap method option

	void setOutputAlpha( Bool outputAlpha );  ///< set output alpha option
	Bool getOutputAlpha( void );  ///< get output alpha option

	void setUseTexturePreview( Bool use );  ///< use the real image data in preview
	Bool getUseTexturePreview( void );  ///< get texture preview option

	void setINICreate( Bool create );  ///< set create INI file option
	Bool createINIFile( void );  ///< get create INI option

	void setOutputFileName( const char *name );  ///< set output filename base
	char *getOutputFile( void );  ///< get output filename
	char *getOutputDirectory( void );  ///< get output directory

	void setCompressTextures( Bool compress );  ///< set compress textures option
	Bool getCompressTextures( void );  ///< get compress textures option

	void setUseSubFolders( Bool use );  ///< set use sub-folders option
	Bool getUseSubFolders( void );  ///< get use sub-folders option

	void clearDirectories( void );  ///< clear the list of source directories
	void addSourceDirectory( const char *path, Bool subDirs );  ///< add a source directory

protected:

	Bool checkOutputDirectory( void );  ///< verify output directory is OK

	void resetImageDirectoryList( void );  ///< clear the image directory list
	void resetImageList( void );  ///< clear the image list
	void resetPageList( void );  ///< clear the texture page list
	void addDirectory( const char *path, Bool subDirs );  ///< add directory to directory list
	void addImagesInDirectory( const char *dir );  ///< add all images from the specified directory
	void addImage( const char *path );  ///< add image to image list
	Bool validateImages( void );  ///< validate that the loaded images can all be processed
	Bool packImages( void );  ///< do the packing
	void writeFinalTextures( void );  ///< write the packed textures

	Bool generateINIFile( void );  ///< generate the INI file for this image set

	TexturePage *createNewTexturePage( void );  ///< create a new texture page

	void sortImageList( void );  ///< sort the image list

	ImagePackerHost *m_host;  ///< the UI host
	ICoord2D m_targetSize;  ///< the target size
	Bool m_useSubFolders;  ///< use subfolders option
	char m_outputFile[ MAX_OUTPUT_FILE_LEN ];  ///< output filename
	char m_outputDirectory[ _MAX_PATH ];  ///< destination for texture files

	ImageDirectory *m_dirList;  ///< the directory list
	UnsignedInt m_dirCount;  ///< length of dirList
	UnsignedInt m_imagesInDirs;  ///< number of images in all directories
	ImageInfo **m_imageList;  ///< the image list
	UnsignedInt m_imageCount;  ///< length of imageList
	char m_statusBuffer[ 1024 ];  ///< for printing status messages
	TexturePage *m_pageTail;  ///< end of the texture page list
	TexturePage *m_pageList;  ///< the final images generated from the packer
	UnsignedInt m_pageCount;  ///< length of page list
	UnsignedInt m_gapMethod;  ///< gap method option bits
	UnsignedInt m_gutterSize;  ///< gutter gaps between images in pixels
	Bool m_outputAlpha;  ///< final image files will have an alpha channel
	Bool m_createINI;  ///< create the INI file from compressed image data

	Int m_targetPreviewPage;  ///< preview page we're looking at
	Bool m_showTextureInPreview;  ///< show actual texture in preview window

	Targa *m_targa;  ///< targa for loading file headers
	Bool m_compressTextures;  ///< compress the final textures

};

// INLINING ///////////////////////////////////////////////////////////////////
inline void ImagePacker::setHost( ImagePackerHost *host ) { m_host = host; }
inline ImagePackerHost *ImagePacker::getHost( void ) { return m_host; }
inline void ImagePacker::setTargetSize( Int width, Int height ) { m_targetSize.x = width; m_targetSize.y = height; }
inline ICoord2D *ImagePacker::getTargetSize( void ) { return &m_targetSize; }
inline Int ImagePacker::getTargetWidth( void ) { return m_targetSize.x; }
inline Int ImagePacker::getTargetHeight( void ) { return m_targetSize.y; }
inline UnsignedInt ImagePacker::getImageCount( void ) { return m_imageCount; }
inline ImageInfo *ImagePacker::getImage( Int index ) { return m_imageList[ index ]; }
inline void ImagePacker::setTargetPreviewPage( Int page ) { m_targetPreviewPage = page; }
inline Int ImagePacker::getTargetPreviewPage( void ) { return m_targetPreviewPage; }
inline UnsignedInt ImagePacker::getPageCount( void ) { return m_pageCount; }
inline void ImagePacker::setGutter( UnsignedInt size ) { m_gutterSize = size; }
inline UnsignedInt ImagePacker::getGutter( void ) { return m_gutterSize; }
inline void ImagePacker::setOutputAlpha( Bool outputAlpha ) { m_outputAlpha = outputAlpha; }
inline Bool ImagePacker::getOutputAlpha( void ) { return m_outputAlpha; }
inline TexturePage *ImagePacker::getFirstTexturePage( void ) { return m_pageList; }
inline void ImagePacker::setUseTexturePreview( Bool use ) { m_showTextureInPreview = use; }
inline Bool ImagePacker::getUseTexturePreview( void ) { return m_showTextureInPreview; }
inline void ImagePacker::setINICreate( Bool create ) { m_createINI = create; }
inline Bool ImagePacker::createINIFile( void ) { return m_createINI; }
inline char *ImagePacker::getOutputFile( void ) { return m_outputFile; }
inline char *ImagePacker::getOutputDirectory( void ) { return m_outputDirectory; }
inline void ImagePacker::setCompressTextures( Bool compress ) { m_compressTextures = compress; }
inline Bool ImagePacker::getCompressTextures( void ) { return m_compressTextures; }
inline void ImagePacker::setUseSubFolders( Bool use ) { m_useSubFolders = use; }
inline Bool ImagePacker::getUseSubFolders( void ) { return m_useSubFolders; }
inline void ImagePacker::setGapMethod( UnsignedInt methodBit ) { BitSet( m_gapMethod, methodBit ); }
inline void ImagePacker::clearGapMethod( UnsignedInt methodBit ) { BitClear( m_gapMethod, methodBit ); }
inline UnsignedInt ImagePacker::getGapMethod( void ) { return m_gapMethod; }

// EXTERNALS //////////////////////////////////////////////////////////////////
extern ImagePacker *TheImagePacker;
