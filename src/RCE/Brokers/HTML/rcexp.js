/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  rcexp.js
///
///  Remote Control expert's side logic
///
///  @author Kirill Solovyov @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////

/// last started service
var g_lastStartedService=-1;

/// Initialization page
function InitPage()
{
	if(CoVBroker==null||CoVBroker.object==null)
	{
		g_AddLog("CoVBroker has not created");
	}
	CoVBroker.attachEvent("NotifyLogMessage",g_AddLog);
	CoVBroker.attachEvent("RequestSent",OnRequestSent);
	InitUI();
	UIInto();
	var srvVBArray=CoVBroker.GetAvailableServices();
	var services=srvVBArray.toArray();
	g_AddLog("AvailableService="+services.toString());
	for(i=0;i<services.length;++i)
	{
		var oOption=document.createElement("OPTION");
		srvTypeSelect.options.add(oOption);
		oOption.innerText=services[i];
		oOption.value=i;
	}
	//if(g_lastStartedService>0)
		//StartFeature(g_lastStartedService);
}

// the function start feature using UI settings
function StartFeature(serviceNumId)
{
	UIInto();
	var id=document.uniqueID;
	ServicesHosts.outerHTML+="<div id=\""+id+"\" STYLE=\"width: 100%; height: 90%\"></div>";
	var svcId;
	svcId=$('CoVBroker').StartToolService(g_relayServer,
                                         g_support_sessionId,
                                         gCurrentUser.jabberUserName,
                                         gCurrentUser.jabberPassword,
                                         gPartnerUser.jabberUserName,
                                         serviceNumId,$(id));
	$(id).insertAdjacentHTML("afterBegin","<div onclick=\"CoVBroker.StopToolService("+svcId+");$('"+id+"').removeNode('false');\">Close</div>");
	//g_viewerId=CoVBroker.StartToolService(relaySrv,uiSid.value,uiUser.value,uiPasswd.value,uiRemoteUser.value,serviceNumId,document.getElementById(id));
	//g_viewerId=CoVBroker.StartToolService(relaySrv,uiSid.value,uiUser.value,uiPasswd.value,uiRemoteUser.value,g_RC_CONST_BST_RCVIEWER,ViewerHostObject);
	g_AddLog("StartToolService="+svcId);
	//ViewerHostObject\n height="+ViewerHostObject.style.height+" overflow="+ViewerHostObject.style.overflow +" "+ViewerHostObject.style.overflowY);
	g_lastStartedService=serviceNumId;
	date=new Date();
	date.setDate(date.getDate()+365);
	document.cookie="lastStartedService="+escape(g_lastStartedService) + "; expires=" + date.toGMTString()+"; path=/";
}


function SendChatMessage(msg)
{
	//CoVBroker.Respond(uiSid.value,0,0,msg);
	CoVBroker.HandleRequest(uiRemoteUser.value,0,uiUser.value,0,0,0,0,msg);
	CoVBroker.HandleRequest(uiUser.value,g_viewerId,uiUser.value,0,0,0,0,msg);
}


// it's called when request was sent by VBroker
function OnRequestSent(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
    if(dstUserId==gCurrentUser.jabberUserName)//local
    {
        if(g_RC_BSVCID_JS==dstSvcId)//request to JS
        {
            if(rType==g_RC_BRT_GET_SESSION_INFO)//session information request
            {
                var info=gCurrentUser.jabberUserName+";;"+gCurrentUser.displayUserName+";;"+gPartnerUser.jabberUserName+";;"+gPartnerUser.displayName;
                $('CoVBroker').HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,rType|g_RC_BRT_RESPONSE,0,info);
            }
        }
    }
    else // remote, send through jabber
      sendRCRequest(gPartnerUser.jabberFullUserName,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
}

// it send request through jabber
function sendRCRequest(to,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	$("spanRemoteSending").innerHTML+=to+":"+dstUserId+"-"+dstSvcId+"-"+srcUserId+"-"+srcSvcId+"-"+rId+"-"+rType+"-"+param+"-"+params;
}
