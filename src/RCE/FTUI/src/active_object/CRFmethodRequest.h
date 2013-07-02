/////////////////////////////////////////////////////////////////////////
///
///  CRFmethodRequest.h
///
///  implementation of Active Object pattern
///
///
///  @author Dmiry S. Golub @date 2/6/2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "IMethodRequest.h"


///  class for list drives in remote machine
class CListDrivesMR :public IMethodRequest
{
public:
	typedef boost::function<void(TDriveInfo&, const s_param& )> TCallback;
	/// .ctor
	CListDrivesMR(CFileAccessClient* servant, TCallback callback);
	/// .dtor
	~CListDrivesMR(void);
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	void callMethod();
private:
	/// pointer to callback function which will be called
	TCallback m_futureResult;
	/// method param store \sa callMethod
	s_param m_par;
};

///  This is a method request for list files in remote machine
class CListFilesMR : public IMethodRequest
{
public:
	typedef boost::function<void(TFileInfo&)> TCallback;
	/// .ctor
	CListFilesMR(CFileAccessClient*,TCallback,const s_param& par);
	/// .dtor
	~CListFilesMR();
	/// virtual method which makes all work
	void callMethod();
private:
	/// data for callMethod
	s_param   m_param;
	/// callback method which will be called after finishing callMethod
	TCallback m_futureResult;
};

///  This is a method request for retrieving files from remote machine
class CRetrieveFileMR : public IMethodRequest
{
public:
	typedef boost::function<void(s_param&)> TCallback;
	/// .ctor
	CRetrieveFileMR( CFileAccessClient*, TCallback , const s_param&  );
	/// .dtor
	virtual ~CRetrieveFileMR();
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	void callMethod();
private:
	/// data for callMethod
	s_param m_par;
	/// pointer to callback function which will be called
	TCallback	m_futureResult;
};

///  This is a method request for sending files to remote machine
class CSendFileMR : public IMethodRequest
{
public:
	typedef boost::function<void(s_param&)> TCallback;
	/// .ctor
	CSendFileMR( CFileAccessClient*, TCallback , const s_param&  );
	/// .dtor
	virtual ~CSendFileMR();
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	void callMethod();
private:
	/// data for callMethod
	s_param m_par;
	/// pointer to callback function which will be called
	TCallback	m_futureResult;
};

///  This is a method request for creating directory on remote machine
class CCreateDirectoryMR : public IMethodRequest
{
public:

	typedef boost::function<void(s_param)> TCallback;
	/// .ctor
	CCreateDirectoryMR(CFileAccessClient*,TCallback,const s_param& par);
	/// .dtor
	~CCreateDirectoryMR();
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	void callMethod();
private:
	/// pointer to callback function which will be called
	TCallback m_futureResult;
	/// data for callMethod
	s_param m_par;
};

///  This is a method request for removing directory on remote machine
class CRemoveDirectoryMR : public IMethodRequest
{
public:

	typedef boost::function<void(s_param&)> TCallback;
	/// .ctor
	CRemoveDirectoryMR(CFileAccessClient*,TCallback,const s_param& par);
	/// .dtor
	~CRemoveDirectoryMR();
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	void callMethod();
private:
	/// pointer to callback function which will be called
	TCallback m_futureResult;
	/// data for callMethod
	s_param m_par;
};

///  This is a method request for raname file or directory on remote machine
class CRenameMR : public IMethodRequest
{
public:

	typedef boost::function<void(s_param&)> TCallback;
	/// .ctor
	CRenameMR(CFileAccessClient*,TCallback,const s_param& par);
	/// .dtor
	~CRenameMR();
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	void callMethod();
private:
	/// pointer to callback function which will be called
	TCallback m_futureResult;
	/// data for callMethod
	s_param m_par;
};

