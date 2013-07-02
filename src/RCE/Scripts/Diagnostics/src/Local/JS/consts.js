/* consts.js : Constant data for all */ 

/**
 * @author noame Anatoly  Gutnick
 */

var gInfoPaneMessage = {
    PressRetrieveToStart:"Click the Retrieve button to start.",
    SelectSubcategory:"Select a subcategory",
	Refreshing:"Refreshing system info...",
	WaitingForCustomerAproval:""
}

var gStatusBarMessage = {
	PressRetrieveToStart:
		{ text:"Click the Retrieve button to start.", color:"gray"},
    SelectSubcategory:
		{ text:"Select a subcategory.", color:"gray"},
	Refreshing:
		{ text:"Refreshing system info...", color:"green"},
	WaitingForCustomerAproval:
		{ text:"Waiting for customer approval...", color:"blue"},
	WarningWaitCompletion:
		{ text:"Retrieving previous subcategory. Please wait...", color:"blue"},
	RequestesdDataRetrieved:
		{ text:"Requested info retrieved", color:"green"},
	DataRequestedBeforeRetrieved:
		{ text:"Previous subcategory retrieved.", color:"green"},
	StartedRetrievingFromWaitingList:
		{ text:"Retrieving next subcategory.", color:"green"},
	LocalDataRetrieved:
		{ text:"Click Retrieve at any time to refresh info.", color:"green"},
	CustomerDeclinedRequest:
		{ text:"", color:"orange"},
	CustomerGrantedPermission:
		{ text:"Customer granted permission.", color:"blue"},
	RemoteHostInitiated:
		{ text:"Remote host initiated", color:"blue"},
	RemoteHostInitFailed:
		{ text:"Remote host initialization failed. Code#", color:"orange"},
	RemoteScriptExecutionFailed:
		{ text:"Remote execution failed. Code#", color:"orange"},
	RemoteScriptExecutionCompleted:
		{ text:"Remote execution completed", color:"green"},
	DeployFailed:
		{ text:"Installation failed. Code#", color:"orange"},
	FailedToRetrieveResults:
		{ text:"Failed to retrieve requested info. Code#", color:"orange"},
	ScriptDeployCompleted:
		{ text:"Successful installation. Retrieving, please wait... ", color:"blue"},
	InstallationProgress:
		{ text:"", color:"blue"},
	InternetCategoryNotAvailibleOnVista:
		{ text:"The Internet category is not available on Vista", color:"green"},
	CantRetrieveInfoFromNonEnglishOS:
		{ text:"Cannot retrieve info from the customer's non-english operating system", color:"orange"},
	ServiceWaiting:
		{ text:"", color:"orange"},
	OriginalRemoteHostInitFailed:
		{ text:"", color:"orange"},
	VistaFirstTimeSlowWarning:
		{ text:"Retrieving info from Vista. First time this will take several minutes...", color:"blue"},
	FirstTimeRecommendationWarning:
		{ text:"Retrieving info. It is recommended to retrieve System Summary category first...", color:"blue"},
	RequestesdDataRetrievedOSLanguageWarning:
		{ text:"System Summary retrieved. Cannot retrieve other categories from non-english OS:", color:"blue"},
	CantRetrieveInfoUnknownReason:
		{ text:"Cannot retrieve info. Customer canceled or Unknwon error", color:"orange"},
	ServiceStopedByCustomer:
		{ text:"The service was stopped by the customer", color:"orange"}
}
