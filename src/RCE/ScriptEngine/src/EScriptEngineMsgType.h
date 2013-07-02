/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  EScriptEngineMsgType.h
///
///  Declares EScriptEngineMsgType enumeration for types of protocol messages
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

///  EScriptEngineMsgType enumeration for types of protocol messages
enum EScriptEngineMsgType
{
	semtDeploy			= 0,	/// Corresponds DeployFile method 
								/// Message should contain file name and content of script file
	semtExecFile		= 1,	/// Corresponds IScriptEngineClient->InvokeFile method
								/// Message should contain array with language, fileName, procedureName, parameters
	semtExecCode		= 2,	/// Corresponds IScriptEngineClient->InvokeCode method
								/// Message should contain array with language, code, procedureName, parameters
	semtTimeout			= 3,	/// Corresponds IScriptEngineClient->SetTimeOut method
								/// Message should contain timeout value
	semtOnError			= 4,	/// Error occurs at script execution
								/// Message should contain error string
	semtOnTimeout		= 5,	/// Timeout expired at script execution
								/// Message shouldn't contain data
	semtOnSuccess		= 6,	/// Script executed successfully
								/// Message should contain array with results
	semtOnDeploy		= 7,	/// File deploying result
								/// Message should contain array with deploy result and error description
	semtSetParams		= 8,	/// Result parameters
								/// Message should contain array with parameters and JSON object
	semtGetFiles		= 9,	/// Corresponds IScriptEngineClient->GetRemoteFiles method 
								/// Message should contain array with names of files
	semtOnGetFiles		= 10	/// Remote files received
								/// Message should contain result, error scring and array with files
};
