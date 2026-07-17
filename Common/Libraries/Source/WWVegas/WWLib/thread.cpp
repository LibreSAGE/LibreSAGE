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
#endif
#include "systimer.h"


ThreadClass::ThreadClass(const char *thread_name) : handle(0), running(false), thread_priority(0)
{
	if (thread_name) {
		assert(strlen(thread_name) < sizeof(ThreadName) - 1);
		strncpy(ThreadName, thread_name, sizeof(ThreadName));
		ThreadName[sizeof(ThreadName) - 1] = '\0';
	} else {
		strncpy(ThreadName, "WWVegasThread", sizeof(ThreadName));
		ThreadName[sizeof(ThreadName) - 1] = '\0';
	}
}

ThreadClass::~ThreadClass()
{
	Stop();
}

void __cdecl ThreadClass::Internal_Thread_Function(void* params)
{
	ThreadClass* tc=reinterpret_cast<ThreadClass*>(params);
	tc->ThreadID = _Get_Current_Thread_ID();
	tc->Thread_Function();
#ifdef _UNIX
	// Free the pthread_t that Execute() heap-allocated. Zero the handle first: that is what tells a
	// waiting Stop() the thread has finished.
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

	// Raise the flag here rather than from inside the thread. running is what Stop() clears to ask
	// the thread to exit, so a thread that raises it on start-up can overwrite -- and silently
	// discard -- a Stop() that got in first, leaving the thread looping over a request it never saw.
	running=true;

	#ifdef _UNIX
		static auto pthread_wrapper = [](void* params) -> void* {
			ThreadClass::Internal_Thread_Function(params);
			return nullptr;
		};

		// handle must be published before the thread starts: the thread reads it to free it.
		handle = new pthread_t;
		int res = pthread_create((pthread_t*)handle, NULL, pthread_wrapper, this);
		if (res != 0) {
			delete (pthread_t*)handle;
			handle=0;
			running=false;
			WWDEBUG_SAY(("ThreadClass::Execute: Failed to start thread %s\n", ThreadName));
			return;
		}
		pthread_detach(*(pthread_t*)handle);
		pthread_setname_np(*(pthread_t*)handle, ThreadName);
	#else
		handle=(void*)_beginthread(&Internal_Thread_Function,0,this);
		if (handle == (void*)-1) {
			handle=0;
			running=false;
			WWDEBUG_SAY(("ThreadClass::Execute: Failed to start thread %s\n", ThreadName));
			return;
		}
		SetThreadPriority((HANDLE)handle,THREAD_PRIORITY_NORMAL+thread_priority);
		SetThreadDescription((HANDLE)handle, ThreadName);
	#endif
	WWDEBUG_SAY(("ThreadClass::Execute: Started thread %s, thread ID is %p\n", ThreadName, (void*)handle));
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

	// Wait for Thread_Function() to notice and return; the thread clears handle on its way out.
	// There is no force-kill: a thread cannot be terminated from the outside safely. SIGKILL is
	// delivered to the whole process rather than to one thread, so it takes the game down with it,
	// and TerminateThread abandons whatever locks the thread was holding. A Thread_Function() that
	// keeps going once running is false is a bug in that thread, so report it and keep waiting.
	unsigned time=TIMEGETTIME();
	bool overdue=false;
	while (handle) {
		if (!overdue && (TIMEGETTIME()-time)>ms) {
			WWDEBUG_SAY(("ThreadClass::Stop: thread %s has not exited after %u ms, still waiting\n", ThreadName, ms));
			overdue=true;
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