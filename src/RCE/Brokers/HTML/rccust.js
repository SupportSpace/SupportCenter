/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  rcexp.js
///
///  Remote Control customer's side logic
///
///  @author Kirill Solovyov @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

/// relative path and name of installation packege, for example: "scripts/tools/bin/SupportSpaceTools.msi"
var	g_relativeMsi="SupportSpaceTools.msi";
/// code base of BrokerProxy object
var versionFolder="bin";
/// version.js
var SSToolsProductCode="{1BBCEFD3-486C-480d-B536-52E5F4BF9E99}";
/// stargate variable
var shuttleVersion="4.0.48.6";

/// BrokerProxy object
var g_brokerProxy=null;
// first installation reload
var reloadRegExp=/#7777dstSvcId([-]{0,1}[0-9]{1,})rId([-]{0,1}[0-9]{1,})srvType([-]{0,1}[0-9]{1,})/;
// set when reload need for show Install dialog again
var g_reloadIsNeeded=false;

/// Initialization page
function InitPage()
{
	InitUI();
	UIInto();
	coBrokerInit();
}

// it's called by onload page
function coBrokerInit()
{
	if(reloadRegExp.exec(document.location.href)!=null)
	{
		//inject BrokerProxy and send service request approved
		ApprovedServiceRequest(gCurrentUser.jabberUserName,g_RC_BSVCID_JS,gPartnerUser.jabberUserName,RegExp.$1,RegExp.$2, g_RC_BRT_SERVICE|g_RC_BRT_RESPONSE,g_RC_BRR_APPROVED,RegExp.$3);
	}
}

// initialize new session
function InitSession(relaySrv,sId,userId,passwd,remoteUserId)
{
	if(g_brokerProxy!=null)
		g_brokerProxy.InitSession(relaySrv,sId,userId,passwd,remoteUserId);
}

// initialization CoBrokerProxy AX object
function InitBrokerProxy(brokerProxy)
{
	g_brokerProxy=brokerProxy;
	if(Prototype.Browser.IE)
	{
		g_brokerProxy.attachEvent("NotifyLogMessage",g_AddLog);
		g_brokerProxy.attachEvent("RequestSent",OnRequestSent);
	}
	g_brokerProxy.Init("../"+versionFolder+"/SupportSpaceTools.msi",shuttleVersion,SSToolsProductCode);
	g_brokerProxy.InitSession( g_relayServer,
	                           g_support_sessionId,
	                           gCurrentUser.jabberUserName,
	                           gCurrentUser.jabberPassword,
	                           gPartnerUser.jabberUserName);
}

// inject CoBrokerProxy object with code base (installation)
function InjectBrokerProxyAndApprovedRC(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	if($('CoBrokerProxy')==null||$('CoBrokerProxy').object==null)
	{
		var reloadParams="#7777dstSvcId"+srcSvcId+"rId"+rId+"srvType"+param;
		if(reloadRegExp.exec(document.location.href)==null)
			document.location.href+=reloadParams;
		else
			document.location.href=document.location.href.replace(reloadRegExp,reloadParams);
		if(g_reloadIsNeeded)
			window.location.reload();//"don't install" pressed button require reload for installation dialog show again
		$('spanCoBrokerProxy').innerHTML="<OBJECT ID=\"CoBrokerProxy\" CLASSID=\"CLSID:08653405-44A9-4E99-9C09-DD00770AAA08\" width=\"50\" height=\"23\" codebase=\""+ versionFolder +"/SupportSpace_tools.dll#version=-1,-1,-1,-1"+"\"></OBJECT>";
	}
	var _dstUserId=dstUserId;
	var _dstSvcId=dstSvcId;
	var _srcUserId=srcUserId;
	var _srcSvcId=srcSvcId;
	var _rId=rId;
	var _rType=rType;
	var _param=param;
	var _params=params;

	///waiting for downloading and installation CoBrokerProxy()
	function WaitInstallationAndApprovedRC()
	{
		if($("CoBrokerProxy")==null||
				$("CoBrokerProxy").readyState!=4) /*Object is not completely initialized*/
		{
			setTimeout(WaitInstallationAndApprovedRC,1000);
		}
		else
		{
			if($("CoBrokerProxy").object==null)
					g_reloadIsNeeded=true;
			else
			{
				InitBrokerProxy($('CoBrokerProxy'));
				ApprovedServiceRequest(_dstUserId,_dstSvcId,_srcUserId,_srcSvcId,_rId,_rType,_param,_params);
			}
		}
	}
	if(!$("CoBrokerProxy")||!$("CoBrokerProxy").object||$("CoBrokerProxy").readyState!=4/*Object is completely initialized*/)
	{
		$("spanChat").innerHTML+="Help message in Yellow bar case<br/>";
	}
	WaitInstallationAndApprovedRC();
}


