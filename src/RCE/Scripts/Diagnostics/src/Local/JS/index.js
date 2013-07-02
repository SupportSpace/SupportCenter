var g_bInit = false;
var g_sCurrentCategoryId;
var g_bRunFirstTime = "1";
var g_bCustomerGrantedPermissions = false;
var g_bEventsAttached = false;
var g_bWaitingForCustomerAproval = false;

function DeployAndRunScriptOnRemoteMachine(categoryId)
{
	g_sCurrentCategoryId = categoryId;

	if(g_bInit==false)
	{
		// to init host and deploy script once
		Init();
	}
	else
	{
		if(g_bWaitingForCustomerAproval==true) //probably OnUnexpectedError happened
		{
			updateInfoPaneStatus(gInfoPaneMessage.WaitingForCustomerAproval);
			UpdateStatusBar(gStatusBarMessage.WaitingForCustomerAproval, "");
		}

		// host inited and diag script deployed
		StartRemoteScript();
	}
}

function Init()
{
	g_bInit = true;
	g_bWaitingForCustomerAproval = true;
	AttachEvents();

	//
	updateInfoPaneStatus(gInfoPaneMessage.WaitingForCustomerAproval);
	UpdateStatusBar(gStatusBarMessage.WaitingForCustomerAproval, "");

	// todo here http://srv-dev/jira/browse/STL-483
	// "Retrieve" button is not grayed out when waiting for customer approval.
	theRetrieveButton.setDisabled(true);

	// 
	ScriptEngineClient.InitHost();
	
	//
	ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.ScriptEngineClient.InitHost called");
}

//  AttachEvents may be called once
function AttachEvents()
{
	if(g_bEventsAttached==false)
	{
		ScriptEngineClient.attachEvent("OnHostInited",OnHostInited);
		ScriptEngineClient.attachEvent("OnFilesTransferred", OnFilesTransferred); // No matter when to attach event, you can attach right before starting deploy
		ScriptEngineClient.attachEvent("OnDeployComplete", OnDeploy);
		ScriptEngineClient.attachEvent("OnProgress", OnProgress);
		ScriptEngineClient.attachEvent("OnUnexpectedError", OnUnexpectedError);
		g_bEventsAttached = true;
	}
}

function OnFilesTransferred(success,err)
{
	if (success)
	{
		ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.OnFilesTransferred success");
	}
	else
	{
		ScriptEngineClient.WriteLogError("DiagnosticsWidget.OnFilesTransferred err:" + err);
		UpdateStatusBar(gStatusBarMessage.FailedToRetrieveResults, ":" + err);
	}
}

function OnExecComplete(success, err, params, obj)
{
	theRetrieveButton.setDisabled(false);

	if (success)
	{
		g_bWaitingForCustomerAproval = false;
		UpdateStatusBar(gStatusBarMessage.RemoteScriptExecutionCompleted, "");

		switch(params)
		{
			case "NotAvailibleOnVista":
				ScriptEngineClient.WriteLogWarning("DiagnosticsWidget.OnExecComplete success" + params);
				UpdateStatusBar(gStatusBarMessage.InternetCategoryNotAvailibleOnVista);
				updateInfoPaneStatus(gStatusBarMessage.InternetCategoryNotAvailibleOnVista);
				OnRemoteInitFailed();
				return;
			case "CantRetrieveInfoFromNonEnglishOS":
				ScriptEngineClient.WriteLogWarning("DiagnosticsWidget.OnExecComplete success" + params);
				UpdateStatusBar(gStatusBarMessage.CantRetrieveInfoFromNonEnglishOS);
				updateInfoPaneStatus(gStatusBarMessage.CantRetrieveInfoFromNonEnglishOS);
				OnRemoteInitFailed();
				return;
			case "CantRetrieveInfoUnknownReason":
				ScriptEngineClient.WriteLogWarning("DiagnosticsWidget.OnExecComplete success" + params);
				UpdateStatusBar(gStatusBarMessage.CantRetrieveInfoUnknownReason);//todo
				updateInfoPaneStatus(gStatusBarMessage.CantRetrieveInfoUnknownReason);
				OnRemoteInitFailed();
				g_bRunFirstTime = "1"; //if execution failed then on Vista we runn all again
				return;
			default:
				ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.OnExecComplete success");//do not print params - too big exception
				OnRemoteDataRetrieved(g_sCurrentCategoryId, params);
				break;
		}
	}
	else
	{
		ScriptEngineClient.WriteLogError("DiagnosticsWidget.OnExecComplete failed with code:" + err);
		g_bRunFirstTime = "1"; //if execution failed then on Vista we runn all again
		g_bInit = true; //TODO for test only
		UpdateStatusBar(gStatusBarMessage.RemoteScriptExecutionFailed, ":" +err);
		OnRemoteInitFailed();
	}
}

