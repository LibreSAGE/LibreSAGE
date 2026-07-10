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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: ThreadUtils.cpp //////////////////////////////////////////////////////
// GameSpy thread utils
// Author: Matthew D. Campbell, July 2002

#include "Common/STLTypedefs.h"
#include <unicode/ustring.h>

//-------------------------------------------------------------------------

std::u16string MultiByteToWideCharSingleLine( const char *orig )
{
	if (orig == NULL)
		return std::u16string();

	// Preflight to get the length (in UTF-16 units, excluding the null). ICU reports
	// U_BUFFER_OVERFLOW_ERROR here because the destination is NULL/zero-length - that
	// is expected, not a failure, so we must not bail out on it.
	Int len = 0;
	UErrorCode err = U_ZERO_ERROR;
	u_strFromUTF8(NULL, 0, &len, orig, -1, &err); // get length
	if (len <= 0)
	{
		return std::u16string();
	}

	WideChar *dest = NEW WideChar[len+1];

	// MultiByteToWideChar(CP_UTF8, 0, orig, -1, dest, len);
	err = U_ZERO_ERROR;
	u_strFromUTF8(dest, len+1, NULL, orig, -1, &err);
	if (U_FAILURE(err))
	{
		delete[] dest;
		return std::u16string();
	}
	dest[len] = 0;
	WideChar *c = NULL;
	do
	{
		c = u_strchr(dest, u'\n');
		if (c)
		{
			*c = u' ';
		}
	}
	while ( c != NULL );
	do
	{
		c = u_strchr(dest, u'\r');
		if (c)
		{
			*c = u' ';
		}
	}
	while ( c != NULL );

	dest[len] = 0;
	std::u16string ret = dest;
	delete[] dest;
	return ret;
}

std::string WideCharStringToMultiByte( const WideChar *orig )
{
	if (orig == NULL)
		return std::string();

	// Preflight to get the required UTF-8 length (excluding the null terminator).
	// Passing a NULL/zero-length destination makes ICU report the needed size via
	// U_BUFFER_OVERFLOW_ERROR - that is expected here and is NOT a real failure, so
	// we must not treat it as one (doing so silently returned an empty string).
	Int len = 0;
	UErrorCode err = U_ZERO_ERROR;
	u_strToUTF8(NULL, 0, &len, orig, -1, &err);
	if (len <= 0)
		return std::string();

	std::string ret;
	ret.resize(len);
	err = U_ZERO_ERROR;
	u_strToUTF8(&ret[0], len + 1, NULL, orig, -1, &err);
	if (U_FAILURE(err))
		return std::string();

	return ret;
}

//-------------------------------------------------------------------------

