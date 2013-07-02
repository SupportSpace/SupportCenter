//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CCommandProxy.h
///
///  Declares CCommandProxy class
///  Uses in widgets for communication with CCommandManager
///  
///  @author Alexander Novak @date 22.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include "CommandDefinitions.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCommandManager;
class CCommandProxy;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Base class for command managers
class CAbstractCommandManager 
	:	public CSkinnedElement
{
	friend class CCommandProxy;
protected:
	virtual void ExecuteCommand(const CCommandProxy* sender, EWidgetCommand command, const void* commandData) = NULL;
public:
	CAbstractCommandManager(CSkinnedElement *parent = NULL, 
					boost::shared_ptr<CSkinsImageList> skinsImageList = boost::shared_ptr<CSkinsImageList>(reinterpret_cast<CSkinsImageList*>(NULL)))
					: CSkinnedElement(parent, skinsImageList)
	{
	}
};

class CCommandProxy
{
	boost::shared_ptr<CAbstractCommandManager> m_commandManager;
public:
	CCommandProxy(boost::shared_ptr<CAbstractCommandManager> manager);
	virtual void DispatchCommand(EWidgetCommand command, const void* commandData = NULL);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
