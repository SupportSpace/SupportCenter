// Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
// Copyright (C) 2002 RealVNC Ltd.  All Rights Reserved.
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
// USA.

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef _WIN32
//#include <winsock.h>
#include <sys/timeb.h>
//#define read(s,b,l) recv(s,(char*)b,l,0)
//#undef errno
//#define errno WSAGetLastError()
#else
#include <unistd.h>
#include <sys/time.h>
#endif

// XXX should use autoconf HAVE_SYS_SELECT_H
#ifdef _AIX
#include <sys/select.h>
#endif

#include "FdInStream.h"
#include "Exception.h"

#include <NWL/Streaming/CAbstractStream.h>

#include <stdexcept>

using namespace rdr;

enum { DEFAULT_BUF_SIZE = 8192,
       MIN_BULK_SIZE = 1024 };

#ifdef _WIN32
static void gettimeofday(struct timeval* tv, void*)
{
  LARGE_INTEGER counts, countsPerSec;
  static double usecPerCount = 0.0;

  if (QueryPerformanceCounter(&counts)) {
    if (usecPerCount == 0.0) {
      QueryPerformanceFrequency(&countsPerSec);
      usecPerCount = 1000000.0 / countsPerSec.QuadPart;
    }

    LONGLONG usecs = (LONGLONG)(counts.QuadPart * usecPerCount);
    tv->tv_usec = (long)(usecs % 1000000);
    tv->tv_sec = (long)(usecs / 1000000);

  } else {
    struct timeb tb;
    ftime(&tb);
    tv->tv_sec = tb.time;
    tv->tv_usec = tb.millitm * 1000;
  }
}
#endif

FdInStream::FdInStream(int fd_, int timeout_, int bufSize_)
  : fd(fd_), timeout(timeout_), blockCallback(0), blockCallbackArg(0),
    timing(false), timeWaitedIn100us(5), timedKbits(0),
    bufSize(bufSize_ ? bufSize_ : DEFAULT_BUF_SIZE), offset(0)
{
	ptr = end = start = new U8[bufSize];

	// sf@2002
	m_fDSMMode = false;
	m_fReadFromNetRectBuf = false;
	m_nNetRectBufOffset = 0;
	m_nReadSize = 0;

	m_nBytesRead = 0; // For stats
}

FdInStream::FdInStream(CAbstractStream* stream_  ):m_stream( stream_ ), timeout(0), blockCallback(0), blockCallbackArg(0),
timing(false), timeWaitedIn100us(5), timedKbits(0),bufSize(DEFAULT_BUF_SIZE), offset(0)
{
	ptr = end = start = new U8[bufSize];
	// sf@2002
	m_fDSMMode = false;
	m_fReadFromNetRectBuf = false;
	m_nNetRectBufOffset = 0;
	m_nReadSize = 0;
	m_nBytesRead = 0; // For stats
}

FdInStream::FdInStream(int fd_, void (*blockCallback_)(void*),
                       void* blockCallbackArg_, int bufSize_)
  : fd(fd_), timeout(0), blockCallback(blockCallback_),
    blockCallbackArg(blockCallbackArg_),
    timing(false), timeWaitedIn100us(5), timedKbits(0),
    bufSize(bufSize_ ? bufSize_ : DEFAULT_BUF_SIZE), offset(0)
{
	ptr = end = start = new U8[bufSize];
	
	// sf@2002
	m_fDSMMode = false;
	m_fReadFromNetRectBuf = false;
	m_nNetRectBufOffset = 0;
	m_nReadSize = 0;
	
}

FdInStream::~FdInStream()
{
  delete [] start;
}


int FdInStream::pos()
{
  return offset + ptr - start;
}

