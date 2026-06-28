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


#include "thread.h"
#include "wwdebug.h"
#ifdef _WINDOWS
#define _WIN32_WINNT 0x0400
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#endif
#include "systimer.h"


ThreadClass::ThreadClass(const char *thread_name) : handle(0), running(false), thread_priority(0)
{
	if (thread_name) {
		assert(strlen(thread_name) < sizeof(ThreadName) - 1);
		strcpy(ThreadName, thread_name);
	} else {
		strcpy(ThreadName, "WWVegasThread");;
	}
}

ThreadClass::~ThreadClass()
{
	Stop();
}

void __cdecl ThreadClass::Internal_Thread_Function(void* params)
{
	ThreadClass* tc=reinterpret_cast<ThreadClass*>(params);
	tc->running=true;
	tc->ThreadID = _Get_Current_Thread_ID();
	tc->Thread_Function();
#ifdef _UNIX
	// Free the pthread_t that Execute() heap-allocated. Zero the handle first so that
	// a concurrent Stop() sees the thread as finished and does not also delete it
	// (Stop() only frees the handle on its timeout/kill path).
	pthread_t* h = (pthread_t*)tc->handle;
	tc->handle=0;
	delete h;
#else
	tc->handle=0;
#endif
	tc->ThreadID = 0;
}

void ThreadClass::Execute()
{
	WWASSERT(!handle);	// Only one thread at a time!
	#ifdef _UNIX
		static auto pthread_wrapper = [](void* params) -> void* {
			ThreadClass::Internal_Thread_Function(params);
			return nullptr;
		};

		handle = new pthread_t;
		int res = pthread_create((pthread_t*)handle, NULL, pthread_wrapper, this);
		if (res == 0) {
			pthread_detach(*(pthread_t*)handle);
			pthread_setname_np(*(pthread_t*)handle, ThreadName);
		}
	#else
		handle=_beginthread(&Internal_Thread_Function,0,this);
		SetThreadPriority((HANDLE)handle,THREAD_PRIORITY_NORMAL+thread_priority);
		SetThreadDescription((HANDLE)handle, ThreadName);
	#endif
	WWDEBUG_SAY(("ThreadClass::Execute: Started thread %s, thread ID is %X\n", ThreadName, handle));
}

void ThreadClass::Set_Priority(int priority)
{
	thread_priority=priority;
	#ifdef _UNIX
		if (handle) {
			pthread_setschedprio(*(pthread_t*)handle, thread_priority);
		}
	#else
		if (handle) SetThreadPriority((HANDLE)handle,THREAD_PRIORITY_NORMAL+thread_priority);
	#endif
}

void ThreadClass::Stop(unsigned ms)
{
	running=false;
	unsigned time=TIMEGETTIME();
	while (handle) {
		if ((TIMEGETTIME()-time)>ms) {
			#ifdef _UNIX
			int res = pthread_kill(*(pthread_t*)handle, SIGKILL);
			WWASSERT(res == 0);	// Thread still not killed!
			delete (pthread_t*)handle;
			#else
			int res=TerminateThread((HANDLE)handle,0);
			WWASSERT(res);	// Thread still not killed!
			#endif
			handle=0;
		}
		Sleep(0);
	}
}

void ThreadClass::Sleep_Ms(unsigned ms)
{
	Sleep(ms);
}


void ThreadClass::Switch_Thread()
{
	#ifdef _UNIX
		::sched_yield();
	#else
		::SwitchToThread ();
	#endif
}

// Return calling thread's unique thread id
unsigned ThreadClass::_Get_Current_Thread_ID()
{
	#ifdef _UNIX
		return (unsigned)pthread_self();
	#else
		return GetCurrentThreadId();
	#endif
}

bool ThreadClass::Is_Running()
{
	return handle != NULL;
}