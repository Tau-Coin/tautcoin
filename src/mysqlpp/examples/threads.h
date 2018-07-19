/***********************************************************************
 threads.h - Abstracts away the differences between POSIX threads and
	Windows native threads.  Used by the cpool example only; we could
	keep this code inline there, but it's really just unimportant
	details.

 Copyright (c) 2008 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the
 CREDITS.txt file in the top directory of the distribution for details.

 This file is part of MySQL++.

 MySQL++ is free software; you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 MySQL++ is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with MySQL++; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 USA
***********************************************************************/

#if !defined(MYSQLPP_THREADS_H)
#define MYSQLPP_THREADS_H

#include <mysql++.h>

#if defined(MYSQLPP_PLATFORM_WINDOWS)
#	define HAVE_THREADS
#	define CALLBACK_SPECIFIER WINAPI
	typedef DWORD thread_return_t;
	typedef LPVOID thread_arg_t;
	static int create_thread(LPTHREAD_START_ROUTINE worker, thread_arg_t arg)
	{
		return CreateThread(0, 0, worker, arg, 0, 0) ? 0 : GetLastError();
	}
	static void sleep(int s) { Sleep(s * 1000); }
#else
#	include "../config.h.in"
#	if defined(HAVE_UNISTD_H)
#		include <unistd.h>
#	endif
#	if defined(HAVE_PTHREAD)
#		define HAVE_THREADS
#		define CALLBACK_SPECIFIER
		typedef void* thread_return_t;
		typedef void* thread_arg_t;
		static int create_thread(thread_return_t(*worker)(thread_arg_t),
				thread_arg_t arg)
		{
			pthread_t pt;
			return pthread_create(&pt, 0, worker, arg);
		}
#	endif
#endif

#endif // !defined(MYSQLPP_THREADS_H)
