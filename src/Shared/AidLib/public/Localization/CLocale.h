/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLocale.h
///
///  current locale object
///
///  @author "Archer Software" Sogin M. @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <map>
#include <AidLib/strings/tstring.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/Loki/Singleton.h>

/// less for tstring
struct tstring_less : public std::binary_function<tstring, tstring, bool>
{	
	// functor for operator<
	bool operator()(const tstring& _Left, const tstring& _Right) const
	{	// apply operator< to operands
		return _Left.compare(_Right) < 0;
	}
};


///Locale class
class AIDLIB_API CLocale
{
protected:
	/// localized strings
	std::map<tstring,tstring,tstring_less> m_strings;
public:
	/// initializes object instance
	CLocale(void);
	/// destroys object instance
	virtual ~CLocale(void);

	/// returns string by it's english variant. adds string to map if no such one
	/// @param id string id
	/// @return localized string
	tstring GetString(const tstring& str);

	/// Set locale string value by key (english variant)
	/// @param key key (english string variant)
	/// @param value localized string value
	void SetString(const tstring& key, const tstring& value);
};

/// Should be used to access CLocale as single instance
#define LOCALE_INSTANCE Loki::SingletonHolder<CLocale, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()

#define MLocalize(x) (LOCALE_INSTANCE.GetString(x))
#define MSetString(key, value) (LOCALE_INSTANCE.SetString(key, value))