/////////////////////////////////////////////////////////////////////////
///
///  IMethodRequest.h
///
///  Base class for all Active Object's operations
///
///
///  @author Dmiry S. Golub @date 2/6/2007
///
////////////////////////////////////////////////////////////////////////

#if !defined(EA_989780E5_F67C_4dab_9996_86FB238CCB15__INCLUDED_)
#define EA_989780E5_F67C_4dab_9996_86FB238CCB15__INCLUDED_

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <FTUI/FileTransfer/CFileAccessClient.h>

/// parametrs store for all operation
struct s_param
{
	/// local name of the file ( source or destination )
	tstring localName;
	/// remote name of the file  ( source or destination )
	tstring remoteName;
	/// user permissions
	bool permission;
	/// return code of the function
	ResponseCode ret_code;
	/// operation which was made
	TransferOpearation operation;
};


///  Base class for all Active Object's operations . It's a pure abstract class
class IMethodRequest
{
protected:
	/// pointer to CFileAccessClient class which is a servent according  Active Object pattern
	CFileAccessClient* m_pServant;
public:
	/// .ctor
	IMethodRequest( CFileAccessClient* s ):m_pServant(s)
	{
	}
	/// .dtor
	virtual ~IMethodRequest()=0
	{
	};
	/// virtual methos which should be implemented in inhatited classes
	/// \sa CScheduler::Dispatch()
	virtual void callMethod() =0;
};
#endif // !defined(EA_989780E5_F67C_4dab_9996_86FB238CCB15__INCLUDED_)
