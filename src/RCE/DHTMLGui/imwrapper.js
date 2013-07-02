/////////////////////////////////////////////////////////////////////////
///
///  Archer Software,Ltd
///
///  imwrapper.js
///
///  java script,  java script part of GUI Remote Control
///
///  @author Dmitry Netrebenko @date 12.12.2006
///
////////////////////////////////////////////////////////////////////////

// Connection settings for Jabber server
//var im_http_base = new String("JHB/");
var im_http_base = new String("http://213.8.114.131:18080/jwchat/JHB/");
//var im_http_base = new String("JHB/");
var im_timeout = 2000;
//var im_domain = new String("supportspace.com");
var im_domain = new String("fwtest");
var im_res = new String("jsjac_simpleclient");

///  Processes message from jabber server
///  @param message
function IMHandleMessage(aJSJaCPacket) 
{
	// Add message to internal queue
	var str = new String(aJSJaCPacket.getFrom());
	var pos = str.indexOf('@');
	if (pos >= 0)
	{
		str = str.substring(0,pos);
	}
	$('IMMediatorObject').HandleMsgInternal(str, aJSJaCPacket.getBody());
}

function IMHandleEvent(aJSJaCPacket) 
{
/// NOT USED
}

function IMHandlePresence(aJSJaCPacket)
{
/// NOT USED
}


function IMHandleDisconnected()
{
	document.getElementById('userid').disabled = false;
	document.getElementById('password').disabled = false;
	document.getElementById('btnlogin').disabled = false;
	document.getElementById('bsnew').disabled = true;

	alert('You were disconnected from Jabber server');
}


///  Disables some controls
function IMHandleConnected() 
{
	document.getElementById('userid').disabled = true;
	document.getElementById('password').disabled = true;
	document.getElementById('btnlogin').disabled = true;
	document.getElementById('bsnew').disabled = false;

	con.send(new JSJaCPresence());
}

///  Processes errors
function IMHandleError(e) 
{
	// TODO: handle error e.firstChild.nodeName;
	AddLogMessage((new Date()).toLocaleTimeString()+"> IMHandlerError   "+e.firstChild.nodeName,0);
}

///  Login to jabber server
function IMLogin()
{

	try 
	{
		// setup args for connection's contructor
		oArgs = new Object();
		oArgs.httpbase = im_http_base;
		oArgs.timerval = im_timeout;

		con = new JSJaCHttpBindingConnection(oArgs);

		con.registerHandler('message',IMHandleMessage);
		con.registerHandler('onconnect',IMHandleConnected);
		con.registerHandler('onerror',IMHandleError);
		con.registerHandler('presence',IMHandlePresence);
		con.registerHandler('iq',IMHandleEvent);
		con.registerHandler('ondisconnect',IMHandleDisconnected);


		// setup args for connect method
		oArgs = new Object();
		oArgs.domain = im_domain;
		oArgs.username = document.getElementById('userid').value;
		oArgs.resource = im_res;
		oArgs.pass = document.getElementById('password').value;
		con.connect(oArgs);
	} 
	catch (e) 
	{
		// TODO: handle error e.toString();
		AddLogMessage((new Date()).toLocaleTimeString()+"> IMLogin   "+e.description,0);
		return;
	}
	//hostObject.statNotifyLogMessage((new Date()).toLocaleTimeString()+"> IMLogin",4);
}

///  Logout from jabber server
function IMLogout()
{
	con.disconnect();

}


///  OnSendMessage event handler
///  @param peerid
///  @param message text
function OnIMSendMessage(peer,msg)
{
	var aMsg = new JSJaCMessage();
	var to = peer+"@"+im_domain;
	aMsg.setTo(to);
	aMsg.setBody(msg);
	if(con!=null)
		con.send(aMsg);
}

function OnLoadPage()
{
}

function OnUnloadPage()
{
	if (typeof(con) != 'undefined' && con.disconnect) 
		con.disconnect();
	//DeinitPage();
}
