/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVersionReader.h
///
///  class for retriving msi components versions
///
///  @author "Archer Software" Sogin M. @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include <msi.h>
#pragma comment(lib, "msi.lib")

// Length of GUID
#define MSI_GUID_LEN 38

/// Version reader for msi products
class CVersionReader
{
private:
	/// product code
	tstring m_productCode;
	/// map of installstate codes strings
	std::map<INSTALLSTATE,tstring> m_msiStatesMap;

	/// returns install state string by state value
	/// @param state install state value
	tstring InstallState2String(INSTALLSTATE state) const;

	/// returns ProductCode by it's upgrade code, if such not found, or
	/// there're more than one product, throws an exception
	/// @param upgradeCode upgrade code
	static tstring GetProductCodeByUpgradeCode(const tstring& upgradeCode);

	/// returns ProductCode by it's component GUID, if such not found,throws an exception
	/// @param componentGUID GUID of component
	static tstring GetProductCodeByComponentGUID(const tstring& componentGUID);

public:

	/// Key types for product search
	typedef enum _ESearchKeyType
	{
		SK_PRODUCT_CODE		= 0,	// Search product by its code
		SK_UPGRAGE_CODE		= 1,	// Search product by upgrade code
		SK_COMPONENT_GUID	= 2		// Search product by component GUID
	} ESearchKeyType;

	CVersionReader(	const tstring& productKey, const ESearchKeyType keyType = SK_PRODUCT_CODE );

	virtual ~CVersionReader();

	/// Returns version for main component fail
	/// if there isn't file or component - exception is thrown
	/// @param componentCode component GUID
	tstring GetComponentVersion(const tstring& componentCode) const;
};
