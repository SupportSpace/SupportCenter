//-------------------------
//   SHUTTLE TOOLS
//-------------------------

/// service identifier predefined values
var g_RC_BSVCID_BROKER=-256;        /// request sends to Broker/VBroker
var g_RC_BSVCID_JS=-255;            /// request sends to JS
var g_RC_BSVCID_AUTOSET=-254;       /// must be set by request handler  (for service it do by Broker/VBroker)

/// Brokers requests results
var g_RC_BRR_ERROR=-256;            /// any errors //TODO spesify more
var g_RC_BRR_UNKNOWN=-255;          /// request type is not unknown
var g_RC_BRR_DECLINED=0;            /// request is declined
var g_RC_BRR_APPROVED=1;            /// request is approved
var g_RC_BRR_BUSY=2;                /// request handler busy
var g_RC_BRR_BPFAILED=3;            /// BrokerProxy ActiveX object creation failed (for example in case when user push "don't install" button)

/// Brokers Requests types
var g_RC_BRT_RESPONSE=0x80000000;    /// respons on any request, it mark original request as responce via bitwise OR operation (for exapmle: BRT_SERVICE|BRT_RESPONSE - responce on service request
var g_RC_BRT_PING = 0;               /// ping request - blank request. it is used for connection establishment (for example).
var g_RC_BRT_CHAT_MESSAGE = 2;       /// chat message; params="[Message]";
var g_RC_BRT_SERVICE = 3;            /// service creation request param=srvType, params="[Caption text];;[Message text];;[approve button text];;[decline button text];;[Access Mode]"
var g_RC_BRT_PROGRESS = 4;           /// progress reqeust param=percent completion params="message"
var g_RC_BRT_MESSAGEBOX = 5;         /// message box show request param=count buttons params="[Caption text];;[Message text];;[button1 text];;[button2 text];;...;;[buttonN text]"
var g_RC_BRT_INSTALLATION = 7;       /// installation progress reqeuest param percent params="[Message text]"
var g_RC_BRT_SERVICE_DESTROYED = 11; /// service destroyed iformation request param=srvId params="[remote svcId]"
var g_RC_BRT_SVC_STATE_CHANGED = 12; /// service state has been changed (for example DS becom turned ON or OFF). param = new_state_id, params = "Service name"
var g_RC_BRT_GET_SESSION_INFO = 14;  /// get information about session param=reserved params="[userId];;[UserName];;[remoteUserId];;[remoteUserName]"
var g_RC_BRT_STOP_SERVICE = 19;      /// asks service to stop. no respond. no params
var g_RC_BRT_BROKER_STARTED=20;      /// notification request of new BrokerProxy instance started in case using separate process
var g_RC_BRT_PROPERTY_LOAD=22;       /// property load request params=[property name], response params=[property name;;property value]
var g_RC_BRT_PROPERTY_SAVE=23;       /// property save request params=[property name;;property value]
var g_RC_BRT_BROWSER_MINIMIZE=24;    /// minimize browser request. send to Broker by js


///// Available services types
//g_RC_CONST_BST_VBROKER=0;    ///Visual broker service (expert's side)
//g_RC_CONST_BST_BROKER=1;       ///Broker service (customer's side)
//g_RC_CONST_BST_INSTALLER=2;    ///Installer service
//g_RC_CONST_BST_RCVIEWER=3;     ///this name can be changed by implementing developers ONLY BY INITIAL TIME
//g_RC_CONST_BST_RCHOST=4;       ///this name can be changed by implementing developers ONLY BY INITIAL TIME
//g_RC_CONST_BST_FAVIEWER=5;     ///this name can be changed by implementing developers ONLY BY INITIAL TIME
//g_RC_CONST_BST_FAHOST=6;        ///this name can be changed by implementing developers ONLY BY INITIAL TIME

function $(elementName){return document.getElementById(elementName);}

var Prototype = {
  Version: '1.6.0',

  Browser: {
    IE:     !!(window.attachEvent && !window.opera),
    Opera:  !!window.opera,
    WebKit: navigator.userAgent.indexOf('AppleWebKit/') > -1,
    Gecko:  navigator.userAgent.indexOf('Gecko') > -1 && navigator.userAgent.indexOf('KHTML') == -1,
    MobileSafari: !!navigator.userAgent.match(/Apple.*Mobile.*Safari/)
  },

  BrowserFeatures: {
    XPath: !!document.evaluate,
    ElementExtensions: !!window.HTMLElement,
    SpecificElementExtensions:
      document.createElement('div').__proto__ &&
      document.createElement('div').__proto__ !==
        document.createElement('form').__proto__
  },

  ScriptFragment: '<script[^>]*>([\\S\\s]*?)<\/script>',
  JSONFilter: /^\/\*-secure-([\s\S]*)\*\/\s*$/,

  emptyFunction: function() { },
  K: function(x) { return x }
};

