	// CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK 
	// CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK 

		function CALLBACK_StargateOfflineMsgNotification(sData, lMid)
		{
			window.status="CALLBACK_StargateOfflineMsgNotification";
			eCbkEventsHandler.sEvent = 'StargateOfflineMsgNotification';
			eCbkEventsHandler.sData = sData;
			eCbkEventsHandler.lMid = lMid;
			eCbkEventsHandler.fireEvent("onclick");
		}

		function CALLBACK_SetStargateStatus(sStatus, iCallUID){
			window.status="CALLBACK_SetStargateStatus";
			eCbkEventsHandler.sEvent = 'SetStargateStatus';
			eCbkEventsHandler.sStatus = sStatus;
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/******
		 * CALLBACKS_Messenger: Events that occur when Supporter Press Login or Login automatically
		 */ 
		function CALLBACK_SettingsOk(
			cOnIncomingCallsShowTrayMessage,
			cAutomaticallyRun,
			cOpenMainWindowOnWindowsStartUp,
			cShowAway,
			sShowAway,
			cHandleCallsDisplayBusy,
			sHandleCallsDisplayBusy,
			cOnIncomingCallsShowTrayMessage,
			cOnIncomingCallsAnimateTrayIcon,
			cPromptOnItemsOnLogout,
			cPromptAboutSnoozingItemsOnLogout,
			cPlaySoundUponIncomingCall,
			cPlaySoundUponConnectingToCustomer,
			sDisplayItemsAtTime,
			cPortOpened,
			sPortOpened,
			cScreenSaverIsOn
		)
		{
			window.status="CALLBACK_SettingsOk";
			eCbkEventsHandler.sEvent = 'SettingsOk';
			eCbkEventsHandler.cOnIncomingCallsShowTrayMessage = cOnIncomingCallsShowTrayMessage;
			eCbkEventsHandler.cAutomaticallyRun = cAutomaticallyRun;
			eCbkEventsHandler.cOpenMainWindowOnWindowsStartUp = cOpenMainWindowOnWindowsStartUp;
			eCbkEventsHandler.cShowAway = cShowAway;
			eCbkEventsHandler.sShowAway = sShowAway;
			eCbkEventsHandler.cHandleCallsDisplayBusy = cHandleCallsDisplayBusy;
			eCbkEventsHandler.sHandleCallsDisplayBusy = sHandleCallsDisplayBusy;
			eCbkEventsHandler.cOnIncomingCallsShowTrayMessage = cOnIncomingCallsShowTrayMessage;
			eCbkEventsHandler.cOnIncomingCallsAnimateTrayIcon = cOnIncomingCallsAnimateTrayIcon;
			//eCbkEventsHandler.cPromptOnItemsUpdate = cPromptOnItemsUpdate;
			eCbkEventsHandler.cPromptOnItemsOnLogout = cPromptOnItemsOnLogout;
			eCbkEventsHandler.cPromptAboutSnoozingItemsOnLogout = cPromptAboutSnoozingItemsOnLogout;
			eCbkEventsHandler.cPlaySoundUponIncomingCall = cPlaySoundUponIncomingCall;
			eCbkEventsHandler.cPlaySoundUponConnectingToCustomer = cPlaySoundUponConnectingToCustomer;
			eCbkEventsHandler.sDisplayItemsAtTime = sDisplayItemsAtTime;
			eCbkEventsHandler.cPortOpened = cPortOpened;
			eCbkEventsHandler.sPortOpened = sPortOpened;
			eCbkEventsHandler.cScreenSaverIsOn = cScreenSaverIsOn;

			eCbkEventsHandler.fireEvent("onclick");
		}
		/******
		 * CALLBACK_SettingsClose: Events that occur when Supporter Press Login or Login automatically
		 */ 
		function CALLBACK_SettingsClose()
		{
			window.status="CALLBACK_SettingsClose";
			eCbkEventsHandler.sEvent = 'SettingsClose';	
			eCbkEventsHandler.fireEvent("onclick");
		}

		/******
		 * CALLBACKS_Messenger: Events that occur when Supporter Press Login or Login automatically
		 */ 
		function CALLBACK_LoginPageStart(sLoginEmail,sPassword,sLoginRememberMe,sLoginStatus)
		{
			window.status="CALLBACK_LoginPageStart" + sLoginEmail;
			eCbkEventsHandler.sEvent = 'LoginPageStart';

			eCbkEventsHandler.sLoginEmail = sLoginEmail;
			eCbkEventsHandler.sPassword = sPassword;
			eCbkEventsHandler.sLoginRememberMe = sLoginRememberMe;
			eCbkEventsHandler.sLoginStatus = sLoginStatus;

			eCbkEventsHandler.fireEvent("onclick");
		}
		/******
		 * CALLBACKS_Messenger: Events that occur on the Application Messenger 
		 */ 
		function CALLBACKS_Messenger(sEvent)
		{
			window.status="CALLBACKS_Messenger " + sEvent;
			eCbkEventsHandler.sLoginEmail = $('loginEmail').value;//TODO - need here to know what user need Settings page
			eCbkEventsHandler.sEvent = sEvent;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/******
		 * CALLBACKS_Messenger: Events that occur on the Application Messenger 
		 */ 
		function CALLBACK_NewCallDeleted(iCallUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" deleted";
			eCbkEventsHandler.sEvent = 'NewCallDeleted';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		//CALLS
		function CALLBACK_NewCallAdded(iCallUID,sCalls)
		{
			window.status="CALLBACK: In call "+iCallUID+" added";
			eCbkEventsHandler.sEvent = 'NewCallAdded';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.sCalls = sCalls;
			eCbkEventsHandler.fireEvent("onclick");
		}
		//FLAGS
		/*******
		 * CALLBACK_FlagChanged: This function is called every time that the flag is changed
		 * 
		 * @param {int} iCallUID
		 * @param {string} sFlagStatus - the color name of the flag (e.g. "Red") or "Completed" or "Cleared"
		 */
		function CALLBACK_FlagChanged(iCallUID,sFlagStatus)
		{
			window.status="CALLBACK: In call "+iCallUID+" flag changed to "+sFlagStatus;
			eCbkEventsHandler.sEvent = 'FlagChanged';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.sFlagStatus = sFlagStatus;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/*******
		 * CALLBACK_GoToManageFlags: This function is called when manage flags is selected
		 * @param {int} iCallUID
		 */
		function CALLBACK_GoToManageFlags(iCallUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" manage flags was selected";
			eCbkEventsHandler.sEvent = 'ManageFlags';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}
		
		// FORWARD
		/*******
		 * CALLBACK_Snooze: This function is called when manage contacts is selected
		 * @param {int} iCallUID
		 */
		function CALLBACK_FwdToContact(iCallUID,iCustID)
		{
			window.status="CALLBACK: In call "+iCallUID+" forward to contact "+iCustID+" was selected";
			eCbkEventsHandler.sEvent = 'FwdToContact';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.iCustomerUID = iCustID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/*******
		 * CALLBACK_GoToManageContacts: This function is called when manage contacts is selected
		 * @param {int} iCallUID
		 */
		function CALLBACK_GoToManageContacts(iCallUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" manage contacts was selected";
			eCbkEventsHandler.sEvent = 'ManageContacts';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		// SNOOZE
		/*******
		 * CALLBACK_Snooze: This function is called when snooze is selected
		 * @param {int} iCallUID
		 * @param {int} iSec
		 */
		function CALLBACK_Snooze(iCallUID,iSec)
		{
			window.status="CALLBACK: In call "+iCallUID+" snooze: "+iSec;
			eCbkEventsHandler.sEvent = 'Snooze';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.iSec = iSec;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/******
		 * CALLBACK_GoToCalender: This function is called when calender is selected
		 * @param {int} iCallUID
		 */
		function CALLBACK_GoToCalender(iCallUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" calender was selected";
			eCbkEventsHandler.sEvent = 'GoToCalender';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		// REPLY
		/*******
		 * CALLBACK_SendReply: This function is called when a predefined reply is selected
		 * @param {int}		iCallUID
		 * @param {string}	sReply
		 * @param {int}		iReplyUID
		 */
		function CALLBACK_SendReply(iCallUID,iReplyUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" replyID: "+iReplyUID;
			eCbkEventsHandler.sEvent = 'SendReply';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.iReplyUID = iReplyUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/*******
		 * CALLBACK_SendCustomReply: This function is called when a custom reply is selected
		 * @param {int} iCallUID
		 * @param {string} sReply
		 */
		function CALLBACK_SendCustomReply(iCallUID,sReply)
		{
			window.status="CALLBACK: In call "+iCallUID+" custom reply: "+sReply;
			eCbkEventsHandler.sEvent = 'SendCustomReply';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.sCustomReply = sReply;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/*******
		 * CALLBACK_GoToManageReplies: This function is called when manage replies is selected
		 * @param {int} iCallUID
		 */
		function CALLBACK_GoToManageReplies(iCallUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" manage replies was selected";
			eCbkEventsHandler.sEvent = 'GoToManageReplies';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/*****
		 * CALLBACK_GoToCustomerProfile: This function is called when the link  
		 * to customer is selected and should go to the customer profile page 
		 * @param {int} iCallUID
		 * @param {int} iCustomerUID
		 */
		function CALLBACK_GoToCustomerProfile(iCallUID,iCustomerUID)
		{
			window.status="CALLBACK: In call "+iCallUID+" customer profile was selected";
			eCbkEventsHandler.sEvent = 'CustomerProfile';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.iCustomerUID = iCustomerUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/*****
		 * CALLBACK_GoToPickupCall : This function is called when the pickup button is pressed
		 * @param {int} iCallUID - the User ID
		 */
		function CALLBACK_PickupCall(iCallUID, sCustomerName)
		{
			window.status="CALLBACK: In call "+iCallUID+" pickup was selected";
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.sDisplayUserName = sCustomerName;
			eCbkEventsHandler.sEvent = 'pickup';
			eCbkEventsHandler.fireEvent("onclick");
		}

		// ALERT
		/******
		 * CALLBACK_CloseAlert: Events that occur on the Alert
		 */ 
		function CALLBACKS_Alert(sEvent)
		{
			window.status="CALLBACKS_Alert" + sEvent;
			eCbkEventsHandler.sEvent = sEvent;
			eCbkEventsHandler.fireEvent("onclick");
		}
		
        /***
         * User clicks on the alert
         */
		function CALLBACK_ClickAlert(iCallUID)
		{
			window.status="CALLBACKS_Alert onclick";
			eCbkEventsHandler.sEvent = "onclick";
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

        /***
         * User clicks on Expand button
         */
		function CALLBACK_Expand(iCallUID)
		{
			window.status="CALLBACKS_Alert expandAlert";
			eCbkEventsHandler.sEvent = 'expandAlert';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}
		
        /***
         * User close the Alert
         */
		function CALLBACK_Close(iCallUID)
		{
			window.status="CALLBACKS_Alert close";
			eCbkEventsHandler.sEvent = 'closeAlert';
			eCbkEventsHandler.iCallUID = iCallUID;
			eCbkEventsHandler.fireEvent("onclick");
		}

        /***
         * A new version is available
         */
		function CALLBACK_UpdateVersion(sProductVersion)
		{
			window.status="CALLBACK_NewVersionAvailible";
			eCbkEventsHandler.sEvent = 'updateVersion';
			eCbkEventsHandler.sProductVersion = sProductVersion;
			eCbkEventsHandler.fireEvent("onclick");
		}

        /***
         * The request status has changed
         */
		function CALLBACK_RequestStatusChanged(supportRequestId,state,bStayOnline4Customer,sCustomerName)
		{
			window.status="CALLBACK_RequestStatusChanged";
			eCbkEventsHandler.sEvent = 'requestStatusChanged';
			eCbkEventsHandler.iCallUID = supportRequestId;
			eCbkEventsHandler.sState = state;
			eCbkEventsHandler.bStayOnline4Customer = bStayOnline4Customer;
			eCbkEventsHandler.sDisplayUserName = sCustomerName;
			eCbkEventsHandler.fireEvent("onclick");
		}
        /***
         * The page has been loaded
         */
        function CALLBACK_PageLoaded()
		{
			eCbkEventsHandler.sEvent = 'pageLoaded';
			eCbkEventsHandler.fireEvent("onclick");
		}
        
        function CALLBACK_CloseMessenger()
        {
            eCbkEventsHandler.sEvent = 'closeMessenger';
			eCbkEventsHandler.fireEvent("onclick");
        }
        
        function CALLBACK_OpenLink(link)
        {
            eCbkEventsHandler.sEvent = 'openLink';
            eCbkEventsHandler.sLink = link;
			eCbkEventsHandler.fireEvent("onclick");
        }
        
        function CALLBACK_SettingsTest()
        {
            eCbkEventsHandler.sEvent = 'connectionTest';
			eCbkEventsHandler.bUseCustomPort = $('cPortOpened').checked;
            eCbkEventsHandler.iCustomPort = $('sPortOpened').value;
            eCbkEventsHandler.bUPnP = $('cPortAutoconfigure').checked;
			eCbkEventsHandler.fireEvent("onclick");
        }
        
        /**** CONSULT EVENTS ****/
       	function CALLBACK_NewConsultAdded(id, sData)
		{
			eCbkEventsHandler.sEvent = 'NewConsultRequestAdded';
			eCbkEventsHandler.iCallUID = id;
            eCbkEventsHandler.sCalls = sData;
			eCbkEventsHandler.fireEvent("onclick");
		}
        function CALLBACK_ClickConsultAlert(workflowID, sDisplayName)
		{
			eCbkEventsHandler.sEvent = "startConsultAlert";
			eCbkEventsHandler.iWorkflowID = workflowID;
			eCbkEventsHandler.sDisplayUserName = sDisplayName;
			eCbkEventsHandler.fireEvent("onclick");
		}
        function CALLBACK_ConsultStatusChanged(consultId,state)
		{
			eCbkEventsHandler.sEvent = 'consultStatusChanged';
			eCbkEventsHandler.iCallUID = consultId;
			eCbkEventsHandler.sState = state;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/***********************************************************************************
         *   Js can use this function to write to Debug View instead of using alert()
		 *   sLogType can be: MESSAGE, WARNING, ERROR
		 */
		function CALLBACK_AddDhtmlLog(sLogString, sLogType)
		{
			eCbkEventsHandler.sEvent = 'addDhtmlLog';
			eCbkEventsHandler.sLogString = sLogString;
			eCbkEventsHandler.sLogType = sLogType;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/***********************************************************************************
         *   Js can use this function to show simple MessageBox with Title:SupportCenter and OK button
		 *   sString - text to show in the message box
		 */
		function CALLBACK_ShowMessageBox(sMboxText)
		{
			eCbkEventsHandler.sEvent = 'showMessageBox';
			eCbkEventsHandler.sMboxText = sMboxText;
			eCbkEventsHandler.fireEvent("onclick");
		}

		/***********************************************************************************
         *   Js can use this function to show simple MessageBox with Title:SupportCenter and OK button
		 *   sString - text to show in the message box
		 */
		function CALLBACK_ReadOfflineMsg()
		{
			eCbkEventsHandler.sEvent = 'pickup';
			eCbkEventsHandler.fireEvent("onclick");
		}

		function  CALLBACK_InfoMsgReportIssue()
		{
			eCbkEventsHandler.sEvent = 'ReportIssue';
			eCbkEventsHandler.fireEvent("onclick");
		}
		
		function  CALLBACK_InfoMsgReadMore()
		{
			eCbkEventsHandler.sEvent = 'ReadMore';
			eCbkEventsHandler.fireEvent("onclick");
		}
        
	// CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK 
	// CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK CALLBACK 
