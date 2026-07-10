/*
**	Command & Conquer Generals(tm)
**	Command & Conquer Generals Zero Hour(tm)
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
 *                     $Archive:: /Commando/Code/WWAudio/soundstreamhandle.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/23/01 4:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soundstreamhandle.h"
#include "audiblesound.h"


//////////////////////////////////////////////////////////////////////
//
//	SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundStreamHandleClass::SoundStreamHandleClass (void)	
#ifdef SAGE_USE_MILES
	: 
	SampleHandle ((HSAMPLE)INVALID_MILES_HANDLE),
	StreamHandle ((HSTREAM)INVALID_MILES_HANDLE)
#endif
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundStreamHandleClass::~SoundStreamHandleClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Initialize (SoundBufferClass *buffer)
{
	SoundHandleClass::Initialize (buffer);

	if (Buffer != NULL) {
	#ifdef SAGE_USE_MILES
		//
		//	Create a stream from the sample handle
		//
		StreamHandle = ::AIL_open_stream_by_sample (WWAudioClass::Get_Instance ()->Get_2D_Driver (),
								SampleHandle, buffer->Get_Filename (), 0);

		/*StreamHandle = ::AIL_open_stream (WWAudioClass::Get_Instance ()->Get_2D_Driver (),
								buffer->Get_Filename (), 0);*/
	#endif
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Start_Sample (void)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_start_stream (StreamHandle);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Stop_Sample (void)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_pause_stream (StreamHandle, 1);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Resume_Sample (void)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_pause_stream (StreamHandle, 0);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::End_Sample (void)
{
	//
	//	Stop the sample and then release our hold on the stream handle
	//
	Stop_Sample ();
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_close_stream (StreamHandle);
		StreamHandle = (HSTREAM)INVALID_MILES_HANDLE;
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Pan (int32_t pan)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_pan (StreamHandle, pan);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
int32_t
SoundStreamHandleClass::Get_Sample_Pan (void)
{
	int32_t retval = 0;
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_pan (StreamHandle);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Volume (int32_t volume)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_volume (StreamHandle, volume);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
int32_t
SoundStreamHandleClass::Get_Sample_Volume (void)
{
	int32_t retval = 0;
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_volume (StreamHandle);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Loop_Count (uint32_t count)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_loop_block (StreamHandle, 0, -1);
		::AIL_set_stream_loop_count (StreamHandle, count);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
uint32_t
SoundStreamHandleClass::Get_Sample_Loop_Count (void)
{
	uint32_t retval = 0;
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_stream_loop_count (StreamHandle);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_MS_Position (uint32_t ms)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_ms_position (StreamHandle, ms);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Get_Sample_MS_Position (int32_t *len, int32_t *pos)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_stream_ms_position (StreamHandle, len, pos);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_User_Data (int32_t i, uintptr_t val)
{
#ifdef SAGE_USE_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_user_data (SampleHandle, i, val);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
uintptr_t
SoundStreamHandleClass::Get_Sample_User_Data (int32_t i)
{
	uintptr_t retval = 0;
#ifdef SAGE_USE_MILES
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_user_data (SampleHandle, i);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
int32_t
SoundStreamHandleClass::Get_Sample_Playback_Rate (void)
{	
	int32_t retval = 0;
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_playback_rate (StreamHandle);
	}
#endif
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Playback_Rate (int32_t rate)
{
#ifdef SAGE_USE_MILES
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_playback_rate (StreamHandle, rate);
	}
#endif
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Miles_Handle (uint32_t handle)
{
#ifdef SAGE_USE_MILES
	SampleHandle = (HSAMPLE)handle;
#endif
	return ;
}