function StartRemoteScript()
{
	try{
		//
		//	SystemSummary will be retrieved using WMI on both Vista and XP with different OS-system language
		//  then we have SystemSummary fast on Vista and working on non-english OS-system language
		//
		if(g_sCurrentCategoryId!="SystemSummary")
		{
			if(g_OSystem=="Vista" && g_bRunFirstTime=="1")
			{
				updateInfoPaneStatus(gInfoPaneMessage.Refreshing);
				UpdateStatusBar(gStatusBarMessage.VistaFirstTimeSlowWarning,"");
			}

			if(g_OSystem==null && g_bRunFirstTime=="1")
			{
				updateInfoPaneStatus(gInfoPaneMessage.Refreshing);
				UpdateStatusBar(gStatusBarMessage.FirstTimeRecommendationWarning,"");
				ScriptEngineClient.WriteLogWarning("DiagnosticsWidget. FirstTimeRecommendationWarning shown");
				//force first time SystemSummary 
				//g_sCurrentCategoryId = "SystemSummary";
				//g_ItemIdRequestedFromRemote = "SystemSummary"; 
				//g_bRunFirstTime = "0";
				//ScriptEngineClient.InvokeFile('VBScript' /*lang*/,'diag.vbs' /*file*/,'BuildJsonFileByDiagCategoryRemote' /*method*/, g_sCurrentCategoryId, /*params*/"0",'OnExecComplete' /*completion routine*/);
				//return;
			}

			ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.InvokeFile to retrieve CategoryID:" + g_sCurrentCategoryId);
			ScriptEngineClient.SetTimeout(300000); // 5 minutes timeout required on Vista 
			ScriptEngineClient.InvokeFile('VBScript' /*lang*/,'diag.vbs' /*file*/,'BuildJsonFileByDiagCategoryRemote' /*method*/, g_sCurrentCategoryId, /*params*/g_bRunFirstTime,'OnExecComplete' /*completion routine*/);
			g_bRunFirstTime = "0";
		}
		else
		{
			ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.InvokeFile SystemSummary");
			ScriptEngineClient.SetTimeout(300000); // 5 minutes timeout required on Vista 
			ScriptEngineClient.InvokeFile('VBScript' /*lang*/,'diag.vbs' /*file*/,'BuildJsonFileByDiagCategoryRemote' /*method*/, g_sCurrentCategoryId, /*params*/"0",'OnExecComplete' /*completion routine*/);
		}
	}
	catch(e)
	{
		//  fix STL-680. and all errors may be also send in callback OnExecComplete with error. then do nothing here  
		ScriptEngineClient.WriteLogError("DiagnosticsWidget.StartRemoteScript exception" + e);
	}
}

function OnDeploy(success,err)
{
	if(success)
	{
		ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.OnDeploy success");
		UpdateStatusBar(gStatusBarMessage.ScriptDeployCompleted, "");
		StartRemoteScript();
	}
	else
	{
		UpdateStatusBar(gStatusBarMessage.DeployFailed, ":" + err);
		ScriptEngineClient.WriteLogError("DiagnosticsWidget.OnDeploy failed" + err);
		theRetrieveButton.setDisabled(false);
	}
}

function OnHostInited(success,err)
{
	theRetrieveButton.setDisabled(false);

	if(success)
	{
		ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.OnHostInited success");
		UpdateStatusBar(gStatusBarMessage.RemoteHostInitiated, "");
		ScriptEngineClient.Deploy();
	}
	else
	{
		g_bInit = false;
		g_bWaitingForCustomerAproval = false;

		ScriptEngineClient.WriteLogError("DiagnosticsWidget.OnHostInited failed" + err);
		UpdateStatusBar(gStatusBarMessage.OriginalRemoteHostInitFailed, err);		
		OnRemoteInitFailed();
	}
}

function OnProgress(string) 
{
	ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.OnProgress " + string);

	switch(string)
	{
		case "Customer granted permission":
			//fix this workaround in the future...currently the only one string in switch
			UpdateStatusBar(gStatusBarMessage.CustomerGrantedPermission, "");
			g_bCustomerGrantedPermissions = true;
			g_bWaitingForCustomerAproval = false;
		break;
		default:
			UpdateStatusBar(gStatusBarMessage.InstallationProgress, string); //installing ....
		break;
	}
}

function OnUnexpectedError(errorCode, errorMessage)
{
	ScriptEngineClient.WriteLogMessage("DiagnosticsWidget.OnUnexpectedError" + errorMessage);
	UpdateStatusBar(gStatusBarMessage.ServiceStopedByCustomer);
	theRetrieveButton.setDisabled(false);
	g_bWaitingForCustomerAproval = true; //will need customer aproval next time
}
