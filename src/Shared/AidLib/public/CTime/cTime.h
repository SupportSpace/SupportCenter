//===========================================================================
// Archer Software.
//                                   cTime.h
//
//---------------------------------------------------------------------------
// classes for working with 64bit time values
//---------------------------------------------------------------------------
//
// Version : 01.00
// By      : Max Sogin
// Date    : 8/1/05 01:49:03 PM
//===========================================================================

#ifndef	CTIME_H
#define	CTIME_H
//---------------------------------------------------------------------------

#include <windows.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/strings/tstring.h>
#include <AidLib/CException/CException.h>

#ifdef OLETIME		//----------------Support for OLE time format-----------

#ifdef __AFX_H__
	#include <AFXDISP.H>
#else
	#include <Oleauto.h>
#endif //__AFX_H__
	#pragma comment (lib,"Oleaut32.lib")
#endif //OLETIME

//Defines format of strings to use (short or long)
#define SHORT_DATE_FORMAT 1

//===========================================================================
//
// @{CLS}                                cDate
//
//---------------------------------------------------------------------------
// Description : Class for working Date types
//===========================================================================
class /*AIDLIB_API*/ cDate
{
private:
	//Here we storing real date
	FILETIME FileTime;
public:
	//Creates cDate from String values
	cDate(const tstring Str);

	//Creates object from FILETIME
	cDate(FILETIME _FileTime) : FileTime(_FileTime) {};
	//Creates object from SYSTIME
	cDate(SYSTEMTIME SysTime)
	{
		if (!SystemTimeToFileTime(&SysTime,&FileTime)) 
		{
			*this=cDate();
			Log.WinError(_ERROR_,_T("cDate: failed to SystemTimeToFileTime"));
		}
	}

	//Creates object from ULARGE_INTEGER
	cDate(ULARGE_INTEGER Time) { FileTime.dwLowDateTime = Time.LowPart; FileTime.dwHighDateTime = Time.HighPart; };

	//Creates object from LARGE_INTEGER
	cDate(LARGE_INTEGER Time) { FileTime.dwLowDateTime = Time.LowPart; FileTime.dwHighDateTime = Time.HighPart; };

	//Creates object with invalid date and time
	cDate() {FileTime.dwLowDateTime=0;	FileTime.dwHighDateTime=0;};

#ifdef OLETIME		//----------------Support for OLE time format-----------
	cDate(DATE D) {	SYSTEMTIME st;	if (VariantTimeToSystemTime(D,&st)) *this=cDate(st); else *this=cDate(); };

#endif //OLETIME

	//Returns true, if date and time is invalid
	bool Invalid() const { return !(FileTime.dwLowDateTime || FileTime.dwHighDateTime); };

	//Conversion to FILETIME
	operator FILETIME() const {return FileTime;};

	//Conversion to SYSTEMTIME
	operator SYSTEMTIME() const
	{
		SYSTEMTIME SysTime;
		if (!FileTimeToSystemTime(&FileTime,&SysTime)) 
		{
			Log.WinError(_ERROR_,_T("cDate: failed to FileTimeToSystemTime"));
		}
		return SysTime;
	};

	//Conversion to ULARGE_INTEGER
	operator ULARGE_INTEGER() const
	{
		ULARGE_INTEGER Result;
		Result.LowPart=FileTime.dwLowDateTime;
		Result.HighPart=FileTime.dwHighDateTime;
		return Result;
	}

	//Conversion to ULARGE_INTEGER
	operator LARGE_INTEGER() const
	{
		LARGE_INTEGER Result;
		Result.LowPart=FileTime.dwLowDateTime;
		Result.HighPart=FileTime.dwHighDateTime;
		return Result;
	}

	bool operator <= (const cDate& Date) const
	{
		return (*this < Date) || (*this == Date);
	}

	//Comparision > operator
	bool operator > (const cDate& Date) const 
	{
		return 1 == CompareFileTime(&FileTime,&Date.FileTime);
	};

	//Comparision < operator
	bool operator < (const cDate& Date) const 
	{
		return -1 == CompareFileTime(&FileTime,&Date.FileTime);
	};


