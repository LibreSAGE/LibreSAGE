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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Udp.cpp //////////////////////////////////////////////////////////////
// Implementation of UDP socket wrapper class (taken from wnet lib)
// Author: Matthew D. Campbell, July 2001
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include <SDL3/SDL.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/GameEngine.h"
//#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/udp.h"

UDP::UDP() : 
  socket(NULL),
  myAddr(NULL),
  myPort(0),
  m_lastError(NULL)
{
}

UDP::~UDP()
{
	if (socket)
	{
    NET_DestroyDatagramSocket(socket);
    socket = NULL;
	}
}

Int UDP::Bind(const char *Host,UnsignedShort port)
{
  NET_Address *addr = NET_ResolveHostname(Host);
  NET_Status status = NET_WaitUntilResolved(addr, 1000);

  if (status != NET_SUCCESS) {
    DEBUG_LOG(("UDP::Bind() - Failed to resolve hostname %s\n", Host));
    m_lastError = SDL_GetError();
    return 0;
  }

  return Bind(addr, port);
}

// You must call bind, implicit binding is for sissies
//   Well... you can get implicit binding if you pass 0 for either arg
Int UDP::Bind(NET_Address* Addr,UnsignedShort Port)
{
  // Allow broadcasting on all of our sockets - the LAN lobby relies on it, and
  // SDL3_net only lets us configure this at socket creation time.
  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetBooleanProperty(props, NET_PROP_DATAGRAM_SOCKET_ALLOW_BROADCAST_BOOLEAN, true);

  NET_DatagramSocket * datagram = NET_CreateDatagramSocket(Addr, Port, props);

  SDL_DestroyProperties(props);

  if (datagram == NULL)
  {
    DEBUG_LOG(("UDP::Bind() - Failed to create datagram socket\n"));
    m_lastError = SDL_GetError();
    return UNKNOWN;
  }

  socket = datagram;
  myAddr = Addr;
  myPort = Port;

  return(OK);
}

Int UDP::getLocalAddr(NET_Address* &addr, UnsignedShort &port)
{
  addr = myAddr;
  port = myPort;
  return(OK);
}

Int UDP::Write(const unsigned char *msg,UnsignedInt len,NET_Address* addr,UnsignedShort port)
{
  // A NULL address is valid - SDL3_net treats it as a broadcast. Only a missing
  // port means we have nowhere to send to.
  if (port==0) return(ADDRNOTAVAIL);

  if (!NET_SendDatagram(socket, addr, port, msg, len))
  {
    m_lastError = SDL_GetError();
    DEBUG_LOG(("UDP::Write() - Failed to send datagram\n"));
    return -1;
  }
  
  return len;
}

Int UDP::Read(unsigned char *msg,UnsignedInt len,NET_Address *&from,UnsignedShort &fromPort)
{
  NET_Datagram* datagram = NULL;
  if (!NET_ReceiveDatagram(socket, &datagram))
  {
    m_lastError = SDL_GetError();
    return -1;
  }

  // A successful call with no waiting packet returns a NULL datagram.
  if (datagram == NULL)
  {
    return 0;
  }

  Int recvLen = datagram->buflen;
  if ((UnsignedInt)recvLen > len)
    recvLen = len;

  // Hold onto the sender's address; the caller is responsible for unref'ing it.
  from = NET_RefAddress(datagram->addr);
  fromPort = datagram->port;
  memcpy(msg, datagram->buf, recvLen);
  NET_DestroyDatagram(datagram);
  return recvLen;
}


void UDP::ClearStatus(void)
{
  SDL_ClearError();
	m_lastError = 0;
}

const char* UDP::GetStatus(void)
{
	return m_lastError ? m_lastError : "";
}

/*
//
// Wait for net activity on this socket
//
int UDP::Wait(Int sec,Int usec,fd_set &returnSet)
{
  fd_set inputSet;
 
  FD_ZERO(&inputSet);
  FD_SET(fd,&inputSet);
 
  return(Wait(sec,usec,inputSet,returnSet));
}
*/

/*
//
// Wait for net activity on a list of sockets
//
int UDP::Wait(Int sec,Int usec,fd_set &givenSet,fd_set &returnSet)
{
  Wtime        timeout,timenow,timethen;
  fd_set       backupSet;
  int          retval=0,done,givenMax;
  Bool         noTimeout=FALSE;
  timeval      tv;
 
  returnSet=givenSet;
  backupSet=returnSet;
 
  if ((sec==-1)&&(usec==-1))
    noTimeout=TRUE;
 
  timeout.SetSec(sec);
  timeout.SetUsec(usec);
  timethen+=timeout;
 
  givenMax=fd;
  for (UnsignedInt i=0; i<(sizeof(fd_set)*8); i++)   // i=maxFD+1
  {
    if (FD_ISSET(i,&givenSet))
      givenMax=i;
  }
  ///DBGMSG("WAIT  fd="<<fd<<"  givenMax="<<givenMax);
 
  done=0;
  while( ! done)
  {
    if (noTimeout)
      retval=select(givenMax+1,&returnSet,0,0,NULL);
    else
    {
      timeout.GetTimevalMT(tv);
      retval=select(givenMax+1,&returnSet,0,0,&tv);
    }
 
    if (retval>=0)
      done=1;

    else if ((retval==-1)&&(errno==EINTR))  // in case of signal
    {
      if (noTimeout==FALSE)
      {
        timenow.Update();
        timeout=timethen-timenow;
      }
      if ((noTimeout==FALSE)&&(timenow.GetSec()==0)&&(timenow.GetUsec()==0))
        done=1;
      else
        returnSet=backupSet;
    }
    else  // maybe out of memory?
    {
      done=1;
    }
  }
  ///DBGMSG("Wait retval: "<<retval);
  return(retval);
}
*/

Int UDP::AllowBroadcasts(Bool status)
{
	// Broadcasting is enabled at socket-creation time in Bind() (SDL3_net only
	// supports configuring it there), so there is nothing to toggle here.
	return TRUE;
}