// user approved service request
function ApprovedServiceRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	if($('CoBrokerProxy')==null||$('CoBrokerProxy').object==null)
	{
		if(!Prototype.Browser.IE)
		{
			InjectBrokerProxyAndApprovedRC_FF(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		}
		else
		{
			InjectBrokerProxyAndApprovedRC(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		}
	}
	else
	{
		g_brokerProxy.HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,g_RC_BRT_SERVICE|g_RC_BRT_RESPONSE,g_RC_BRR_APPROVED,params+";;"+document.location.href+";;1");
		g_brokerProxy.HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,g_RC_BRT_PROPERTY_LOAD,0,"time");
		g_brokerProxy.HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,g_RC_BRT_PROPERTY_SAVE,0,"time;;"+(new Date()).toString());
	}
}

function InjectBrokerProxyAndApprovedRC_FF(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	g_AddLog("FireFox detected. Please install plugin and then new SupportSpace app will be opened to continue....");

	//$('spanCoBrokerProxy').innerHTML="<embed type=\"application/s2-f2\" id=\"CoBrokerProxy\" width=1 height=1 hidden=\"true\" ><br>";
	$('spanCoBrokerProxy').innerHTML="<object classid=\"@supportspace.com/SupportSpaceToolsProxy,version=7.0.0.1\" type=\"application/s2-f2\" id=\"CoBrokerProxy\" width=1 height=1 hidden=\"true\" ></object><br>";
	//$('spanCoBrokerProxy').innerHTML="<OBJECT ID=\"CoBrokerProxy\" CLASSID=\"CLSID:08653405-44A9-4E99-9C09-DD00770AAA08\" width=\"0%\" height=\"0%\" codebase=\""+ versionFolder +"/SupportSpace_tools.dll#version=6,0,630,7"+"\"></OBJECT>";
	InitBrokerProxy($('CoBrokerProxy'));

	var reloadParams="#7777dstSvcId"+srcSvcId+"rId"+rId+"srvType"+param;
	if(reloadRegExp.exec(document.location.href)==null)
		document.location.href+=reloadParams;
	else
		document.location.href=document.location.href.replace(reloadRegExp,reloadParams);

	g_AddLog("InjectBrokerProxyAndApprovedRC_FF::HandleRequest is called. srcUserId:" +  srcUserId + "dstUserId:" + dstUserId);
	g_brokerProxy.HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,g_RC_BRT_SERVICE|g_RC_BRT_RESPONSE,g_RC_BRR_APPROVED,param+";;"+document.location.href+";;1");

	//g_currentMsg.body='<embed type="application/s2f2-plugin" width=1 height=1 hidden="true"><br> <a href="http://ws-anatoly:8080/home/S2_XPI.xpi" id="installTrigger25114" title="Add BlogRovr to Firefox" addonName="BlogRovr" addonIcon="/en-US/firefox/images/addon_icon/4689" addonHash="sha1:0526f1943a03a09212e2fd245d6b00631b3ccd15" onclick="installIfRequired()" ><span><span><span><strong>Download Now </strong></span></span></span></a></p><input type=button value="ShowVersion" onclick="ShowVersion()">';
	//g_currentMsg.from = PERMISSION_REQUEST;
	//displayMessage(false);
}

function InjectBrokerProxyAndApprovedRC_IE(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	g_AddLog("IE detected");

	$('spanCoBrokerProxy').innerHTML="<OBJECT ID=\"CoBrokerProxy\" CLASSID=\"CLSID:08653405-44A9-4E99-9C09-DD00770AAA08\" width=\"0%\" height=\"0%\" codebase=\""+ versionFolder +"/SupportSpace_tools.dll#version=6,0,630,7"+"\"></OBJECT>";
	InitBrokerProxy($('CoBrokerProxy'));

	var reloadParams="#7777dstSvcId"+srcSvcId+"rId"+rId+"srvType"+param;
	if(reloadRegExp.exec(document.location.href)==null)
		document.location.href+=reloadParams;
	else
		document.location.href=document.location.href.replace(reloadRegExp,reloadParams);

	g_AddLog("InjectBrokerProxyAndApprovedRC_IE::HandleRequest is called. srcUserId:" +  srcUserId + "dstUserId:" + dstUserId);
	g_brokerProxy.HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,g_RC_BRT_SERVICE|g_RC_BRT_RESPONSE,g_RC_BRR_APPROVED,param+";;"+document.location.href+";;1");

	//g_currentMsg.body='<embed type="application/s2f2-plugin" width=1 height=1 hidden="true"><br> <a href="http://ws-anatoly:8080/home/S2_XPI.xpi" id="installTrigger25114" title="Add BlogRovr to Firefox" addonName="BlogRovr" addonIcon="/en-US/firefox/images/addon_icon/4689" addonHash="sha1:0526f1943a03a09212e2fd245d6b00631b3ccd15" onclick="installIfRequired()" ><span><span><span><strong>Download Now </strong></span></span></span></a></p><input type=button value="ShowVersion" onclick="ShowVersion()">';
	//g_currentMsg.from = PERMISSION_REQUEST;
	//displayMessage(false);
}




