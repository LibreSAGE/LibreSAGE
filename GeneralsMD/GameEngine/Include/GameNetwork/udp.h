/*
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


#pragma once

#ifndef UDP_HEADER
#define UDP_HEADER

#include <SDL3_net/SDL_net.h>

#include "Lib/BaseType.h"

#define DEFAULT_PROTOCOL 0

//#include "wlib/wstypes.h"
//#include "wlib/wtime.h"

class UDP
{
 // DATA
 private:
  NET_DatagramSocket* socket; 
  UnsignedShort       myPort;
  NET_Address*        myAddr;
  
 public:
  // These defines specify a system independent way to
  //   get error codes for socket services.
  enum sockStat
  {
    OK           =  0,     // Everything's cool
    UNKNOWN      = -1,     // There was an error of unknown type
    ADDRNOTAVAIL = -7      // That address is not available on the remote host
  };

// CODE
 private:
	const char* m_lastError;

 public:
                   UDP();
                  ~UDP();
  Int           Bind(NET_Address* addr,UnsignedShort port);
  Int           Bind(const char *Host,UnsignedShort port);
  Int           Write(const unsigned char *msg,UnsignedInt len,NET_Address* addr,UnsignedShort port);
  Int           Read(unsigned char *msg,UnsignedInt len,NET_Address *&from,UnsignedShort &fromPort);
  const char*      GetStatus(void);
  void             ClearStatus(void);
  //int              Wait(Int sec,Int usec,fd_set &returnSet);
  //int              Wait(Int sec,Int usec,fd_set &givenSet,fd_set &returnSet);

  Int             getLocalAddr(NET_Address* &addr, UnsignedShort &port);
  NET_DatagramSocket* getSocket(void) { return(socket); }

	Int						AllowBroadcasts(Bool status);
};

#endif
