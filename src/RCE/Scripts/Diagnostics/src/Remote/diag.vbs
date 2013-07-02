' Globals
Dim fso, WshShell, sScriptName, isVista, sXpathQuery, sCategories, namedJson, sCategoryAvailible, OSystem  ' commonly used objects
Dim bUseWMI

bUseWMI = true ' default may be false. make it true not to use WMI. MWI will not work in .html test page only in .hta
Set fso = CreateObject("Scripting.FileSystemObject")
Set wshShell = CreateObject("WScript.Shell")

'----------------------------------------------------------------------------------------
'
'	BuildJsonFileByDiagCategoryLocal
'
'   Wrapper call to BuildJsonFileByDiagCategory. May be called to tretieve local data
'   Example: >cscript diag.vbs or from local html page 
'----------------------------------------------------------------------------------------
function BuildJsonFileByDiagCategoryLocal(sSubcategory, isRunningFirstTime)

	Dim	sJsonTable

	sJsonTable = BuildJsonFileByDiagCategory(sSubcategory, isRunningFirstTime)
	BuildJsonFileByDiagCategoryLocal = sJsonTable

End function
'----------------------------------------------------------------------------------------
'
'	BuildJsonFileByDiagCategoryRemote
'
'   Wrapper call to BuildJsonFileByDiagCategory. May be called via ScriptEngine
'   Note. ScriptEngineHost.SetReturnParameters must be used instead of regular function return
'----------------------------------------------------------------------------------------
Sub BuildJsonFileByDiagCategoryRemote(sSubcategory, isRunningFirstTime)

	Dim sJsonTable

	sJsonTable = BuildJsonFileByDiagCategory(sSubcategory, isRunningFirstTime)
	ScriptEngineHost.SetReturnParameters sJsonTable, "SomeObject"

End Sub
'----------------------------------------------------------------------------------------
'
'	BuildJsonFileByDiagCategory
'
'   Main function that do msinfo job
'   parameters:
'   sSubcategory - string indicates categoryID
'   isRunningFirstTime - boolean true if is RunningFirst time by script
'----------------------------------------------------------------------------------------
function BuildJsonFileByDiagCategory(sSubcategory, isRunningFirstTime)

	Dim sWorkingDirectory, sXmlOutputFileName, sJsonFileName
	Dim sLangId, isSupported 
	
	namedJson = 0
	sCategoryAvailible = true

	sWorkingDirectory = wshShell.CurrentDirectory + "\"
	LogMsg 1, "BuildJsonFileByDiagCategory sWorkingDirectory:" + sWorkingDirectory

    ' check if script run on vista machine 
	' Dim isRunningFirstTime
	isVista = IsOsVista() 

	' prepeare sCategories, sXmlOutputFileName, sXpathQuery, sJsonFileName
	sCategories = "+" + sSubcategory
	sXmlOutputFileName = sSubcategory + ".nfo"
	sJsonFileName = sSubcategory + ".txt"

	'
	' "System Summary" is always retrieved using WMI...
	'
	if sSubcategory = "SystemSummary" and bUseWMI = true then
		LogMsg 1, "SystemSummary is always retrieved using WMI..."
		sJsonTable = GetSystemInfoWithWMIEx()
		LogMsg 1, "GetSystemInfoWithWMIEx returned:" & sJsonTable
		BuildJsonFileByDiagCategory = sJsonTable
		Exit Function 
	else 
	end if 

	' for each categoryID will be separate query for search in xml file
	GetQueryByCategoryID(sSubcategory)

	'
	' languageId handling block
	' 1) GetOSLanguageID
	sLangId = GetOSLanguageID()
	LogMsg 1, "GetOSLanguageID returned:" & sLangId

	' 2) Check if isGetOSLanguageIDSupported return true
	isSupported = isGetOSLanguageIDSupported(sLangId)
	LogMsg 1, "isGetOSLanguageIDSupported returned:" & isSupported

	' 3) If OS language is not supported then return immidiatly with error code "CantRetrieveInfoFromNonEnglishOS"
	if isSupported=false then 
		sJsonTable = "CantRetrieveInfoFromNonEnglishOS"
		LogMsg 1, "CantRetrieveInfoFromNonEnglishOS (expect SystemSummary) retuned by BuildJsonFileByDiagCategory function"
		'ScriptEngineHost.SetReturnParameters sJsonTable, "SomeObject"
		BuildJsonFileByDiagCategory = sJsonTable
		Exit Function
	else 
	end if 

	LogMsg 1, "isRunningFirstTime:" + isRunningFirstTime

   ' 1) run system info with different approach for vista
	if isVista=true then 
		LogMsg 1, "OSystem is VISTA"
		sXmlOutputFileName = "Summary.nfo"

		if isRunningFirstTime = "1" then 
			Call GetSysInfo(sWorkingDirectory, sXmlOutputFileName, sCategories)  'work fine on XP and Vista as well
		else
			LogMsg 1, "GetSysInfo will not started. Use retrieved data" 
		end if

	else
		LogMsg 1, "OSystem is NOT VISTA" 
		Call GetSysInfo(sWorkingDirectory, sXmlOutputFileName, sCategories)
 	end if 
   
	' 2) convert xml to json
	LogMsg 1, "convert xml to json state" 

   ' generating Coulmn row here
    if namedJson = 1 then 
		LogMsg 1, "convert xml to json state. namedJson is true" 

		if isVista=true then 
			LogMsg 1, "Starting method Xml_Category2JsonObject_Vista" 
			'sXpathQuery = "//MsInfo/Category[@name='System Summary']/Data"
			'sJsonTable = XmlCategory2JsonObjectVista(sSubcategory, sWorkingDirectory, sXmlOutputFileName, sXpathQuery)
		else
		   LogMsg 1, "Next step 2 without Vista" 
		   sJsonTable = XmlCategory2JsonObject(sSubcategory, sWorkingDirectory, sXmlOutputFileName, sXpathQuery)
		end if 

	else
		LogMsg 1, "Before call to XmlCategory2Json sWorkingDirectory is:" + sWorkingDirectory

		'if category is not availible then we must 
		if sCategoryAvailible=true then
			sJsonTable = XmlCategory2Json(sSubcategory, sWorkingDirectory, sXmlOutputFileName, sXpathQuery)
		else
			'for some not availible categories will retrieve with MWI...
			select case sSubcategory
				'case "IEsummary"
					'sJsonTable = MicrosoftIE_Summary()
			    case Else
					sJsonTable = "NotAvailibleOnVista"
			end select

		end if
	end if

	LogMsg 1, "BuildJsonFileByDiagCategory return value" 
	'3) save json to file
	'Call WriteToFile(sWorkingDirectory, sJsonFileName, sJsonTable)
	BuildJsonFileByDiagCategory = sJsonTable

End Function

'----------------------------------------------------------------------------------------
'
'	XmlCategory2Json
'
'   Convert XML node from specified in strQuery category to Json table object
'----------------------------------------------------------------------------------------
Function XmlCategory2Json(sSubcategory, sWorkingDirectory, sInputXmlFileName, strQuery)

	Dim sXmlFullFileName
	sXmlFullFileName = sWorkingDirectory + sInputXmlFileName

	LogMsg 1, "XmlCategory2Json parsing started step 1"
	LogMsg 1, "sSubcategory:" +  sSubcategory 
	LogMsg 1, "sXmlFullFileName:" +  sXmlFullFileName 
	LogMsg 1, "strQuery:" +  strQuery 

	Set xmlDoc = CreateObject( "Microsoft.XMLDOM" )
	xmlDoc.Async = "False"
	xmlDoc.Load( sXmlFullFileName )
	'Set colNodes = xmlDoc.selectNodes( strQuery )  ' we will parse per category. for each category separate json file
	Set objNode = xmlDoc.selectSingleNode( strQuery )

	LogMsg 1, "XmlCategory2Json parsing step 2"

	if objNode is Nothing then 
	  	LogMsg 1, "XML objNode is nothing!!!!!!!!Error!!!!!!!!!!!!!!!!!!!!!!!!"
		XmlCategory2Json = "CantRetrieveInfoUnknownReason"
		Exit Function
    else
	  	LogMsg 1, "XML objNode is NOT nothing" 
    end if