/// the function add messages to logger. It must initialized logger owner. (CServiceRCInstaller.Init())
var g_AddLog=function(){alert("g_AddLog are not initialized!");}

/// initialization of logging
g_AddLog=OnNotifyLogMessage;

/// hard-coded IP address of the relay server
var g_relayServer = "192.168.0.66";

///log event handler
function OnNotifyLogMessage(message,severity)
{	var strseverity="";
	if(severity==null||severity==undefined)
		severity=-1;//js message
	switch(severity)
	{	
		case 0: strseverity="_MES ";break;
		case 1: strseverity="_WAR ";break;
		case 2: strseverity="_ERR ";break;
		case 3: strseverity="_EXC ";break;
		case 4: strseverity="_UTS ";break;
		case 5: strseverity="_UTC ";break;
		case 6: strseverity="_FTS ";break;
		case 7: strseverity="_FTC ";break;
		case -1: strseverity="_JS_ ";break;
		default:strseverity="unkn ";
	}
	//logger.innerText=strseverity+message+"\n"+logger.innerText;
	var maxLengthLog=10000;//32768;
	var oldLog=$('logger').innerHTML;
	if(oldLog.length>maxLengthLog)
	{
		oldLog=oldLog.slice(0,oldLog.lastIndexOf("<br>",maxLengthLog-1));
	}
	$('logger').innerHTML=strseverity+message+"<br>"+oldLog;
}

var g_support_sessionId = "default";
var gPartnerUser = {jabberUserName:"default",
                    jabberFullUserName:"default",
                    displayName:"default"}
var gCurrentUser = {id:"unknown",
                    displayUserName:"default",
                    jabberUserName:"default",
                    jabberPassword:"supportspace-web"}

function $(element){return document.getElementById(element);}


///setup internal variable by UI value
function UIInto()
{
	g_support_sessionId = $('uiSid').value;
	gPartnerUser.jabberUserName=$('uiRemoteUser').value;
	gPartnerUser.jabberFullUserName=$('uiRemoteUser').value;
	gPartnerUser.displayName=$('uiRemoteUser').value;
	gCurrentUser.displayUserName=$('uiUser').value;
	gCurrentUser.jabberUserName=$('uiUser').value;
	gCurrentUser.jabberPassword=$('uiPasswd').value;
}

// the fucntion initialize UI
function InitUI()
{
	//+save user and peer id ondeactivate event of editbox
	$('uiSid').onfocusout=SaveUI;
	$('uiUser').onfocusout=SaveUI;
	$('uiPasswd').onfocusout=SaveUI;
	$('uiRemoteUser').onfocusout=SaveUI;
	function SaveUI()
	{	
		date=new Date();
		date.setDate(date.getDate()+365);
		document.cookie="uiSid="+escape($('uiSid').value)+ "; expires=" + date.toGMTString()+"; path=/";
		document.cookie="uiUser="+escape($('uiUser').value)+"; expires=" + date.toGMTString()+"; path=/";
		document.cookie="uiPasswd="+escape($('uiPasswd').value) + "; expires=" + date.toGMTString()+"; path=/";
		document.cookie="uiRemoteUser="+escape($('uiRemoteUser').value) + "; expires=" + date.toGMTString()+"; path=/";
	}
	//-save user and peer id

	//+load saved earlier user and peer id
	var aCookie = document.cookie.split("; ");
	for (var i=0; i < aCookie.length; i++)
	  { 
		var aCrumb = aCookie[i].split("=");
		if(aCrumb[0]=="uiSid")$('uiSid').value=unescape(aCrumb[1]);
		else if(aCrumb[0]=="uiUser")$('uiUser').value=unescape(aCrumb[1]);
		else if(aCrumb[0]=="uiPasswd")$('uiPasswd').value=unescape(aCrumb[1]);
		else if(aCrumb[0]=="uiRemoteUser")$('uiRemoteUser').value=unescape(aCrumb[1]);
	  }
	//-load saved earlier user and peer id
}