void FdInStream::readBytes(void* data, int length)
{
	// sf@2003 - Seems to fix the ZRLE+DSM bug... 
	if (!m_fDSMMode)
	{
		if (length < MIN_BULK_SIZE)
		{
			struct timeval before, after;
			if (timing)
				gettimeofday(&before, 0);

			int n = 0;
			InStream::readBytes(data, length, n);

			if (timing && n > 512)
			{
				gettimeofday(&after, 0);
				int newTimeWaited = ((after.tv_sec - before.tv_sec) * 10000 +
									(after.tv_usec - before.tv_usec) / 100);
				int newKbits = n * 8 / 1000;

				// limit rate to between 10kbit/s and 40Mbit/s
				if (newTimeWaited > newKbits*1000) newTimeWaited = newKbits*1000;
				if (newTimeWaited < newKbits/4)    newTimeWaited = newKbits/4;

				timeWaitedIn100us += newTimeWaited;
				timedKbits += newKbits;
			}
			return;
		}
	}
	
	U8* dataPtr = (U8*)data;
	
	int	n = end - ptr;
	if (n > length) n = length;
	
	memcpy(dataPtr, ptr, n);
	dataPtr += n;
	length -= n;
	ptr += n;

	//////////////////
	//while(!m_stream->HasInData())
		//Sleep(1);
	//m_stream->Receive((char*) dataPtr, length);
	//return;
	/////////////////

	struct timeval before, after;
	if (timing)
		gettimeofday(&before, 0);

	m_stream->Receive(reinterpret_cast<char*>(dataPtr), length);

	if (timing && length > 1024)
	{
		gettimeofday(&after, 0);
		int newTimeWaited = ((after.tv_sec - before.tv_sec) * 10000 +
							(after.tv_usec - before.tv_usec) / 100);
		int newKbits = length * 8 / 1000;

		// limit rate to between 10kbit/s and 40Mbit/s
		if (newTimeWaited > newKbits*1000) newTimeWaited = newKbits*1000;
		if (newTimeWaited < newKbits/4)    newTimeWaited = newKbits/4;

		timeWaitedIn100us += newTimeWaited;
		timedKbits += newKbits;
	}

	/*while (length > 0) 
	{
		n = readWithTimeoutOrCallback(dataPtr, length);
		dataPtr += n;
		length -= n;
		offset += n;
		//if (length) 
			//Sleep( 1 );
	}*/
}


int FdInStream::overrun(int itemSize, int nItems)
{
  if (itemSize > bufSize)
    throw Exception("FdInStream overrun: max itemSize exceeded");

  if (end - ptr != 0)
    memmove(start, ptr, end - ptr);

  offset += ptr - start;
  end -= ptr - start;
  ptr = start;

  while (end < start + itemSize) {
    int n = readWithTimeoutOrCallback((U8*)end,  start + bufSize - end);
    end += n;
  }

  if (itemSize * nItems > end - ptr)
    nItems = (end - ptr) / itemSize;

  return nItems;
}


int FdInStream::Check_if_buffer_has_data()
{
 InStream::setptr(InStream::getend());
 return checkReadable(fd, 500);
}

int FdInStream::checkReadable(int fd, int timeout)
{
  while (true) {
    fd_set rfds;
    struct timeval tv;
    
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    int n = select(fd+1, &rfds, 0, 0, &tv);
    if (n != -1 || errno != EINTR)
      return n;
    fprintf(stderr,"select returned EINTR\n");
  }
}

inline int FdInStream::readWithTimeoutOrCallback(void* buf, int len)
{
	//Sleep( 1 );
	/*struct timeval before, after;
	if (timing)
		gettimeofday(&before, 0);*/

	int n = m_stream->GetInBuffer( (char*)buf , len );
	if (!n)
		Sleep(1);
	//int n = len;
	//m_stream->Receive( reinterpret_cast<char*>(buf), len );

	/*if (timing && n > 1024)
	{
		gettimeofday(&after, 0);
		int newTimeWaited = ((after.tv_sec - before.tv_sec) * 10000 +
							(after.tv_usec - before.tv_usec) / 100);
		int newKbits = n * 8 / 1000;

		// limit rate to between 10kbit/s and 40Mbit/s
		if (newTimeWaited > newKbits*1000) newTimeWaited = newKbits*1000;
		if (newTimeWaited < newKbits/4)    newTimeWaited = newKbits/4;

		timeWaitedIn100us += newTimeWaited;
		timedKbits += newKbits;
	}*/
	return n;
}

void FdInStream::startTiming()
{
  timing = true;

  // Carry over up to 1s worth of previous rate for smoothing.

  if (timeWaitedIn100us > 10000) {
    timedKbits = timedKbits * 10000 / timeWaitedIn100us;
    timeWaitedIn100us = 10000;
  }
}

void FdInStream::stopTiming()
{
  timing = false; 
  if (timeWaitedIn100us < timedKbits/2)
    timeWaitedIn100us = timedKbits/2; // upper limit 20Mbit/s
}

unsigned int FdInStream::kbitsPerSecond()
{
  // The following calculation will overflow 32-bit arithmetic if we have
  // received more than about 50Mbytes (400Mbits) since we started timing, so
  // it should be OK for a single RFB update.

  return timedKbits * 10000 / timeWaitedIn100us;
}


//
// sf@2002 - DSMPlugin - This hack is necessary because
// the ZRLE encoder/decoder does not use the SendExact/ReadExact functions
// like ALL the others encoders...
// 
void FdInStream::SetReadFromMemoryBuffer(int nReadSize, char* pMemBuffer)
{
	m_nReadSize = nReadSize; // Nb of bytes to read from buffer instead of socket
	m_pNetRectBuf = pMemBuffer; // The memory buffer containing restored data
	m_nNetRectBufOffset = 0;   // Initial offset

	m_fReadFromNetRectBuf = true; // Order to read from the buffer
}