' For Each objNode in colNodes

	Dim objChildNodes, strNode 
	Set objChildNodes = objNode.childNodes
	Dim nCount
	DIM sJsonObjRow
	DIM sJsonObjColumns 
	DIM sJsonTable
	DIM sText

	sJsonObjRow = sJsonObjRow + "["
	sJsonObjColumns = sJsonObjColumns + "["

	LogMsg 1, "XmlCategory2Json parsing step 3"

	if objChildNodes is Nothing then 
	  	LogMsg 1, "objChildNodes is nothing!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
    else
	  	LogMsg 1, "objChildNodes is NOT nothing" 
    end if

	For Each strNode In objChildNodes 
   	    nCount = nCount + 1

		'LogMsg 1, "XmlCategory2Json parsing step 4"

 	    'sJsonObjRow = sJsonObjRow + "{'myId':" & "'" & CStr(nCount) & "'"
		sJsonObjRow = sJsonObjRow + "["

			Set Child = strNode.firstChild
			Do Until Child Is Nothing

			   '  if  Child.text contains "\" then add additionl "\\"  
			   ' sJsonObjRow = sJsonObjRow &  "'" &Child.nodeName & "':" & "'" & Replace(Child.text,"\","\\") & "'" 
			   sText = Replace(Child.text,"\","\\")
			   'sText = Replace(sText,":","=")
			   sText = Replace(sText,"'"," ")

			   sJsonObjRow = sJsonObjRow & "'" & sText & "'" 
 
			   ' generating Coulmn row here
			   if nCount = 1 then 

					   '   width:'auto', noresize:false added for table layot issue
					   '   sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & ", width:'auto', noresize:false" & "}"
					   '   sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & "}"
					   if Child.nextSibling is Nothing then 
   							sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & ", width:'auto', noresize:false" & "}"
					   else
							sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & "},"
					   end if
			   else
			   end if

			   if Child.nextSibling is Nothing then 
				   else
						sJsonObjRow = sJsonObjRow + ","
			   end if

   			   Set Child = Child.nextSibling
	   
		   Loop
	   
	   if nCount < objChildNodes.length then
   	    sJsonObjRow = sJsonObjRow & "],"
	   else
 	    sJsonObjRow = sJsonObjRow & "]"

	   end if
  
	Next

'Next

LogMsg 1, "XmlCategory2Json loop completed"

sJsonObjColumns = sJsonObjColumns  & "]"
sJsonObjRow = sJsonObjRow & "]"

sJsonTable = sJsonTable + "{ Table:{" + "id:'" + sSubcategory + "',columns:" +  sJsonObjColumns + ",rows:" + sJsonObjRow + "}}"

'return value
XmlCategory2Json = sJsonTable

End Function

' {'OS Name':'Microsoft Windows XP Professional','Version':'5.1.2600 Service Pack 2 Build 2600'}
'----------------------------------------------------------------------------------------
'
'	XmlCategory2JsonObject
'
'   Convert XML node from specified in strQuery category to Json table object
'----------------------------------------------------------------------------------------
Function XmlCategory2JsonObject(sSubcategory, sWorkingDirectory, sInputXmlFileName, strQuery)

	Dim sXmlFullFileName
	sXmlFullFileName = sWorkingDirectory + sInputXmlFileName

	LogMsg 1, "XmlCategory2JsonObject staring" + sXmlFullFileName

	Set xmlDoc = CreateObject( "Microsoft.XMLDOM" )
	xmlDoc.Async = "False"
	xmlDoc.Load( sXmlFullFileName )
	'Set colNodes = xmlDoc.selectNodes( strQuery )  ' we will parse per category. for each category separate json file
	Set objNode = xmlDoc.selectSingleNode( strQuery )

' For Each objNode in colNodes

	Dim objChildNodes, strNode 
	Set objChildNodes = objNode.childNodes
	Dim nCount
	DIM sJsonObjRow
	DIM sJsonObjColumns 
	DIM sJsonTable

	DIM sNameTmp, sName, sValueTmp, sValue
	
	sJsonObjRow = sJsonObjRow + "{"
	sJsonObjColumns = sJsonObjColumns + "["

	For Each strNode In objChildNodes 
   	    nCount = nCount + 1

 	    'sJsonObjRow = sJsonObjRow + "{'myId':" & "'" & CStr(nCount) & "'"

			Set Child = strNode.firstChild

			   sNameTmp = Replace(Child.text,"\","\\")
			   sName = Replace(sNameTmp," ","")
			   sName = Replace(sName,"/","")

			   sValue = Replace(Child.nextSibling.text,"\","\\")
			 
			   '  if  Child.text contains "\" then add additionl "\\"  
			   sJsonObjRow = sJsonObjRow & "'" & sName & "'"  & ":" & "'" & sValue & "'"
 
			   ' generating Coulmn row here
			   if nCount = 1 then 
					
					   sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & "}"
			
					   if Child.nextSibling is Nothing then 
					   else
							sJsonObjColumns = sJsonObjColumns + ","
					   end if
			   else
			   end if
	   	   
	   if nCount < objChildNodes.length then
   	    sJsonObjRow = sJsonObjRow & ","
	   else
 	   ' sJsonObjRow = sJsonObjRow & "}"

	   end if
  
	Next

'Next

	LogMsg 1, "XmlCategory2JsonObject completed" + sXmlFullFileName

sJsonObjColumns = sJsonObjColumns  & "]"
sJsonObjRow = sJsonObjRow & "}"

sJsonTable = sJsonTable + "{ Table:{" + "id:'" + sSubcategory + "',columns:" +  sJsonObjColumns + ",rows:" + sJsonObjRow + "}}"
'sJsonTable = sJsonTable + "{ Table:{ columns:" +  sJsonObjColumns + ",rows:" + sJsonObjRow + "}}"

'return value
XmlCategory2JsonObject = sJsonTable

End Function

'----------------------------------------------------------------------------------------
'
'	XmlCategory2JsonEx
'
'   Convert XML node from specified in strQuery category to Json table object
'----------------------------------------------------------------------------------------
Function XmlCategory2JsonEx(sWorkingDirectory, sInputXmlFileName, strQuery)

	Dim sXmlFullFileName
	sXmlFullFileName = sWorkingDirectory + sInputXmlFileName

	Set xmlDoc = CreateObject( "Microsoft.XMLDOM" )
	xmlDoc.Async = "False"
	xmlDoc.Load( sXmlFullFileName )
	'Set colNodes = xmlDoc.selectNodes( strQuery )  ' we will parse per category. for each category separate json file
	Set objNode = xmlDoc.selectSingleNode( strQuery )

' For Each objNode in colNodes

	Dim objChildNodes, strNode 
	Set objChildNodes = objNode.childNodes
	Dim nCount
	DIM sJsonObjRow
	DIM sJsonObjColumns 
	DIM sJsonTable

	sJsonObjRow = sJsonObjRow + "["
	sJsonObjColumns = sJsonObjColumns + "["

	For Each strNode In objChildNodes 
   	    nCount = nCount + 1

 	    sJsonObjRow = sJsonObjRow + "{'myId':" & "'" & CStr(nCount) & "'"

			Set Child = strNode.firstChild
			Do Until Child Is Nothing

			   '  if  Child.text contains "\" then add additionl "\\"  
			   sJsonObjRow = sJsonObjRow &  ",'" &Child.nodeName & "':" & "'" & Replace(Child.text,"\","\\") & "'" 
 
			   ' generating Coulmn row here
			   if nCount = 1 then 
					
					   sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & "}"
			
					   if Child.nextSibling is Nothing then 
					   else
							sJsonObjColumns = sJsonObjColumns + ","
					   end if
			   else
			   end if

   			   Set Child = Child.nextSibling
	   
		   Loop
	   
	   if nCount < objChildNodes.length then
   	    sJsonObjRow = sJsonObjRow & "},"
	   else
 	    sJsonObjRow = sJsonObjRow & "}"

	   end if
  
	Next

'Next

sJsonObjColumns = sJsonObjColumns  & "]"
sJsonObjRow = sJsonObjRow & "]"

sJsonTable = sJsonTable + "{ Table:{ columns:" +  sJsonObjColumns + ",rows:" + sJsonObjRow + "}}"

'return value
XmlCategory2JsonEx = sJsonTable

End Function

'----------------------------------------------------------------------------------------
'
'	WriteToFile write strText to specified strFile
'
'----------------------------------------------------------------------------------------
Function WriteToFile(strDirectory, strFile, strText)

	Dim objFSO, objFolder, objShell, objTextFile, objFile
	
	' Create the File System Object
	Set objFSO = CreateObject("Scripting.FileSystemObject")

	' Check that the strDirectory folder exists
	If objFSO.FolderExists(strDirectory) Then
	   Set objFolder = objFSO.GetFolder(strDirectory)
	Else
	   Set objFolder = objFSO.CreateFolder(strDirectory)
	   WshShell.Echo "Just created " & strDirectory
	End If

	If objFSO.FileExists(strDirectory & strFile) Then
	   Set objFolder = objFSO.GetFolder(strDirectory)
	Else
	   Set objFile = objFSO.CreateTextFile(strDirectory & strFile)
	   'WshShell.Echo "Just created " & strDirectory & strFile
	End If 

	set objFile = nothing
	set objFolder = nothing
	' OpenTextFile Method needs a Const value
	' ForAppending = 8 ForReading = 1, ForWriting = 2
	Const ForWriting = 2

	Set objTextFile = objFSO.OpenTextFile _
	(strDirectory & strFile, ForWriting, True)

	' Writes strText every time you run this VBScript
	objTextFile.WriteLine(strText)
	objTextFile.Close

End Function
'----------------------------------------------------------------------------------------
'
'	LogMsg
'
'----------------------------------------------------------------------------------------
Sub LogMsg( level, strtoLog)

	On Error Resume Next

	const ForAppending = 8, ForReading = 1, ForWriting = 2
	gstrUniqueName = "Scripting" 

	If IsEmpty(LogLevel) Then LogLevel = 1
	If IsEmpty(LogFilePrefix) Then LogFilePrefix = "GetDiagnostics"

	' if vista then logs may be in another folder ....
	if isVista=true then 
		LogFilePath = sWorkingDirectory + "..\" 'C:\Windows\Temp usually on Vista
		'LogFilePath = sWorkingDirectory + "C:\Users\user\"
	else
		LogFilePath = sWorkingDirectory + "..\" 'C:\DOCUME~1\user\LOCALS~1\Temp\ usually on XP
 	end if 

	If CInt(LogLevel) >= level Then
		Dim objFileSysObj, objTextFile

		LogFileName = LogFilePrefix & "_" & gstrUniqueName & ".log"

		Err.Clear
		Set objFileSysObj = CreateObject("Scripting.FileSystemObject")
		If Err.Number Then
			'WScript.Echo "Error " & Err.Number & _ " creating FileSystemObject: " & Err.Description
		Else
			'Open for Append, create if not exists
			Set objTextFile = objFileSysObj.OpenTextFile(LogFilePath & LogFileName, ForAppending, True)
			If Err.Number Then
				WScript.Echo "Error " & Err.Number & _
							 " opening/creating logfile (" & _
							 LogFilePath & LogFile & "): " & Err.Description
			Else
				'WScript.Echo strtoLog

				objTextFile.WriteLine(Now() & "  " & strtoLog)
				objTextFile.Close
				Set objTextFile = Nothing
			End If
			Set objFileSysObj = Nothing
		End If

		Err.Clear
	End If
	On Error Goto 0

End Sub

Function ProcessExists( aProcessName )
	Dim bResult, oWMI, oProcess, oProcesses
	bResult = false
	Set oWMI = GetObject("winmgmts:{impersonationLevel=impersonate}!\root\cimv2")
	Set oProcesses = oWMI.ExecQuery ("Select * from Win32_Process Where Name = '" & aProcessName & "'")
	For Each oProcess in oProcesses
		bResult = true
	Next
	set oWMI = nothing
	ProcessExists = bResult
End Function

Function GetCurrentDirectory()
   Dim aCurrentPath, sErr, oFile
   ' Get current directory (where script is located)

   Set oFile = fso.GetFile(sScriptName)
   sCurrentDir = oFile.path
   aCurrentPath = split(sCurrentDir, ":")
   sCurrentDrive = aCurrentPath(0)
   sErr = "This program must reside on a local drive or mapped drive."
   If Len(sCurrentDrive) <> 1 Then 
      WScript.Echo sErr
      WScript.Quit 1
   End If
   sCurrentDir = aCurrentPath(1)
   sCurrentDir = Mid(sCurrentDir, 1, Len(sCurrentDir) - Len(oFile.name))
   sPdftk = sCurrentDrive & ":" & sCurrentDir & sTKCmd

End Function

'  
'  return true if vista - otherwise return false
' 
function IsOsVista()

	Dim bIsVista, osVer
	bIsVista = false
	osVer = GetOsVersion()

	Select Case osVer
	Case "6.0" OSystem = "Vista"
		bIsVista = true
	Case "5.0" OSystem = "W2K"
	Case "5.1" OSystem = "XP"
	Case "5.2" OSystem = "Windows 2003"
	Case "4.0" OSystem = "NT 4.0"
	Case Else OSystem = "Unknown - probably Win 9x"
	End Select
	'Wscript.Echo "OSystem:" + OSystem
	LogMsg 1, "OS version: " &osVer & " It is: " & OSystem
		
	IsOsVista = bIsVista
end function

'
'	return os version in format like 6.0, 5.0 and so on
'
function GetOsVersion()

	Dim objWMI, objItem, colItems
	Dim strComputer, VerOS, VerBig, Ver9x, Version9x, OS, OSystem

	if bUseWMI=true	then 
		' Get the computer name dot = this computer.
		strComputer = "."
		' This is where WMI interrogates the operating system
		Set objWMI = GetObject("winmgmts:\\" & strComputer & "\root\cimv2")

		Set colItems = objWMI.ExecQuery("Select * from Win32_OperatingSystem",,48)

		' Here we filter Version from the dozens of properties
		For Each objItem in colItems
		VerBig = Left(objItem.Version,3)
		Next
	else
		VerBig = "5.1" 'defult is XP ''TODO if it is Vista we have optiomization - run msinfo once... this will not work
	end if 

	GetOsVersion = VerBig

end function

' StartProcessMethod1 start process using objWMIService.ExecMethod
' 
' -------------------------------------------------------' 
' Function StartProcessMethod1()
Function StartProcessMethod1(sWorkingDirectory, sXmlOutputFileName )

	CONST WshRunning = 0 
	CONST WshFinished = 1
	const cDebuggerProcName = "msinfo32.exe"

	Dim objWMIService, objProcess
	Dim strShell, objProgram, strComputer, strExe, strInput
	'strExe = "msinfo32 /nfo C:\Users\user\Summary.nfo"

	strExe = "msinfo32 /nfo " + sWorkingDirectory + sXmlOutputFileName

	LogMsg 1, "Created process: Step 1. Command to run:" + strExe

	strComputer = "."

	' Connect to WMI
	set objWMIService = getobject("winmgmts://"_
	& strComputer & "/root/cimv2") 
	' Obtain the Win32_Process class of object.
	Set objProcess = objWMIService.Get("Win32_Process")
	Set objProgram = objProcess.Methods_( _
	"Create").InParameters.SpawnInstance_
	objProgram.CommandLine = strExe 

	'Execute the program now at the command line.
	Set strShell = objWMIService.ExecMethod( _
	"Win32_Process", "Create", objProgram) 

	LogMsg 1, "Created process: " & strExe & " on " & strComputer

	' End of Example of a Process VBScript 

	' wait for dump to finish
	While ( ProcessExists( cDebuggerProcName ) )
			WScript.Sleep 1000
			LogMsg 1, "ProcessExists"
	Wend

	LogMsg 1, "Ended process: " & strExe & " on " & strComputer

End Function

' ------------------------------------------------------- 
'	StartProcessMethod2 start process using wshShell.Exec
' ------------------------------------------------------- 
Function StartProcessMethod2(sWorkingDirectory, sXmlOutputFileName)

	CONST WshRunning = 0 
	CONST WshFinished = 1
	const cDebuggerProcName = "msinfo32.exe"
	Dim strExe

	strExe = "msinfo32 /nfo " + sWorkingDirectory + sXmlOutputFileName
	
	LogMsg 1, "StartProcessMethod2 started... Command to run is:" + strExe

	Set ObjExec = wshShell.Exec( strExe ) 
	Do While ObjExec.Status =  WshRunning 
	  LogMsg 1, "running..."
	  'WScript.Sleep 100  'cause exception when run from ScriptEngine TODO...
	Loop 

	LogMsg 1, "StartProcessMethod2 ended with status:" + ObjExec.Status

End Function
' -------------------------------------------------------' 
' GetSysInfo start process using wshShell.Run(fBatFileName, 0, bWaitOnReturn)
' -------------------------------------------------------' 
Sub GetSysInfo(sWorkingDirectory, sOutputFileName, sCategories)

	LogMsg 1, "GetSysInfo starteed"

	'''''''''''''''''''''''''''''''''''
	'  Create the File System Object
	Set objFSO = CreateObject("Scripting.FileSystemObject")
	
	'''''''''''''''''''''''''''''''''''
	'  Create subdirectory TODO - create subfolders 
	'
	Dim objFSO, objFolder

	'''''''''''''''''''''''''''''''''''
	'  Check that the strDirectory folder exists
	If objFSO.FolderExists(sWorkingDirectory) Then
	   Set objFolder = objFSO.GetFolder(sWorkingDirectory)
	Else
	   Set objFolder = objFSO.CreateFolder(sWorkingDirectory)
	End If
	
	'''''''''''''''''''''''''''''''''''
	' Create batch file
	'
	Dim  fBatFileName,  fBatFileFullName, sOutputFullFileName, objFile, origCurrentDirectory

	fBatFileName = "bGetSysInfo.bat"
	fBatFileFullName = sWorkingDirectory + fBatFileName
	sOutputFullFileName = sWorkingDirectory + sOutputFileName

    Set objFile = objFSO.CreateTextFile(strDirectory & fBatFileFullName, True)

	objFile.WriteLine( "cd ""%programfiles%\Common Files\Microsoft Shared\msinfo\""" )
	objFile.WriteLine( "start /wait msinfo32 /categories " + sCategories + " /nfo " +  """" + sOutputFullFileName + """" )
	'objFile.WriteLine( "msinfo32" + " /nfo " +  """" + sOutputFullFileName + """" )
	objFile.Close()

	'''''''''''''''''''''''''''''''''''''''
	' Run script such call working fine alternatives have some issue

	LogMsg 1, "wshShell.Run" + fBatFileName
	origCurrentDirectory = wshShell.CurrentDirectory
	wshShell.CurrentDirectory = sWorkingDirectory

    LogMsg 1, "wshShell.Run starting"
	bWaitOnReturn = true
  	retVal = wshShell.Run(fBatFileName, 0, bWaitOnReturn)
	LogMsg 1, "wshShell.Run completed with RetValue:" & retVal

	''''''''''''''''''''''''''''''''''''''
	' Delete batch file
	objFSO.DeleteFile(fBatFileFullName)
	wshShell.CurrentDirectory = origCurrentDirectory

End Sub

' {'OS Name':'Microsoft Windows XP Professional','Version':'5.1.2600 Service Pack 2 Build 2600'}
'----------------------------------------------------------------------------------------
'
'     XmlCategory2Json
'
'   Convert XML node from specified in strQuery category to Json table object
'----------------------------------------------------------------------------------------
Function XmlCategory2JsonObjectVista(sSubcategory, sWorkingDirectory, sInputXmlFileName, strQuery)

	  LogMsg 1, "XmlCategory2JsonObjectVista starting"

      Dim sXmlFullFileName
      sXmlFullFileName = sWorkingDirectory + sInputXmlFileName

      Set xmlDoc = CreateObject( "Microsoft.XMLDOM" )
      xmlDoc.Async = "False"
      xmlDoc.Load( sXmlFullFileName )
      'Set colNodes = xmlDoc.selectNodes( strQuery )  ' we will parse per category. for each category separate json file
      'Set objNode = xmlDoc.selectNodes( strQuery )

' For Each objNode in colNodes

      Dim objChildNodes, strNode 
      Set objChildNodes = xmlDoc.selectNodes( strQuery )
      DIM sJsonObjRow
      DIM sJsonObjColumns 
      DIM sJsonTable

    DIM sNameTmp, sName, sValueTmp, sValue
    DIM sChild_name 
    DIM sChild_value
    Dim nCount

	  nCount = 0
      
      sJsonObjRow = sJsonObjRow + "{"
      sJsonObjColumns = sJsonObjColumns + "["
      sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & "Value" & "'" & "}"
      
      For Each strNode In objChildNodes 

          'sJsonObjRow = sJsonObjRow + "{'myId':" & "'" & CStr(nCount) & "'"
              nCount = nCount + 1

                  sChild_name = strNode.firstChild.nodeTypedValue
                  sChild_value = strNode.lastChild.nodeTypedValue

                     sNameTmp = Replace(sChild_name ,"\","\\")
                     sName = Replace(sNameTmp," ","")
                     sName = Replace(sName,"/","")

                     sValue = Replace(sChild_value,"\","\\")
                   
                     '  if  Child.text contains "\" then add additionl "\\"  
                     '  sJsonObjRow = sJsonObjRow & "'" & sName & "'"  & ":" & "'" & sValue & "'" & ","
                     
					 if nCount < objChildNodes.length then
					   sJsonObjRow = sJsonObjRow & "'" & sName & "'"  & ":" & "'" & sValue & "'" & ","
					 else
					   'sJsonObjRow = sJsonObjRow & "}"
						sJsonObjRow = sJsonObjRow & "'" & sName & "'"  & ":" & "'" & sValue & "'"
					 end if
 
                     ' generating Coulmn row here
     Next

'Next

sJsonObjColumns = sJsonObjColumns  & "]"
sJsonObjRow = sJsonObjRow & "}"

sJsonTable = sJsonTable + "{ Table:{" + "id:'" + sSubcategory + "',columns:" +  sJsonObjColumns + ",rows:" + sJsonObjRow + "}}"
'sJsonTable = sJsonTable + "{ Table:{ columns:" +  sJsonObjColumns + ",rows:" + sJsonObjRow + "}}"

	  LogMsg 1, "XmlCategory2JsonObjectVista ended"

'return value
XmlCategory2JsonObjectVista = sJsonTable

End Function

' ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' funciotn retrieve IEsummary category for Vista using WMIService as far as it is not availble using msinfo32 on Vista
' 
' { Table:
' { id:'IEsummary',
'   columns:[{name:'Item'},{name:'Value', width:'auto', noresize:false}],
'   rows:[['Version','6.0.2900.2180'],['Build','62900.2180'],['Application Path','C:\\Program Files\\Internet Explorer'],['Language','English (United States)'],['Active Printer','Not Available'],['',''],['Cipher Strength','128-bit'],['Content Advisor','Disabled'],['IEAK Install','No']]}
' }
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
function MicrosoftIE_Summary()

	Dim sJsonTable, sJsonObjColumns, sJsonObjRow

	sJsonObjColumns = "[columns:[{name:'Item'},{name:'Value', width:'auto', noresize:false}]"
	sJsonObjRow = "["

	strComputer = "."
	Set objWMIService = GetObject("winmgmts:\\" & strComputer _
		& "\root\cimv2\Applications\MicrosoftIE")
	Set colIESettings = objWMIService.ExecQuery _
		("Select * from MicrosoftIE_Summary")
	For Each strIESetting in colIESettings
		sJsonObjRow = sJsonObjRow & "['" & "Version" & "','" &  strIESetting.Version & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Build" & "','" &  strIESetting.Build & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Application Path" & "','" &  strIESetting.Path & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Language" & "','" &  strIESetting.Language + "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Active printer" & "','" +  strIESetting.ActivePrinter & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Cipher strength" & "','" & strIESetting.CipherStrength & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Content advisor" & "','" &  strIESetting.ContentAdvisor & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "IEAK installed" & "','" &  strIESetting.IEAKInstall & "'" & "]" & ","
		sJsonObjRow = sJsonObjRow & "['" & "Application Path" & "','" &  strIESetting.Path & "'" & "]"
	Next

	sJsonTable = sJsonTable & "["
	sJsonTable = sJsonTable & "{ Table:{" & "id:'" & "IEsummary" & "',columns:" &  sJsonObjColumns & ",rows:" & sJsonObjRow & "}}"
	sJsonTable = sJsonTable & "]"
	'Wscript.Echo "sJsonTable: " & sJsonTable

	MicrosoftIE_Summary = sJsonTable
	
End function

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'						GetOSLanguageID
'
'	return languageId code
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
function GetOSLanguageID()

	Dim sLangId

	if bUseWMI=true	then 
		strComputer = "."
		Set objWMIService = GetObject("winmgmts:" _
			& "{impersonationLevel=impersonate}!\\" & strComputer & "\root\cimv2")

		Set colOperatingSystems = objWMIService.ExecQuery _
			("Select * from Win32_OperatingSystem")

		For Each objOperatingSystem in colOperatingSystems
			sLangId = objOperatingSystem.OSLanguage
		Next
	else
		sLangId = "1033"' default language will be en-us
	end if 

	GetOSLanguageID = sLangId

End function 

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'						isGetOSLanguageIDSupported
'
'	return true if language supported otherwise return false
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
function isGetOSLanguageIDSupported(sLangId)

	Dim isSupported

	isSupported = true 

	select case sLangId
	
	case "1033"  '  English (United States)
	case "3081"  ' 	English (Australia) 
	case "10249" ' 	English (Belize) 
	case "4105"  ' 	English (Canada)
	case "9225"  ' 	English (Caribbean)
	case "6153"  ' 	English (Ireland) 
	case "8201"  ' 	English (Jamaica)
	case "5129"  ' 	English (New Zealand) 
	case "13321" ' 	English (Philippines)
	case "7177"  '	English (South Africa)
	case "11273" ' 	English (Trinidad)
	case "2057"  ' 	English (United Kingdom)
	case "12297" '	English (Zimbabwe) 
	case Else
		isSupported = false
	end select

	isGetOSLanguageIDSupported = isSupported

End function

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'						GetSystemInfoWithWMI
'
'	retrieve msinfo using MWI classes:
'   Win32_OperatingSystem, Win32_ComputerSystem, Win32_Processor, Win32_BIOS, Win32_TimeZone, Win32_PageFileUsage
'
'   Miised: 'Hardware abstraction layer Version=5.1   http://www.thescripts.com/forum/thread255863.html ?????
'   %systemroot%\system32 folder as hal.dll.
'   http://www.itcareerpath.com/Article/ArticleID/48583/48583.html
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'   Here is example of the retrieved information:
'----------------------------------------------------------------------------------------------------------
'  { Table:
'		{
'			id:'SystemSummary',
'			columns:[{name:'Item'},],
'			rows:
'			{
			'OSName':'Microsoft Windows XP Professional',
			'Version':'5.1.2600 Service Pack 2 Build 2600',
			'OSManufacturer':'Microsoft Corporation',
			'SystemName':'WS-VMXP1',
			'SystemManufacturer':'VMware, Inc.',
			'SystemModel':'VMware Virtual Platform',
			'SystemType':'X86-based PC',
			'Processor':'x86 Family 15 Model 6 Stepping 8 GenuineIntel ~2659 Mhz',
			'BIOSVersionDate':'Phoenix Technologies LTD 6.00, 4/17/2006',
			'SMBIOSVersion':'2.31',
			'WindowsDirectory':'C:\\WINDOWS',
			'SystemDirectory':'C:\\WINDOWS\\system32',
			'BootDevice':'\\Device\\HarddiskVolume1',
			'Locale':'United States',
			'HardwareAbstractionLayer':'Version = "5.1.2600.2180 (xpsp_sp2_rtm.040803-2158)"',
			'UserName':'WS-VMXP1\\user',
			'TimeZone':'Jerusalem Standard Time',
			'TotalPhysicalMemory':'256.00 MB',
			'AvailablePhysicalMemory':'72.64 MB',
			'TotalVirtualMemory':'2.00 GB',
			'AvailableVirtualMemory':'1.96 GB',
			'PageFileSpace':'616.67 MB',
			'PageFile':'C:\\pagefile.sys'
'			}
'		}
'	}
'  
function GetSystemInfoWithWMIEx()
	
	LogMsg 1, "GetSystemInfoWithWMIEx started..."

	Dim sMsInfoBuf, sTmpValue, isOSLanguageSupported 

	' open all statements 
	sMsInfoBuf	= "{ Table:{"
	sMsInfoBuf	= sMsInfoBuf & "id:'SystemSummary',"
	sMsInfoBuf	= sMsInfoBuf & "columns:[{name:'Item'},],"
	sMsInfoBuf	= sMsInfoBuf & "rows:{"

	strComputer = "." 
	Set objWMIService = GetObject("winmgmts:\\" & strComputer & "\root\CIMV2") 
	Set colItemsOS = objWMIService.ExecQuery( _
		"SELECT * FROM Win32_OperatingSystem",,48)

	LogMsg 1, "Win32_OperatingSystem"

	For Each objItemOS in colItemsOS 
		sMsInfoBuf = AddNode(sMsInfoBuf, "OSName", objItemOS.Caption, true)
		sMsInfoBuf = AddNode(sMsInfoBuf, "Version", objItemOS.Version & " Service Pack " & objItemOS.ServicePackMajorVersion  & "." & objItemOS.ServicePackMinorVersion & " Build "& objItemOS.BuildNumber, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "OSManufacturer", objItemOS.Manufacturer, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "SystemName", objItemOS.CSName, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "SystemDirectory", objItemOS.SystemDirectory, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "WindowsDirectory", objItemOS.WindowsDirectory, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "BootDevice", objItemOS.BootDevice, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "Locale", LangidByHex(objItemOS.Locale), false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "PageFileSpace", FormatNumber(objItemOS.SizeStoredInPagingFiles/1000000) & " GB", false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "AvailablePhysicalMemory", FormatNumber(objItemOS.FreePhysicalMemory/1000) & " MB", false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "TotalVirtualMemory", FormatNumber(objItemOS.TotalVirtualMemorySize/1000000) & " GB", false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "AvailableVirtualMemory", FormatNumber(objItemOS.FreeVirtualMemory/1000000) & " GB", false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "OSLanguage", LangidByHex(Hex(objItemOS.OSLanguage)), false)
		'HardwareAbstractionLayer is version info of C:\WINDOWS\system32\hal.dll the same is on XP SP2 and Vista
		Set halFile = GetObject("winMgmts:CIM_DataFile.Name=" & "'" & objItemOS.SystemDirectory & "\hal.dll" & "'")
		sMsInfoBuf = AddNode(sMsInfoBuf, "HardwareAbstractionLayer", "Version = " & halFile.Version, false)

		'OSystem property for internal purposes
		sMsInfoBuf = AddNode(sMsInfoBuf, "OSystem", OSystem, false)
	
		'UnSupportedOSLanguage property is used for internal purposes 
		isOSLanguageSupported = isGetOSLanguageIDSupported(objItemOS.OSLanguage)
	
		if isOSLanguageSupported=false then 
			sMsInfoBuf = AddNode(sMsInfoBuf, "UnSupportedOSLanguage", LangidByHex(Hex(objItemOS.OSLanguage)), false)
		else
			sMsInfoBuf = AddNode(sMsInfoBuf, "UnSupportedOSLanguage", "", false)
		end if 

	Next

	LogMsg 1, "Win32_ComputerSystem"
	Set colItemsCS = objWMIService.ExecQuery( _
		"SELECT * FROM Win32_ComputerSystem",,48) 

	For Each objItemCS in colItemsCS
		sMsInfoBuf = AddNode(sMsInfoBuf, "SystemModel", objItemCS.Model, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "SystemType", objItemCS.SystemType, false)
		sMsInfoBuf = AddNode(sMsInfoBuf, "UserName", objItemCS.UserName, false)

		if(objItemCS.TotalPhysicalMemory > 1000000000) then
			sMsInfoBuf = AddNode(sMsInfoBuf, "TotalPhysicalMemory", FormatNumber(objItemCS.TotalPhysicalMemory/1000000000) & " GB", false)
		else 
			sMsInfoBuf = AddNode(sMsInfoBuf, "TotalPhysicalMemory", FormatNumber(objItemCS.TotalPhysicalMemory/1000000) & " MB", false)
		end if

		sMsInfoBuf = AddNode(sMsInfoBuf, "SystemManufacturer", objItemCS.Manufacturer, false)
	Next

	LogMsg 1, "Win32_Processor"
	Set colItemsProc = objWMIService.ExecQuery( _
		"SELECT * FROM Win32_Processor",,48) 

	For Each objItemProc in colItemsProc 'really loop here if more then one processor
		'Wscript.Echo "TODO Processor and Processor1, Processor2...."
		sMsInfoBuf = AddNode(sMsInfoBuf, "Processor", objItemProc.Description & objItemProc.Manufacturer & " ~" & objItemProc.CurrentClockSpeed & " Mhz", false)
		'Wscript.Echo "---------------------------------------------------------------------------"
	Next

	LogMsg 1, "Win32_BIOS"
	Set colItemsBios = objWMIService.ExecQuery( _
		"SELECT * FROM Win32_BIOS",,48) 

	For Each objItemBios in colItemsBios 
		sMsInfoBuf = AddNode(sMsInfoBuf, "SMBIOSVersion", objItemBios.SMBIOSMajorVersion & "." & objItemBios.SMBIOSMinorVersion, false)
		'Convert here Release date of the Windows BIOS in the Coordinated Universal Time (UTC) format of YYYYMMDDHHMMSS.MMMMMM(+-)OOO.
		sTmpValue = Left(objItemBios.ReleaseDate, 4) & "-" & Mid(objItemBios.ReleaseDate, 5, 2) & "-" & Mid(objItemBios.ReleaseDate, 7 ,2)
		sMsInfoBuf = AddNode(sMsInfoBuf, "BIOSVersionDate", objItemBios.Manufacturer & " " & objItemBios.SMBIOSBIOSVersion & ", " & sTmpValue, false)
	Next

	LogMsg 1, "Win32_TimeZone"
	Set colItemsTimeZone = objWMIService.ExecQuery( _
		"SELECT * FROM Win32_TimeZone",,48) 

	For Each objItemTimeZone in colItemsTimeZone 
		sMsInfoBuf = AddNode(sMsInfoBuf, "TimeZone", objItemTimeZone.StandardName, false)
	Next

	LogMsg 1, "Win32_PageFileUsage"
	Set colItemsPageFile = objWMIService.ExecQuery( _
		"SELECT * FROM Win32_PageFileUsage",,48) 
	
	For Each objItemPageFile in colItemsPageFile 
		sMsInfoBuf = AddNode(sMsInfoBuf, "PageFile", objItemPageFile.Name, false)
	Next

	'close all statements 
	sMsInfoBuf	= sMsInfoBuf & "}}}"
	GetSystemInfoWithWMIEx = sMsInfoBuf

End function

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'						LangidByHex for example 
'
'	http://www.microsoft.com/globaldev/reference/winxp/xp-lcid.mspx LCIDHex
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
function LangidByHex(sLangIdHex)

	Dim sLangID

	if Len(sLangIdHex) = 3 then
		sLangIdHex = "0" & sLangIdHex 'just add 0 to make 4 digits dex
	else 
	end if 

	select case sLangIdHex
	 case "0436"
		 sLangID = "Afrikaans"
	 case "041c"
 		 sLangID = "Albanian"
	 case "0401" 
	 	 sLangID = "Arabic_Saudi_Arabia"
     case "0801"
		 sLangID = "Arabic_Iraq"
	 case "0c01"
		 sLangID = "Arabic_Egypt"
	 case "1001" 
		 sLangID = "Arabic_Libya"
	 case "1401"
		 sLangID = "Arabic_Algeria"
	 case "1801" 
		 sLangID = "Arabic_Morocco"
	 case "1c01"
		 sLangID = "Arabic_Tunisia"
	 case "2001"
		 sLangID = "Arabic_Oman"
	 case "2401" 
		 sLangID = "Arabic_Yemen"
	 case "2801"
		 sLangID = "Arabic_Syria"
	 case "2c01"
		 sLangID = "Arabic_Jordan" 
	 case "3001"
		 sLangID = "Arabic_Lebanon" 
	 case "3401"
		 sLangID = "Arabic_Kuwait"
	 case "3801"
		 sLangID = "Arabic_UAE"
	 case "3c01"
		 sLangID = "Arabic_Bahrain" 
	 case "4001"
		 sLangID = "Arabic_Qatar" 
	 case "042b"
		 sLangID = "Armenian" 
	 case "042c"
		 sLangID = "Azeri_Latin" 
	 case "082c"
		 sLangID = "Azeri_Cyrillic" 
	 case "042d"
		 sLangID = "Basque" 
	 case "0423"
		 sLangID = "Belarusian"
	 case "0445"
		 sLangID = "Bengali_India*" 
	 case "141A"
		 sLangID = "Bosnian_Latin*" 
	 case "0402"
		 sLangID = "Bulgarian" 
	 case "0403"
		 sLangID = "Catalan" 
	 case "0404"
		 sLangID = "Chinese_Taiwan" 
	 case "0804"
		 sLangID = "Chinese_PRC"
	 case "0c04"
		 sLangID = "Chinese_Hong_Kong"
	 case "1004"
		 sLangID = "Chinese_Singapore"
	 case "1404"
		 sLangID = "Chinese_Macau"
	 case "041a"
		 sLangID = "Croatian" 
	 case "101A"
		 sLangID = "Croatian_Bosnia_Herzegovina*"
	 case "0405"
		 sLangID = "Czech" 
	 case "0406"
		 sLangID = "Danish" 
	 case "0465"
		 sLangID = "Divehi"
	 case "0413"
		 sLangID = "Dutch_Standard"
	 case "0813"
		 sLangID = "Dutch_Belgian" 
	 case "0409"
		 sLangID = "English United States"
	 case "0809"
		 sLangID = "English United Kingdom"
	 case "0c09"
		 sLangID = "English Australian" 
	 case "1009"
		 sLangID = "English Canadian"
	 case "1409"
		 sLangID = "English New Zealand" 
	 case "1809"
		 sLangID = "English Ireland" 
	 case "1c09"
		 sLangID = "English South Africa" 
	 case "2009"
		 sLangID = "English Jamaica" 
	 case "2409"
		 sLangID = "English Caribbean" 
	 case "2809"
		 sLangID = "English Belize" 
	 case "2c09"
		 sLangID = "English Trinidad"
	 case "3009"
		 sLangID = "English Zimbabwe" 
	 case "3409"
		 sLangID = "English Philippines "
	 case "0425"
		 sLangID = "Estonian"
	 case "0438"
		 sLangID = "Faeroese" 
	 case "0429"
		 sLangID = "Farsi" 
	 case "040b"
		 sLangID = "Finnish" 
	 case "040c"
		 sLangID = "French_Standard" 
	 case "080c"
		 sLangID = "French_Belgian" 
	 case "0c0c"
		 sLangID = "French_Canadian" 
	 case "100c"
		 sLangID = "French_Swiss" 
	 case "140c"
		 sLangID = "French_Luxembourg" 
	 case "180c"
		 sLangID = "French_Monaco" 
	 case "0437"
		 sLangID = "Georgian" 
	 case "0456"
		 sLangID = "Galician" 
	 case "0407"
		 sLangID = "German_Standard" 
	 case "0807"
		 sLangID = "German_Swiss" 
	 case "0c07"
		 sLangID = "German_Austrian"
	 case "1007"
		 sLangID = "German_Luxembourg" 
	 case "1407"
		 sLangID = "German_Liechtenstein"
	 case "0408"
		 sLangID = "Greek"
	  case "0447"
		 sLangID = "Gujarati" 
	 case "040d"
		 sLangID = "Hebrew" 
	 case "0439"
		 sLangID = "Hindi"
	 case "040e"
		 sLangID = "Hungarian" 
	 case "040f"
		 sLangID = "Icelandic" 
	 case "0421"
		 sLangID = "Indonesian" 
	 case "0410"
		 sLangID = "Italian_Standard"
	 case "0810"
		 sLangID = "Italian_Swiss" 
	 case "0411"
		 sLangID = "Japanese" 
	 case "044b"
		 sLangID = "Kannada"
	 case "043f"
		 sLangID = "Kazakh" 
	 case "0457"
		 sLangID = "Konkani"
	 case "0412"
		 sLangID = "Korean"
	 case "0440"
		 sLangID = "Kyrgyz" 
	 case "0426"
		 sLangID = "Latvian" 
	 case "0427"
		 sLangID = "Lithuanian"
	 case "042f"
		 sLangID = "Macedonian" 
	 case "043e"
		 sLangID = "Malay_Malaysia" 
	 case "083e"
		 sLangID = "Malay_Brunei_Darussalam" 
	 case "044c"
		 sLangID = "Malayalam*" 
	 case "043a"
		 sLangID = "Maltese*" 
	 case "0481"
		 sLangID = "Maori*" 
	 case "044e"
		 sLangID = "Marathi"
	 case "0450"
		 sLangID = "Mongolian" 
	 case "0414"
		 sLangID = "Norwegian_Bokmal"
	 case "0814"
		 sLangID = "Norwegian_Nynorsk" 
	 case "0415"
		 sLangID = "Polish" 
	 case "0416"
		 sLangID = "Portuguese_Brazilian" 
	 case "0816"
		 sLangID = "Portuguese_Standard"
	 case "0446"
		 sLangID = "Punjabi" 
	 case "046b"
		 sLangID = "Quechua_Bolivia*" 
	 case "086b"
		 sLangID = "Quechua_Ecuador*" 
	 case "0c6b"
		 sLangID = "Quechua_Peru*"
	 case "0418"
		 sLangID = "Romanian"
	 case "0419"
		 sLangID = "Russian" 
	 case "243b"
		 sLangID = "Sami_Inari*" 
	 case "103b"
		 sLangID = "Sami_Lule_Norway*" 
	 case "143b"
		 sLangID = "Sami_Lule_Sweden*" 
	 case "0c3b"
		 sLangID = "Sami_Northern_Finland*" 
	 case "043b"
		 sLangID = "Sami_Northern_Norway*" 
	 case "083b"
		 sLangID = "Sami_Northern_Sweden*" 
	 case "203b"
		 sLangID = "Sami_Skolt*" 
	 case "183b"
		 sLangID = "Sami_Southern_Norway*" 
	 case "1c3b"
		 sLangID = "Sami_Southern_Sweden*" 
	 case "044f"
		 sLangID = "Sanskrit" 
	 case "081a"
		 sLangID = "Serbian_Latin"
	 case "181a"
		 sLangID = "Serbian_Latin_Bosnia_Herzegovina*" 
	 case "0c1a"
		 sLangID = "Serbian_Cyrillic" 
	 case "1c1a"
		 sLangID = "Serbian_Cyrillic_Bosnia_Herzegovina*" 
	 case "041b"
		 sLangID = "Slovak" 
	 case "0424"
		 sLangID = "Slovenian"
	 case "040a"
		 sLangID = "Spanish_Traditional_Sort" 
	 case "080a"
		 sLangID = "Spanish_Mexican" 
	 case "0c0a"
		 sLangID = "Spanish_Modern_Sort"
	 case "100a"
		 sLangID = "Spanish_Guatemala"
	 case "140a"
		 sLangID = "Spanish_Costa_Rica"
	 case "180a"
		 sLangID = "Spanish_Panama" 
	 case "1c0a"
		 sLangID = "Spanish_Dominican_Republic"
	 case "200a"
		 sLangID = "Spanish_Venezuela"
	 case "240a"
		 sLangID = "Spanish_Colombia"
	 case "280a"
		 sLangID = "Spanish_Peru" 
	 case "2c0a"
		 sLangID = "Spanish_Argentina" 
	 case "300a"
		 sLangID = "Spanish_Ecuador" 
	 case "340a"
		 sLangID = "Spanish_Chile" 
	 case "380a"
		 sLangID = "Spanish_Uruguay" 
	 case "3c0a"
		 sLangID = "Spanish_Paraguay" 
	 case "400a"
		 sLangID = "Spanish_Bolivia" 
	 case "440a"
		 sLangID = "Spanish_El_Salvador" 
	 case "480a"
		 sLangID = "Spanish_Honduras" 
	 case "4c0a"
		 sLangID = "Spanish_Nicaragua" 
	 case "500a"
		 sLangID = "Spanish_Puerto_Rico" 
	 case "0441"
		 sLangID = "Swahili" 
	 case "041d"
		 sLangID = "Swedish" 
	 case "081d"
		 sLangID = "Swedish_Finland" 
	 case "045a"
		 sLangID = "Syriac"
	 case "0449"
		 sLangID = "Tamil"
     case "0444"
		 sLangID = "Tatar" 
	 case "044a"
		 sLangID = "Telugu" 
	 case "041e"
		 sLangID = "Thai"
	 case "0432"
		 sLangID = "Tswana*" 
	 case "0422"
		 sLangID = "Ukrainian" 
	 case "041f"
		 sLangID = "Turkish" 
	 case "0422"
		 sLangID = "Ukrainian" 
	 case "0420"
		 sLangID = "Urdu"
	 case "0443"
		 sLangID = "Uzbek_Latin"
	 case "0843"
		 sLangID = "Uzbek_Cyrillic" 
	 case "042a"
		 sLangID = "Vietnamese" 
	 case "0452"
		 sLangID = "Welsh*" 
	 case "0434"
		 sLangID = "Xhosa*" 
	 case "0435"
		 sLangID = "Zulu*"

	case Else
		sLangID = "unknown"
	end select

	LangidByHex = sLangID

End function
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'						AddNode to the json table specified in sMsInfoBuf
'
'	name   - (string) new lement name
'   value  - (string) new element value
'   isRoot - (boolean) value may be true if it is first element in the table, otherwise may be false
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
function AddNode(sMsInfoBuf, name, value, isRoot)

	if isRoot=true  then
	else 
		sMsInfoBuf =  sMsInfoBuf & ","
	end if 

	sMsInfoBuf =  sMsInfoBuf & "'" & name &  "':" & "'" & Replace(value,"\","\\") & "'"  

	AddNode = sMsInfoBuf

End function
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'	GetQueryByCategoryID
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
function GetQueryByCategoryID(sSubcategory)

	select case sSubcategory

	case "SystemSummary"
		'' "System Summary" is always retrieved using WMI...
		'LogMsg 1, "SystemSummary is always retrieved using WMI..."
		'sJsonTable = GetSystemInfoWithWMIEx()
		'LogMsg 1, "GetSystemInfoWithWMIEx returned:" & sJsonTable
		''ScriptEngineHost.SetReturnParameters sJsonTable, "SomeObject"
		'BuildJsonFileByDiagCategory = sJsonTable
		'Exit Function 

		'old version code todo deleted it 
		sCategories = "+-" + sSubcategory
		sXmlOutputFileName = sSubcategory + ".nfo"
	    sXpathQuery = "//MsInfo/Category[@name='System Summary']"
		sJsonFileName = sSubcategory + ".txt"
		namedJson = 1

	'-----------------------------------------
	' REM Subcategories for Hardware Resources TODO: this switch may disappear
	'-----------------------------------------
	 case "ResourcesConflicts"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Hardware Resources']/Category[@name='Conflicts/Sharing']"

	 case "ResourcesDMA"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Hardware Resources']/Category[@name='DMA']"

	 case "ResourcesForcedHardware"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Hardware Resources']/Category[@name='Forced Hardware']"

	 case "ResourcesIO"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Hardware Resources']/Category[@name='I/O']"

	 case "ResourcesIRQS"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Hardware Resources']/Category[@name='IRQs']"

	 case "ResourcesMemory"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Hardware Resources']/Category[@name='Memory']"

	'---------------------------------
	' REM Subcategories for Components
	'---------------------------------
	case "ComponentsMultimediaAudio"
	    sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Multimedia']/Category[@name='Audio Codecs']"

	case "ComponentsMultimediaVideo"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Multimedia']/Category[@name='Video Codecs']"

	case "ComponentsDisplay"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Display']"

	case "ComponentsInfrared"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Infrared']"

	case "ComponentsKeyboard"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Input']/Category[@name='Keyboard']"

	case "ComponentsPointDev"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Input']/Category[@name='Pointing Device']"

	case "ComponentsModem"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Modem']"

	case "ComponentsNetAdapter"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Network']/Category[@name='Adapter']"

	case "ComponentsNetworkProtocol"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Network']/Category[@name='Protocol']"

	case "ComponentsNetworkWinSock"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Network']/Category[@name='WinSock']"

	case "ComponentsSerialPorts"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Ports']/Category[@name='Serial']"

	case "ComponentsParallelPorts"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Ports']/Category[@name='Parallel']"

	case "ComponentsStorageDrives"			
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Storage']/Category[@name='Drives']"

	case "ComponentsStorageDisks"			
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Storage']/Category[@name='Disks']"

	case "ComponentsStorageSCSI"			
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Storage']/Category[@name='SCSI']"

	case "ComponentsStorageIDE"			
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Storage']/Category[@name='IDE']"

	case "ComponentsPrinting"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Printing']"

	case "ComponentsProblemDevices"	
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Problem Devices']"

	case  "ComponentsUSB"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='USB']"

	case  "ComponentsMultimediaCDROM"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='CD-ROM']"

	case  "ComponentsMultimediaSound"
		sXpathQuery = "//MsInfo/Category/Category[@name='Components']/Category[@name='Sound Device']"

	'-----------------------------------------
	' REM Subcategory for Software Environment
	'-----------------------------------------
	'
	case "SWEnvDrivers"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='System Drivers']"

	case "SWEnvSignedDrivers"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Signed Drivers']"

	case "SWEnvEnvVars"		
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Environment Variables']"

	case "SWEnvPrint"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Print Jobs']"

	case "SWEnvNetConn"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Network Connections']"

	case "SWEnvRunningTasks"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Running Tasks']"

	case "SWEnvLoadedModules"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Loaded Modules']"

	case "SWEnvServices"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Services']"

	case "SWEnvProgramGroup"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Program Groups']"

	case "SWEnvStartupPrograms"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Startup Programs']"

	case "SWEnvOLEReg"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='OLE Registration']"

	case "SWEnvWindowsError"
		sXpathQuery = "//MsInfo/Category/Category[@name='Software Environment']/Category[@name='Windows Error Reporting']"

	case "IEsummary"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Summary']"

		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IEFileVersions"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='File Versions']"

		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IEConnectivity"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Connectivity']"

		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IECacheSummary"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Cache']/Category[@name='Summary']"

		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IECacheObjectList"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Cache']/Category[@name='List of Objects']"

		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IEContentSummary"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Content']/Category[@name='Summary']"
		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IEContentPersonalCertificates"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Content']/Category[@name='Personal Certificates']"
		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IEContentOtherPeopleCertificates"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Content']/Category[@name='Other People Certificates']"
		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IEContentPublishers"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Content']/Category[@name='Publishers']"
		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	case "IESecurity"
		sXpathQuery = "//MsInfo/Category/Category[@name='Internet Settings']/Category[@name='Internet Explorer']/Category[@name='Security']"
		if isVista=true  then
			sCategoryAvailible = false
		else 
		end if 

	'TODO
	 case Else
	   msgbox "Unknown sSubcategory" + sSubcategory
	   ' TODO return error here
	end select

End function