// received request from jabber
function OnRCRequestRecieved(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
    if(dstUserId==gCurrentUser.jabberUserName)//local
	{
		if(g_RC_BSVCID_JS==dstSvcId)//request to JS
		{
			if(rType==g_RC_BRT_SERVICE)//service request
				HandleServiceRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			else if(rType==g_RC_BRT_SERVICE_DESTROYED)//progress request
				HandleServiceDestroyedRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		}
		else// remote send through broker
			g_brokerProxy.HandleRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
	}
}

// received request from broker
function OnRequestSent(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	//g_AddLog("OnRequestSent("+dstUserId+","+dstSvcId+","+srcUserId+","+srcSvcId+","+rId+","+rType+","+param+","+params+")");
	if(dstUserId==gCurrentUser.jabberUserName)//local
	{
		if(g_RC_BSVCID_JS==dstSvcId)//request to JS
		{
			if(rType==g_RC_BRT_MESSAGEBOX)//message box request
				HandleMessageBoxRequest(null,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			else if(rType==g_RC_BRT_PROGRESS)//progress request
				HandleProgressRequest(null,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			else if(rType==g_RC_BRT_SERVICE_DESTROYED)//progress request
				HandleServiceDestroyedRequest(null,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			else if(rType==g_RC_BRT_SVC_STATE_CHANGED)// service state changed request
				HandleSvcStateChangedRequest(null,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			else if(rType==g_RC_BRT_GET_SESSION_INFO)//session information request
			{
				var info=gCurrentUser.jabberUserName+";;"+gCurrentUser.displayUserName+";;"+gPartnerUser.jabberUserName+";;"+gPartnerUser.displayName;
				g_brokerProxy.HandleRequest(srcUserId,srcSvcId,dstUserId,dstSvcId,rId,rType|g_RC_BRT_RESPONSE,0,info);
			}
			else if(rType==(g_RC_BRT_PROPERTY_LOAD|g_RC_BRT_RESPONSE))
				g_AddLog("LAST INSTANCE CREATION TIME = "+params);
			else if(rType==g_RC_BRT_BROKER_STARTED)//brokerproxy started - close chat session
			  g_brokerProxy.HandleRequest(srcUserId,g_RC_BSVCID_BROKER,srcUserId,g_RC_BSVCID_JS,rId,g_RC_BRT_BROWSER_MINIMIZE,0,"");
		}
	}
	else // remote, send through jabber
		sendRCRequest(gPartnerUser.jabberFullUserName,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
}

function HandleServiceDestroyedRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	$("spanChat").innerHTML+="SERVICE DESTROYED=("+param+","+params+")<br/>";
}

function HandleSvcStateChangedRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	var str="";
	if(param==1)//service is turned on
	{
		var id=document.uniqueID;
		str+="<input id=\""+id+"\" type=button value=\""+params+" stop\" onclick=\";$(\'"+id+"\').outerHTML=\'\'; g_brokerProxy.HandleRequest(\'"+srcUserId+'\',\''+srcSvcId+'\',\''+dstUserId+'\',\''+g_RC_BSVCID_JS+'\',\''+rId+'\',\''+g_RC_BRT_STOP_SERVICE+'\',0,\'\');\" />';
	}
	//alert(params+" STATE="+param+" "+str+" <br/>");
	$("spanChat").innerHTML+=params+" STATE="+param+" "+str+" <br/>";
}

// user approved remote control
function ApprovedRC()
{
	UIInto();
	ApprovedServiceRequest(gCurrentUser.jabberUserName,g_RC_BSVCID_JS,gPartnerUser.jabberUserName,$('uiSvcId').value,0,g_RC_BRT_SERVICE|g_RC_BRT_RESPONSE,g_RC_BRR_APPROVED,"-10");
}


function HandleMessageBoxRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	//param is count of buttons
	var paramsArray=params.split(";;",2+param);
	var buttons = '';
	for(var i=0;i<param;++i)
	{
		var action='disableRequest(this);g_brokerProxy.HandleRequest(\''+srcUserId+'\',\''+srcSvcId+'\',\''+dstUserId+'\',\''+dstSvcId+'\',\''+rId+'\',\''+(g_RC_BRT_MESSAGEBOX|g_RC_BRT_RESPONSE)+'\',\''+i+'\',\'\');';
		buttons+='<div onclick="'+action+'">'+paramsArray[2+i]+'</div>';
	}
	$("spanChat").innerHTML+='<div><div>'+ paramsArray[0] +'</div>\
                                <div>'+ paramsArray[1] +'</div>\
                                <div style="color:blue">'+buttons+'</div></div>';
}

function HandleProgressRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	var progressHeaderId="progressHeader_"+srcSvcId+"_"+rId;
	var progressId="progress_"+srcSvcId+"_"+rId;
	var progress=$(progressId);
	var progressHeader=$(progressHeaderId);
	if(progress)
	{
		progress.innerHTML=param+'% completed';
		progressHeader.innerHTML=params;
	}
	else
	{
		$("spanChat").innerHTML+='<div>\
                              <div id="'+progressHeaderId+'" style="font-weight:bold;">'+params+'</div>\
                              <div id="'+progressId+'" style="background-color:gray;color:white;">'+param+'% completed</div>\
                           </div>';
	}
}

// it send request through jabber
function sendRCRequest(to,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
{
	$("spanRemoteSending").innerHTML+=to+":"+dstUserId+"-"+dstSvcId+"-"+srcUserId+"-"+srcSvcId+"-"+rId+"-"+rType+"-"+param+"-"+params+"</br>";
}

function disableRequest(oElt)
{
	if (!oElt) return;
	oElt.parentNode.style.color=oElt.parentNode.parentNode.style.color;
	oElt.parentNode.innerHTML=oElt.innerHTML+"-ed";
}














///// initialization CoBrokerProxy AX object
//function InitBrokerProxy(brokerProxy)
//{
//	g_brokerProxy=brokerProxy;
//	g_brokerProxy.attachEvent("NotifyLogMessage",g_AddLog);
//	g_brokerProxy.attachEvent("RequestSent",OnRequestSent);
//	g_brokerProxy.Init(g_relativeMsi,g_RCAvailableVersion,g_RCProductCode);
//	g_brokerProxy.InitSession(relaySrv,uiSid.value,uiUser.value,uiPasswd.value,uiRemoteUser.value);
//}

//function InjectBrokerProxyAndApprovedRC()
//{
//	if(g_brokerProxy==null||g_brokerProxy.object==null)
//	{
//		var broker=CoBrokerProxy.outerHTML;
//		var index=broker.search("classid");
//		document.getElementById("spanCoBrokerProxy").innerHTML=broker.substr(0,index)+" codebase=\""+g_codeBase+"\" "+broker.slice(index);
//	}
//	///waiting for downloading and installation CoBrokerProxy()
//	function WaitInstallationAndApprovedRC()
//	{
//		if(	document.getElementById("CoBrokerProxy")==null||
//				document.getElementById("CoBrokerProxy").object==null||
//				document.getElementById("CoBrokerProxy").readyState!=4/*Object is completely initialized*/)
//		{
//			g_AddLog("WaitInstallationAndApprovedRC()");
//			setTimeout(WaitInstallationAndApprovedRC,1000);
//		}
//		else
//		{
//			InitBrokerProxy(document.getElementById("CoBrokerProxy"));
//			ApprovedRC();
//		}
//	}
//	WaitInstallationAndApprovedRC();
//}


//// user approved remote control
//function ApprovedRC()
//{
//	if(g_brokerProxy==null||g_brokerProxy.object==null)
//		InjectBrokerProxyAndApprovedRC();
//	else
//		//g_brokerProxy.HandleRequest(uiUser.value,g_RC_BSVCIDPDV_BROKER,uiRemoteUser.value,g_RC_BSVCIDPDV_JS,0,1,/*g_RC_CONST_BST_RCHOST*/g_RC_CONST_BST_RCVIEWER,"0");
//		g_brokerProxy.HandleRequest(uiRemoteUser.value,uiSvcId.value,uiUser.value,g_RC_BSVCIDPDV_BROKER,0,g_RC_BRT_SERVICE|g_RC_BRT_RESPONSE,g_RC_BRR_APPROVED,"");
//}

//// the fucntion initialize UI
//function InitUI()
//{
//	//+save user and peer id ondeactivate event of editbox
//	uiSid.onfocusout=SaveUI;
//	uiUser.onfocusout=SaveUI;
//	uiPasswd.onfocusout=SaveUI;
//	uiRemoteUser.onfocusout=SaveUI;
//	function SaveUI()
//	{	
//		date=new Date();
//		date.setDate(date.getDate()+365);
//		document.cookie="uiSid="+escape(uiSid.value)+ "; expires=" + date.toGMTString()+"; path=/";
//		document.cookie="uiUser="+escape(uiUser.value)+"; expires=" + date.toGMTString()+"; path=/";
//		document.cookie="uiPasswd="+escape(uiPasswd.value) + "; expires=" + date.toGMTString()+"; path=/";
//		document.cookie="uiRemoteUser="+escape(uiRemoteUser.value) + "; expires=" + date.toGMTString()+"; path=/";
//	}
//	//-save user and peer id

//	//+load saved earlier user and peer id
//	var aCookie = document.cookie.split("; ");
//	for (var i=0; i < aCookie.length; i++)
//	  { 
//		var aCrumb = aCookie[i].split("=");
//		if(aCrumb[0]=="uiSid")uiSid.value=unescape(aCrumb[1]);
//		else if(aCrumb[0]=="uiUser")uiUser.value=unescape(aCrumb[1]);
//		else if(aCrumb[0]=="uiPasswd")uiPasswd.value=unescape(aCrumb[1]);
//		else if(aCrumb[0]=="uiRemoteUser")uiRemoteUser.value=unescape(aCrumb[1]);
//	  }
//	//-load saved earlier user and peer id
//}

//function OnRequest(sid,rid,rtype,params)
//{
//	g_AddLog("OnRequest("+sid+","+rid+","+rtype+",'"+params+"')");
//	switch(rtype)
//	{
//		case 1:
//		case 2:
//				var mode="UNKNOWN";
//				if(params==0)
//					mode="View Only";
//				else if (params==1)
//					mode="Visual Pointer";
//				else if (params==2)
//					mode="Full Control";

//				if(confirm("Expert sent request of RemoteControl on ["+mode+"] mode"))
//					CoBrokerProxy.Respond(sid,rid,rtype,"1");
//				else
//					CoBrokerProxy.Respond(sid,rid,rtype,"0");
//			break;
//		default:
//			g_AddLog("OnRequest(). Unknown type.");
//	}
//}

//function OnRequestSent(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
//{
//	g_AddLog("OnRequestSent("+dstUserId+","+dstSvcId+","+srcUserId+","+srcSvcId+","+rId+","+rType+","+param+","+params+")");
//	if(dstUserId==uiUser.value)//local
//	{
//		if(g_RC_BSVCIDPDV_JS==dstSvcId)//request to JS
//		{
//			if(rType==5)//message box request
//				HandleMessageBoxRequest(null,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
//		}
//	}
//}

//function SendChatMessage(msg)
//{
//	g_brokerProxy.HandleRequest(uiRemoteUser.value,0,uiUser.value,0,0,0,0,msg);
//}

//function HandleMessageBoxRequest(from,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params)
//{
//	//param is count of buttons
//	var paramsArray=params.split(";;",2+param);
//	var buttons="";
//	for(i=0;i<param;++i)
//	{
//		var action='g_brokerProxy.HandleRequest(\''+srcUserId+'\',\''+srcSvcId+'\',\''+dstUserId+'\',\''+dstSvcId+'\',\''+rId+'\',\''+(5|0x80000000)+'\',\''+i+'\',\'\');';
//		buttons+='<input type="button" onclick="'+action+'" value="'+paramsArray[2+i]+'" />';
//	}
//        g_currentMsg = '<span >\
//                                <span>'+ paramsArray[0] +'</span>\
//                                <span>'+ paramsArray[1] +'</span>\
//                                <span>\
//                                  '+buttons+'\
//                                </span>\
//                             </span>';
//	//g_currentMsg.from = '';
//	//displayMessage(false);
//	//alert(g_currentMsg);
//	spanMessageBox.innerHTML=g_currentMsg;
////Buttons is not disabled after use.
//}





