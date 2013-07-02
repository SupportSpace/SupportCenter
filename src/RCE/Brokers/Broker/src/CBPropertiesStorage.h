/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBPropertiesStorage.h
///
///  Command class of ATL OLEDB declaration 
///
///  @author Kirill Solovyov @date 10.05.2008
///
////////////////////////////////////////////////////////////////////////

// CBPropertiesStorage.h : Declaration of the CBPropertiesStorage

#pragma once

// code generated on Saturday, May 10, 2008, 17:49

[
	//#error Security Issue: The connection string may contain a password
	// The connection string below may contain plain text passwords and/or
	// other sensitive information. Please remove the #error after reviewing
	// the connection string for any security related issues. You may want to
	// store the password in some other form or use a different user authentication.
db_source(L"Provider=MSDASQL.1;Persist Security Info=False;Data Source=SupportSpace;Extended Properties=\"DSN=SupportSpace;DriverId=533;FIL=dBase 5.0;MaxBufferSize=2048;PageTimeout=600;\""),
db_command(L" SELECT NAME, VALUE FROM props.dbf WHERE \"NAME\"=?")
]
class CBPropertiesStorage
{
public:

	// In order to fix several issues with some providers, the code below may bind
	// columns in a different order than reported by the provider

	[ db_column(ordinal=L"1", status=L"m_dwNAMEStatus", length=L"m_dwNAMELength") ] TCHAR m_NAME[255];
	// [ db_column(2, status=m_dwVALUEStatus, length=m_dwVALUELength) ] ISequentialStream* m_VALUE;
	[ db_column(ordinal=L"2", status=L"m_dwVALUEStatus", length=L"m_dwVALUELength") ] TCHAR m_VALUE[8000];
	
	/// parameter of command (sql query)
	[ db_param(ordinal=L"1", paramtype=L"DBPARAMIO_INPUT") ] TCHAR m_qNAME[255];

	// The following wizard-generated data members contain status
	// values for the corresponding fields. You
	// can use these values to hold NULL values that the database
	// returns or to hold error information when the compiler returns
	// errors. See Field Status Data Members in Wizard-Generated
	// Accessors in the Visual C++ documentation for more information
	// on using these fields.
	// NOTE: You must initialize these fields before setting/inserting data!

	DBSTATUS m_dwNAMEStatus;
	DBSTATUS m_dwVALUEStatus;

	// The following wizard-generated data members contain length
	// values for the corresponding fields.
	// NOTE: For variable-length columns, you must initialize these
	//       fields before setting/inserting data!

	DBLENGTH m_dwNAMELength;
	DBLENGTH m_dwVALUELength;


	void GetRowsetProperties(CDBPropSet* pPropSet)
	{
		pPropSet->AddProperty(DBPROP_CANFETCHBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_CANSCROLLBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_IRowsetChange, true);//, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_IRowsetUpdate, true);
		pPropSet->AddProperty(DBPROP_UPDATABILITY, DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE);
	}
};


