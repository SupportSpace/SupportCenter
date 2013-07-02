	// INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE 
		
   	var gFlags = [];
	var gReplies = [];
	var gSnoozes = [];
	var gContacts = [];
    var gCurrentCallItem = new CallItem();
	var gCallItemsList = [];
    var gConsultCall = null;
        
	function INTERFACE_UpdateData(sSubject,sData,p1,p2,p3)
	{
   		switch (sSubject) 
		{
            case gSubject.InitTexts:
                initTexts(sData);
            break;
			case gSubject.Flags:
				SetFlags(sData);
			break;
			case gSubject.Replies:
				SetReplies(sData);
			break;
			case gSubject.Snoozes:
				SetSnoozes(sData);
			break;
			case gSubject.Contacts:
				SetContacts(sData);
			break;
			case gSubject.AddCalls:
                addCalls(sData);
            break;
            case gSubject.AddPendingCalls:
                gCallItemsList = [];
                //dropCallsUI();    
				addCalls(sData);
			break;
			case gSubject.DelCalls:
				deleteCalls(sData);
			break;
			case gSubject.UpdateVersion:
				updateVersion(sData);
			break;
			case gSubject.RequestNotification:
				requestStatusChanged(sData);
			break;
            case gSubject.LoginInfo:
                setLoginInfoUI(p1,p2);
            break;
            case gSubject.Logout:
                setLogoutUI(p1);
			break;
            case gSubject.Settings:
                setSettingsUI(sData);
            break;
            case gSubject.SettingsClose:
                SettingsClose();
            break;
            case gSubject.LoginStart:
                OnLoginStartClicked()
            break;
            case gSubject.LoginState:
                setLoginState(p1);
            break;
			case gSubject.HelpAbout:
				setHelpAboutUI(p1);
			break;
			case gSubject.AlertCall:
				alertCall(sData,p1);
			break;
            case gSubject.AlertUpgrade:
				alertUpgradeUI(p1);
			break;
            case gSubject.AlertMissedCalls:
				alertMissedCallUI(p1);
			break;
            case gSubject.ConnectionTest:
                setConnectionTestStateUI(p1,p2);
            break;
            case gSubject.ConsultCall:
                setConsultCall(sData); // In messenger IE control from stargate
            break;
            case gSubject.AlertConsult:
                setAlertConsultCall(sData); // In alert IE control from shuttle
            break;
			case gSubject.OpenWindow:
				openWindow(p1, p2, p3);
				break;
			case gSubject.StargateBusy:
				setStargateStatus(sSubject, null);
				break;
			case gSubject.StargateOnline:
				setStargateStatus(sSubject, null);
				break;
			case gSubject.StargateBusyLock:
				setStargateStatus(sSubject, sData);
				break;
			case gSubject.StargateOnnlineForCustomer:
				setStargateStatus(sSubject,null);
				break;

			case gSubject.StargateOfflineMsgNotification:
			case gSubject.StargateSystemMsgNotification: //the same actions
				setStargateOfflineMsgNotification(sSubject, sData); 
				break;

			case gSubject.AlertOfflineMsgNotification:
			case gSubject.AlertSystemMsgNotification://the same actions
				alertOfflineMsgUI(sData);
				break;

			case gSubject.AlertInfoMsgNotification:
				alertInfoMsgUI(p1);
				break;
		}

		//CALLBACK_AddDhtmlLog("INTERFACE_UpdateData with sSubject" + sSubject , "MESSAGE");
	}

	function setStargateOfflineMsgNotification(sSubject, sData)
	{
		var sDataFixed = "(" + sData + ")";//eval requires () for object or [] for array.		
		var oData = eval(sDataFixed);
		var lMid;

		if(oData.supportMessageId!=undefined)
			lMid = oData.supportMessageId;
		else
			lMid = 0;

		CALLBACK_StargateOfflineMsgNotification(sData, lMid);
	}

	function setStargateStatus(sStatus, sData)
	{
		var sDataJson;
		var iCallID = 0;
	
		if(sData!=null){
			sDataJson = eval(sData);
			iCallID = sDataJson[0].id; 
		}

		CALLBACK_SetStargateStatus(sStatus, iCallID);
	}

	
	function openWindow(sUrl, bThreaterMode, sSessionId)
	{
		var winWidth=self.screen.availWidth - 10;
		var winHeight=self.screen.availHeight - 40;
		try{
			CALLBACK_AddDhtmlLog("openWindow() started." + sSessionId , "MESSAGE");
			var win = window.open(sUrl, 'desktop_'+sSessionId,'menubar=0,toolbar=0,status=0,resizable=1,location=0,top=0,left=0,width='+ winWidth +',height='+winHeight);

			// if win indicates error then we have to try with CALLBACK_OpenLink(sUrl);
			// note. this can be case like popup blocker prevented 
			// See http://srv-dev/jira/browse/STL-256 for possible error discussion

			if(win == null)
			{ 
				CALLBACK_AddDhtmlLog("openWindow() failed with bad return value. Probably iexplore process opened first on this PC hang up" + sSessionId , "ERROR");
			    CALLBACK_OpenLink(sUrl);
			}

		}catch(e)
		{
		    CALLBACK_AddDhtmlLog("openWindow() failed with exception. Probably iexplore process opened first on this PC hang up" + sSessionId , "WARNING");
			CALLBACK_OpenLink(sUrl);
		}

		CALLBACK_AddDhtmlLog("openWindow() completed." + sSessionId , "MESSAGE");
	}
	
	function SetFlags(sFlags)
	{	
		gFlags = eval(sFlags);
	}
	
	function SetReplies(sReplies)
	{
		gReplies = eval(sReplies);
	}
	
	function SetSnoozes(sSnoozes)
	{
		gSnoozes = eval(sSnoozes);			
	}
	
	function SetContacts(sContacts)
	{
		gContacts = eval(sContacts);
	}
    		
	var gTestDebugCalls; 		
	function testAddCall()
	{
		var iCallUID = Math.floor(Math.random()*100);

		gTestDebugCalls = '[{\
					"submissionDate":{"month":2,"day":4,"year":107,"time":1173359990111,"seconds":50,"timezoneOffset":-120,"date":8,"hours":15,"minutes":19},\
					"problemDescription":"My cat ate my pc",\
					"supportRequestSubmissionMode":"DIRECT",\
					"state":"NEW",\
					"customer":{"accountNonExpired":true,"registrationDate":null,"accountNonLocked":true,"province":null,"firstName":"jhon","birthdate":null,"authorities":[],"id":0,"nickName":"","postcode":"","confirmPassword":"","username":"customer1","jabberUsername":"","city":null,"gender":"M","password":"1234","passwordHint":"","street":"","roles":[],"version":0,"lastName":"snow","credentialsNonExpired":true,"country":null,"useNickname":false,"enabled":true,"fullName":"jhon snow'+iCallUID+'","email":"","phoneNumber":"","unregistrationDate":null,"website":""},\
					"id":'+iCallUID+'\
				}]';

		//addCalls(gTestDebugCalls)
	}
	
	function addCalls(sCalls)
	{
    	gCurrentCallItem = new CallItem();	// NOAM - not sure who is the current call item?
		var aCalls = eval(sCalls);
		if (aCalls.length < 1)
			return;
			
		gCurrentCallItem.getCall(aCalls[0]);

		for (var i=0;i<aCalls.length;i++)
		{
			gCallItemsList.push(new CallItem());
			gCallItemsList[gCallItemsList.length-1].getCall(aCalls[i]);

//			if (i==0)
//				gCurrentCallItem.getCall(aCalls[0]);

/*			if(typeof(addCallUI)=='function')
				addCallUI(gCallItemsList[gCallItemsList.length-1],sCalls);*/
			CALLBACK_NewCallAdded(gCallItemsList[gCallItemsList.length-1].iCallUID,sCalls);
		}
	}

	function alertCall(sCalls, callID)
	{
		var aCalls = eval(sCalls);
		for (var i=0;i<aCalls.length;i++)
		{
			gCallItemsList.push(new CallItem());
			gCallItemsList[gCallItemsList.length-1].getCall(aCalls[i]);
		}
		gCurrentCallItem = CallItem.getCallItemById(callID);
	}
    
	function testDeleteCall()
	{
		//deleteCalls(gTestDebugCalls);	
		requestStatusChanged(gTestDebugCalls);
	}
	
	function deleteCalls(sCalls)
	{
		var aCalls = eval(sCalls);
        
		for (var i=0;i<aCalls.length;i++)
		{
			for (var j=0;j<gCallItemsList.length;j++)
			{
				if (aCalls[i].id == gCallItemsList[j].iCallUID)
                    gCallItemsList.splice(j,1)
			}				
			deleteCallUI(aCalls[i].id);
		}
	}

	function updateVersion(sData)
	{
		var sDataFixed = "(" + sData + ")";//eval requires () for object or [] for array.
		OnUpdateVersion(sDataFixed);	
	}
	
	function requestStatusChanged(sRequest)
	{
		var sRequestFixed = "(" + sRequest + ")";//eval requires () for object or [] for array.
		requestStatusChangedUI(sRequestFixed)
	}
/*
	function INTERFACE_GetOpenCloseStatus()
	{
		return gRequestItemIsOpen;
	}
*/    
    function setConsultCall(sData)
    {
        var sDataFixed = "(" + sData + ")"; //eval requires () for object or [] for array.
        var oConsultCall = eval(sDataFixed);
        if (oConsultCall.state == 'PENDING')
        {
           if (oConsultCall)
                gConsultCall = oConsultCall
           CALLBACK_NewConsultAdded(gConsultCall.id,sData);
        }
        else
        {
            CALLBACK_ConsultStatusChanged(oConsultCall.id,oConsultCall.state);
        }
    }
    
//	 INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE INTERFACE 
 