	//Equal == operator
	bool operator == (const cDate& Date) const
	{
		return 0 == CompareFileTime(&FileTime,&Date.FileTime);
	};

		
	//===========================================================================
	//
	// @{FUN}                               GetNow()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Sets internal value to current date time
	// Return type	: cDate (*this)
	// Errors		: 
	//===========================================================================
	cDate GetNow()
	{
		SYSTEMTIME SysTime;
		GetLocalTime(&SysTime);
		return (*this)=cDate(SysTime);
	}

	cDate GetNever()
	{
		cDate date;
		date.AddMilliSecs( -1 );
		return (*this)=date;
	}

	//===========================================================================
	//
	// @{FUN}                            AddMilliSecs()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Adds count millisecond to datetime value
	// Return type	: cDate 
	//   Argument	: const int MsCount - [in]
	// Errors		: 
	//===========================================================================
	cDate AddMilliSecs(const LONGLONG MsCount)
	{
		LARGE_INTEGER li(*this);
		li.QuadPart += MsCount*10000;
		return (*this) = cDate(li);
	}

	//===========================================================================
	//
	// @{FUN}                         AddCountMonth()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Adds count of month
	// Return type	: 
	//   Argument	: const unsigned int MonthCount - [in]
	// Errors		: 
	//===========================================================================
	cDate AddCountMonth(const int MonthCount)
	{
		SYSTEMTIME SysTime = (SYSTEMTIME)(*this);
		
		unsigned int dYear;

		SysTime.wMonth += MonthCount;

		dYear = (SysTime.wMonth - 1) / 12;
		SysTime.wYear += dYear;

		SysTime.wMonth = (SysTime.wMonth - 1) % 12 + 1;

		return (*this) = cDate(SysTime);
	}

	//===========================================================================
	//
	// @{FUN}                            AddCountDays()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Adds count of days to date
	// Return type	: cDate 
	//   Argument	: const int DaysCount - [in]
	// Errors		: 
	//===========================================================================
	cDate AddCountDays(const int DaysCount)
	{
		ULARGE_INTEGER ULTime = (ULARGE_INTEGER)(*this);

		ULTime.QuadPart += ((LONGLONG)DaysCount) * 10000000 * 60 * 60 * 24; 

		return (*this)=cDate(ULTime);
	};


	//===========================================================================
	//
	// @{FUN}                            AddCountWeeks()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Adds count of weeks to date
	// Return type	: cDate 
	//   Argument	: const int WeeksCount - [in]
	// Errors		: 
	//===========================================================================
	cDate AddCountWeeks(const int WeeksCount)
	{
		ULARGE_INTEGER ULTime = (ULARGE_INTEGER)(*this);
		ULTime.QuadPart += ((LONGLONG)WeeksCount) * 10000000 * 60 * 60 * 24 * 7;
		return (*this)=cDate(ULTime);
	}

	//===========================================================================
	//
	// @{FUN}                            SetDayOfWeek()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Sets the day of the week (it cat both increasa and reduce time,
	//				: dependingon difference)
	// Return type	: cDate 
	//   Argument	: const unsigned int DayOfWeek - [in]
	// Errors		: 
	//===========================================================================
	cDate SetDayOfWeek(const unsigned int DayOfWeek)
	{
		SYSTEMTIME SysTime = (SYSTEMTIME)(*this);
		int Diff = DayOfWeek - SysTime.wDayOfWeek;
		ULARGE_INTEGER ULTime = (ULARGE_INTEGER)(*this);
		ULTime.QuadPart += ((LONGLONG)Diff) * 10000000 * 60 * 60 * 24;
		return (*this)=cDate(ULTime);
	}


	//===========================================================================
	//
	// @{FUN}                            SetMonthOfYear()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Sets the day of the month
	// Return type	: cDate 
	//   Argument	: const unsigned int MonthOfYear - [in]
	// Errors		: if no such daty in month InvalidDate returned
	//===========================================================================
	cDate SetMonthOfYear(const unsigned int MonthOfYear, const bool HideErrors=false)
	{
		SYSTEMTIME SysTime = (SYSTEMTIME)(*this);
		SysTime.wMonth = MonthOfYear;
		if (!HideErrors)
			return (*this)=cDate(SysTime);
		else
		{
			if (!SystemTimeToFileTime(&SysTime,&FileTime)) 
			{
				return (*this)=cDate();
			} else
			{
				return (*this);
			}
		}
	}

