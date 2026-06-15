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

#include "mutex.h"
#include "wwdebug.h"
#ifdef _WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif

// ----------------------------------------------------------------------------

MutexClass::MutexClass(const char* name) : handle(NULL), locked(false)
{
	#ifdef _UNIX
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		handle = new pthread_mutex_t;
		pthread_mutex_init((pthread_mutex_t*)handle, &attr);
		pthread_mutexattr_destroy(&attr);
	#else
		handle=CreateMutex(NULL,false,name);
		WWASSERT(handle);
	#endif
}

MutexClass::~MutexClass()
{
	WWASSERT(!locked); // Can't delete locked mutex!
	#ifdef _UNIX
		pthread_mutex_destroy((pthread_mutex_t*)handle);
		delete (pthread_mutex_t*)handle;
	#else
		CloseHandle(handle);
	#endif
}

bool MutexClass::Lock(int time)
{
	#ifdef _UNIX
		int res;
		if (time==WAIT_INFINITE) {
			res = pthread_mutex_lock((pthread_mutex_t*)handle);
			if (res != 0) 
				return false;
		}
		else if(time==0) {
			res = pthread_mutex_trylock((pthread_mutex_t*)handle);
			if (res != 0) {
				return false;
			}
		}
		else {
			// pthread_mutex_timedlock() expects an ABSOLUTE deadline (CLOCK_REALTIME),
			// not a relative timeout, so add the requested wait (in milliseconds) to now.
			struct timespec timeoutTime;
			clock_gettime(CLOCK_REALTIME, &timeoutTime);
			timeoutTime.tv_sec += time / 1000;
			timeoutTime.tv_nsec += (long)(time % 1000) * 1000000L; // ms -> ns
			if (timeoutTime.tv_nsec >= 1000000000L) {
				timeoutTime.tv_sec += 1;
				timeoutTime.tv_nsec -= 1000000000L;
			}
			res = pthread_mutex_timedlock((pthread_mutex_t*)handle, &timeoutTime);
			if (res != 0)
				return false;
		}
	#else
		int res = WaitForSingleObject(handle,time==WAIT_INFINITE ? INFINITE : time);
		if (res!=WAIT_OBJECT_0) return false;
	#endif
	locked++;
	return true;
}

void MutexClass::Unlock()
{
	WWASSERT(locked);
	#ifdef _UNIX
		int res = pthread_mutex_unlock((pthread_mutex_t*)handle);
		res;	// silence compiler warnings
		WWASSERT(res==0);
	#else
		int res=ReleaseMutex(handle);
		res;	// silence compiler warnings
		WWASSERT(res);
	#endif
	locked--;
}

// ----------------------------------------------------------------------------

MutexClass::LockClass::LockClass(MutexClass& mutex_,int time) : mutex(mutex_)
{
	failed=!mutex.Lock(time);
}

MutexClass::LockClass::~LockClass()
{
	if (!failed) mutex.Unlock();
}







// ----------------------------------------------------------------------------

CriticalSectionClass::CriticalSectionClass() : handle(NULL), locked(false)
{
	#ifdef _UNIX
		// A critical section can be entered recursively by the same thread, so the
		// backing pthread mutex must be recursive too.
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		handle = new pthread_mutex_t;
		pthread_mutex_init((pthread_mutex_t*)handle, &attr);
		pthread_mutexattr_destroy(&attr);
	#else
		handle=W3DNEWARRAY char[sizeof(CRITICAL_SECTION)];
		InitializeCriticalSection((CRITICAL_SECTION*)handle);
	#endif
}

CriticalSectionClass::~CriticalSectionClass()
{
	WWASSERT(!locked); // Can't delete locked critical section!
	#ifdef _UNIX
		pthread_mutex_destroy((pthread_mutex_t*)handle);
		delete (pthread_mutex_t*)handle;
	#else
		DeleteCriticalSection((CRITICAL_SECTION*)handle);
		delete[] handle;
	#endif
}

void CriticalSectionClass::Lock()
{
	#ifdef _UNIX
		pthread_mutex_lock((pthread_mutex_t*)handle);
		locked++;
	#else
		EnterCriticalSection((CRITICAL_SECTION*)handle);
		locked++;
	#endif
}

void CriticalSectionClass::Unlock()
{
	WWASSERT(locked);
	#ifdef _UNIX
		locked--;
		pthread_mutex_unlock((pthread_mutex_t*)handle);
	#else
		LeaveCriticalSection((CRITICAL_SECTION*)handle);
		locked--;
	#endif
}

// ----------------------------------------------------------------------------

CriticalSectionClass::LockClass::LockClass(CriticalSectionClass& critical_section) : CriticalSection(critical_section)
{
	CriticalSection.Lock();
}

CriticalSectionClass::LockClass::~LockClass()
{
	CriticalSection.Unlock();
}


