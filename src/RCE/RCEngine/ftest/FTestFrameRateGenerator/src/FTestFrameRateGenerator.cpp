// frames_generator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/scoped_ptr.hpp>
#include <AidLib/Strings/tstring.h>

/// Global variables
int g_cols = 16;
int g_rows = 16;
void (*generator)() = NULL;
tstring g_fileName = _T("frames.dat");

void GenerateFramesForTest1()
{
	HANDLE hFile = CreateFile(
		g_fileName.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

// TODO: check handle
	if(INVALID_HANDLE_VALUE == hFile)
		return;

	/// Create shared pointer to file's handle
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spFile(hFile,CloseHandle);


	/// Write number of columns
	DWORD write = 0;
	WriteFile(hFile, &g_cols, 4, &write, NULL); 

// TODO: check

	/// Write number of rows
	WriteFile(hFile, &g_rows, 4, &write, NULL);

// TODO: check

	/// Calculate number of bytes per frame
	int bytesPerFrame = g_cols * g_rows;

	/// Write number of frames
	int framesCount = g_cols * g_rows;
	WriteFile(hFile, &framesCount, 4, &write, NULL);

// TODO: check

	for(int i = 0; i < framesCount; ++i)
	{
		for(int j = 0; j < bytesPerFrame; ++j)
		{
			unsigned char value = 0;
			if(i == j)
				value = 1;
			WriteFile(hFile, &value, 1, &write, NULL);
		}
	}
}

void GenerateFramesForTest2()
{
	HANDLE hFile = CreateFile(
		g_fileName.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

// TODO: check handle
	if(INVALID_HANDLE_VALUE == hFile)
		return;

	/// Create shared pointer to file's handle
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spFile(hFile,CloseHandle);


	/// Write number of columns
	DWORD write = 0;
	WriteFile(hFile, &g_cols, 4, &write, NULL); 

// TODO: check

	/// Write number of rows
	WriteFile(hFile, &g_rows, 4, &write, NULL);

// TODO: check

	/// Calculate number of bytes per frame
	int bytesPerFrame = g_cols * g_rows;

	/// Write number of frames
	int framesCount = g_cols;
	WriteFile(hFile, &framesCount, 4, &write, NULL);

// TODO: check

	for(int i = 0; i < framesCount; ++i)
	{
		for(int j = 0; j < g_rows; ++j)
		{
			for(int k = 0; k < g_cols; ++k)
			{
				unsigned char value = 0;
				if((k <= i) && (j <= i))
					value = 1;
				WriteFile(hFile, &value, 1, &write, NULL);
			}
		}
	}
}

void GenerateFramesForTest3()
{
	HANDLE hFile = CreateFile(
		g_fileName.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

// TODO: check handle
	if(INVALID_HANDLE_VALUE == hFile)
		return;

	/// Create shared pointer to file's handle
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spFile(hFile,CloseHandle);


	/// Write number of columns
	DWORD write = 0;
	WriteFile(hFile, &g_cols, 4, &write, NULL); 

// TODO: check

	/// Write number of rows
	WriteFile(hFile, &g_rows, 4, &write, NULL);

// TODO: check

	/// Calculate number of bytes per frame
	int bytesPerFrame = g_cols * g_rows;

	/// Write number of frames
	int framesCount = g_cols * g_rows;
	WriteFile(hFile, &framesCount, 4, &write, NULL);

// TODO: check

	for(int i = 0; i < framesCount; ++i)
	{
		bool evenFrame = (i%2==0);
		if(evenFrame)
		{
			for(int j = 0; j < bytesPerFrame; ++j)
			{
				unsigned char value = 0;
				if(i == j)
					value = 1;
				WriteFile(hFile, &value, 1, &write, NULL);
			}

		}
		else
		{
			int row = i / g_cols;
			int col = i % g_cols;
			for(int j = 0; j < g_rows; ++j)
			{
				for(int k = 0; k < g_cols; ++k)
				{
					unsigned char value = 0;
					if(((j == row) || (j == row + 1)) && ((k == col) || (k == col - 1)))
						value = 1;
					WriteFile(hFile, &value, 1, &write, NULL);
				}
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	/// Parse command line
	for(int i = 0; i < argc; ++i )
	{
		tstring param(argv[i]);
		tstring paramName = param.substr(0, 3);
		paramName = UpperCase(paramName);
		param.erase(0, 3);
		if(!paramName.compare(_T("-C=")))
		{
			g_cols = atoi(param.c_str());
		}
		else
		{
			if(!paramName.compare(_T("-R=")))
			{
				g_rows = atoi(param.c_str());
			}
			else
			{
				if(!paramName.compare(_T("-F=")))
				{
					g_fileName = param;
				}
				else
				{
					if(!paramName.compare(_T("-T=")))
					{
						int test_number = atoi(param.c_str());
						switch(test_number)
						{
						case 1:
							generator = &GenerateFramesForTest1;
							break;
						case 2:
							generator = &GenerateFramesForTest2;
							break;
						case 3:
							generator = &GenerateFramesForTest3;
							break;
						}
					}
				}
			}
		}

	}
	
	if(!generator)
		generator = &GenerateFramesForTest1;

	(*generator)();
	return 0;
}