	//===========================================================================
	//
	// @{FUN}                            SetDayOfMonth()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Sets the day of the month
	// Return type	: cDate 
	//   Argument	: const unsigned int DayOfMonth - [in]
	// Errors		: if no such daty in month InvalidDate returned
	//===========================================================================
	cDate SetDayOfMonth(const unsigned int DayOfMonth, const bool HideErrors=false)
	{
		SYSTEMTIME SysTime = (SYSTEMTIME)(*this);
		SysTime.wDay = DayOfMonth;
		if (!HideErrors)
			return (*this)=cDate(SysTime);
		else
		{
			if (!SystemTimeToFileTime(&SysTime,&FileTime)) 
			{
				return (*this)=cDate();
			} else
			{
				return (*this);
			}
		}
	}

	//===========================================================================
	//
	// @{FUN}                        int GetMonthNumByName()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Returns month number by its name. Current user locale used. If name
	//				: notvalid 1 returned. 1 is equal to yanuary
	// Return type	: static unsigned 
	//   Argument	: const tstring MonthName - [in]
	// Errors		: on critical errors exception thrown
	//===========================================================================
	static unsigned int GetMonthNumByName(const tstring MonthName) ;


	//===========================================================================
	//
	// @{FUN}                        int GetMonthNumByShortName()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Returns month number by its name. Current user locale used. If name
	//				: notvalid 1 returned. 1 is equal to yanuary
	// Return type	: static unsigned 
	//   Argument	: const tstring MonthName - [in]
	// Errors		: on critical errors exception thrown
	//===========================================================================
	static unsigned int GetMonthNumByShortName(const tstring MonthName) ;


	//===========================================================================
	//
	// @{FUN}                        int GetDayNumByName()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Returns day number by its name. Current user locale used. If name
	//				: notvalid 1 returned. 1 is equal to monday
	// Return type	: static unsigned 
	//   Argument	: const tstring MonthName - [in]
	// Errors		: on critical errors exception thrown
	//===========================================================================
	static unsigned int GetDayNumByName(const tstring DayName) ;


	//===========================================================================
	//
	// @{FUN}                        cDate operator -(const &cDate)
	//
	//---------------------------------------------------------------------------
	// Effects 		: Substracts date from object
	// Return type	: cDate
	//   Argument	: const &cDate date - [in]
	// Errors		: on critical errors exception thrown
	//===========================================================================
	cDate operator -(const cDate& date)
	{
	TRY_CATCH
		ULARGE_INTEGER li(*this);
		li.QuadPart -= ULARGE_INTEGER(date).QuadPart;		
		*this = li;
		return *this;
	CATCH_THROW("cDate::operator -")
	};

	//===========================================================================
	//
	// @{FUN}                        cDate operator +(const &cDate)
	//
	//---------------------------------------------------------------------------
	// Effects 		: Adds date to object
	// Return type	: cDate
	//   Argument	: const &cDate date - [in]
	// Errors		: on critical errors exception thrown
	//===========================================================================
	cDate operator +(const cDate& date)
	{
	TRY_CATCH
		ULARGE_INTEGER li(*this);
		li.QuadPart += ULARGE_INTEGER(date).QuadPart;		
		*this = li;
		return *this;
	CATCH_THROW("cDate::operator +")
	};

	//===========================================================================
	//
	// @{FUN}                        tstring FormatTime(const tstring& format = tstring())
	//
	//---------------------------------------------------------------------------
	// Effects 		: Formats time to string
	// Return type	: tstring 
	//   Argument	: const const tstring& format - [in] format string
	// Errors		: on critical errors exception thrown
	//===========================================================================
	tstring FormatTime(const tstring& format = tstring())
	{
	TRY_CATCH
		
		tstring Result;
		SYSTEMTIME SysTime = (SYSTEMTIME)(*this);
		TCHAR Buf[MAX_PATH];
		if (!GetTimeFormat(LOCALE_USER_DEFAULT,0,&SysTime,format.empty()?NULL:format.c_str(),Buf,MAX_PATH))
			Result = _T("Invalid time");
		else
			Result = Buf;

		return Result;

	CATCH_THROW("cTime::Format")
	}
};


#define cTime cDate;

//-------------------------------------------------------------------------------------
#endif // CTIME_H

