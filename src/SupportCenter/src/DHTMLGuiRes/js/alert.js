function alertBodyOnLoad()
{
    if (gConsultCall)
        setConsultCallUI(gConsultCall)
    if (gCurrentCallItem && !gCurrentCallItem.isEmpty())
        setCustomerCallUI(gCurrentCallItem)
    
    CALLBACK_PageLoaded(); // Tell to Shuttle that the page is loaded
}

function setAlertConsultCall(sData)
{
   var sDataFixed = '('+sData+')'
   var oConsultCall = eval(sDataFixed);
   if (oConsultCall)
        gConsultCall = oConsultCall;        
}

function setConsultCallUI(oConsultCall)
{
    $("dAlertOriginName").innerHTML="<nobr>"+oConsultCall.originator.displayUserName+"</nobr><br>";
	$("dAlertCallText1").innerHTML='Expert consultation request.';
	$("dAlertIcon").className="dAlertIconConsult";//STL-268
}

function setCustomerCallUI(oCustomerCall)
{
	$("dAlertIcon").className="dAlertIconSession";//STL-270
    $("dAlertOriginName").innerHTML="<nobr>"+oCustomerCall.oCustomer.sCustomerName+"</nobr><br>";

	if(oCustomerCall.resumeSession==false || oCustomerCall.resumeSession == undefined){
		$("dAlertCallText1").innerHTML=oCustomerCall.sCallProblemDescription+"<br>";
		$("dAlertCallText1").title = oCustomerCall.sCallProblemDescription
	    //$("dAlertCallText2").innerHTML="<nobr>"+oCustomerCall.sCallCategoryPath+"</nobr><br>";
	}
	else{
		var sResumeTxt = "Resume session request."
		$("dAlertCallText1").innerHTML=sResumeTxt+"<br>";
		$("dAlertCallText1").title = sResumeTxt;
	}
}

function alertClicked(id)
{
    CALLBACK_ClickAlert(id);
	pickupButtonClicked();
}

/*function expandAlert()
{
	CALLBACK_Expand(gCurrentCallItem.iCallUID);
}*/

function closeAlert()
{
	// Send to the "Client" a "Alert Clicked" event
    if (gConsultCall)
		CALLBACK_Close(gConsultCall.workflowId);
    else    
	    if (gCurrentCallItem && !gCurrentCallItem.isEmpty())
			CALLBACK_Close(gCurrentCallItem.iCallUID);
		else
			CALLBACK_Close(-1); // Update
}
// pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup .............

function pickupButtonClicked()
{
	// Send to the "Client" a "Alert Clicked" event
    if (gConsultCall)
        CALLBACK_ClickConsultAlert(gConsultCall.workflowId, gConsultCall.originator.displayUserName);//display name of Expert asked consult
        
    if (gCurrentCallItem && !gCurrentCallItem.isEmpty())
		CALLBACK_PickupCall(gCurrentCallItem.iCallUID, gCurrentCallItem.oCustomer.sCustomerName); //display name of Customer 
}

function alertUpgradeUI()
{
    $('dAlertOriginName').innerHTML = 'Upgrade Notice'
    $('dAlertIcon').hide();
    $('dAlertRegular').hide();
    $('dAlertUpgrade').show();    
}

function alertMissedCallUI(sAlertText)
{
    $('dAlertOriginName').innerHTML = 'Missed Calls Notice'
    $('dAlertIcon').hide();
    $('dAlertRegular').hide();
    $('dAlertUpgrade').hide();    
    $('dAlertMissedCalls').show();
	$("dAlertMissedCallsText").innerHTML = sAlertText + '<span style="cursor:pointer;text-decoration:underline">click here</span>';
	$("dAlertMissedCallsText").title = sAlertText + 'click here';
}

function alertInfoMsgUI(sAlertText)
{
    $('dAlertOriginName').innerHTML = 'Info Message'
	$("dAlertIcon").className="dAlertIconInfoMsg";
	$("dAlertInfoMsgText").innerHTML = "Your connection quality may be too poor to provide service";

	$('dAlertRegular').hide();
    $('dAlertUpgrade').hide();    
    $('dAlertMissedCalls').hide();
	$("dAlertOfflineMsg").hide();

    $('dAlertInfoMsg').show();
}

//http://srv-filer/confluence/display/Stargate/Stargate+Shuttle+Integration
//"notificationDisplayedText":"You have received a new message from <Customer Name>. Click here to read.",
//"supportMessageId": <id of support message>
//"supportMessageType":"offlineMessage"/"systemMessage"
function alertOfflineMsgUI(sData)
{
    var sDataFixed = "(" + sData + ")";//eval requires () for object or [] for array.		
	var oData = eval(sDataFixed);
	var sText;

    $("dAlertOriginName").innerHTML= 'New Messages';
/*
	if(oData.numberOfUnreadMessages==1)
		sText = "You have 1 new message";
	else
		sText = "You have " +  oData.numberOfUnreadMessages + " new messages";
*/
	sText = oData.notificationDisplayedText;

	$("dAlertOfflineMsgText").innerHTML = sText;
	
	$("dAlertIcon").className="dAlertIconOfflineMsg";

	$("dAlertOfflineMsg").show();

	$('dAlertRegular').hide();
    $('dAlertUpgrade').hide();    
    $('dAlertMissedCalls').hide();
}

function alertOfflineMsgClicked(bSelected)
{
	if(bSelected==true)
		CALLBACK_ReadOfflineMsg();
	else
		CALLBACK_Close(-1); 
}

function alertInfoMsgReadMore()
{
	//alert("alertInfoMsgReadMore");
	CALLBACK_InfoMsgReadMore();	
}

function alertInfoMsgReportIssue()
{
	//alert("alertInfoMsgReportIssue");
	CALLBACK_InfoMsgReportIssue();
}