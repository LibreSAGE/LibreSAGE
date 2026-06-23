/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : wwaudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/sound2dhandle.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 3:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUND2DHANDLE_H
#define __SOUND2DHANDLE_H

#include "soundhandle.h"


//////////////////////////////////////////////////////////////////////
//
//	Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
class Sound2DHandleClass : public SoundHandleClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	Sound2DHandleClass (void);
	~Sound2DHandleClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	Sound2DHandleClass *		As_Sound2DHandleClass (void)	{ return this; }

	//
	//	Handle access
	//
#ifdef SAGE_USE_MILES
	HSAMPLE						Get_HSAMPLE (void)		{ return SampleHandle; }
#else
	virtual void*			Get_HSAMPLE (void)		{ return NULL; }
#endif

	//
	//	Inherited
	//
	void							Set_Miles_Handle (uint32 handle);
	void							Initialize (SoundBufferClass *buffer);
	void							Start_Sample (void);
	void							Stop_Sample (void);
	void							Resume_Sample (void);
	void							End_Sample (void);
	void							Set_Sample_Pan (int32_t pan);
	int32_t							Get_Sample_Pan (void);
	void							Set_Sample_Volume (int32_t volume);
	int32_t							Get_Sample_Volume (void);
	void							Set_Sample_Loop_Count (uint32_t count);
	uint32_t						Get_Sample_Loop_Count (void);
	void							Set_Sample_MS_Position (uint32_t ms);
	void							Get_Sample_MS_Position (int32_t *len, int32_t *pos);
	void							Set_Sample_User_Data (int32_t i, uintptr_t val);
	uintptr_t						Get_Sample_User_Data (int32_t i);
	int32_t							Get_Sample_Playback_Rate (void);
	void							Set_Sample_Playback_Rate (int32_t rate);
	
protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
#ifdef SAGE_USE_MILES
	HSAMPLE		SampleHandle;
#endif
};


#endif //__SOUND2DHANDLE_H
