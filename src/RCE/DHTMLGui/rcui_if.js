////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  rcui.js
///
///  java script,  java script part of GUI Remote Control
///
///  @author "Archer Software" Solovyov K. @date 29.11.2006
///
////////////////////////////////////////////////////////////////////////
function $(obj){return document.getElementById(obj);}
//function $$(frame,obj){return document.frames(frame).document.getElementById(obj);}

function TrimString(str)
{	if(str=="")return "";
	var i,j;
	for(i=0;i<str.length&&str.charAt(i)==' ';++i);//trim left
	if(i==str.length)return "";//blank string. white space only.
	for(j=str.length-1;j>=0&&str.charAt(j)==' ';--j);//trim right
	return str.substring(i,j+1);
}

/// dynamic include js files, something like #include in C++
function MyInclude(jsfile)
{
	document.write("<script language=\"JavaScript\" type=\"text/javascript\" src=\""+jsfile+"\"></script>");
}



/*@cc_on @*/
/*@set @_debug=true @*/
/*@if (@_debug)@*/
   //alert("debug");
/*@end@*/

//------------------------------------------------------------------------------------------
//			global variable (properties of window object)
//------------------------------------------------------------------------------------------

/// hard-coded IP address of the relay server
//var relayServer='192.168.0.135';
var relayServer='213.8.114.131';
/// jabber includs
var relayJabber="jabberscripts/";
/// relative installation script name. for example "bin/RCComponents.msi";
var relativeMsi="bin/SupportSpaceTools.msi";
/// codebase value for CRCInstallerAXCtrl activeX object
//var relayCodebase="http://213.8.114.131:18080/jwchat/integ_v2_8/bin/Bootstrapper.exe#version=1,0,1,14";
//var relayCodebase="http://213.8.114.131:18080/jwchat/integ_v2_8/bin/Bootstrapper.exe";

/// Current session
var currentSession=null;
/// Sessions (associative) array
var sessions = new Object();
/// the singleton host activeX object <object>(CRCHostAXCtrl)
//var hostObject=new CreateObject('RCUI.RCHostAXCtrl');TODO thus is beautiful, but i don't know how attach event handler
var hostObject=null;
/// the singleton installer activeX object <object> (CRCInstallerAXCtrl)
var installerObject=null;
/// the function add messages to logger. It must initialized logger owner. (CServiceRCInstaller.Init())
var AddLogMessage=function(){alert("AddLogMessage are not initialized!");}
/// page install mask
var pageInstallMask=0;
///  pointer of CService object, which connect are establishing now. For one connect establish process only.
var g_connectingService=null;

/// Array with find method
Array.find = function (key) 
{
};

/// local strings array
var localStrings = new Object();
localStrings["1"] = "Getting external IP";
localStrings["2"] = "Retriving initial message from remote peer";
localStrings["3"] = "Waiting for TCP and UDP connections";
localStrings["4"] = "Connect aborted";
localStrings["5"] = "Starting NAT traversal connect";
localStrings["6"] = "Trying connect to peer through relay server";
localStrings["7"] = "Connect through relayed stream";
localStrings["8"] = "Trying connect to peer through relay server through proxy";
localStrings["9"] = "Connect through relayed through proxy";
localStrings["10"] = "Waiting while previous connect terminates...";
localStrings["11"] = "Failed to SetTimer. Connect timeout won't be handled";
localStrings["12"] = "Connected";
localStrings["13"] = "Connect timeout expired";
localStrings["14"] = "Getting external IP through proxy";
localStrings["15"] = "Sending initial message to remote peer";
localStrings["16"] = "GetHandleMsgEvent returned invalid event handle";
localStrings["17"] = "ReSending initial message to remote peer";
localStrings["18"] = "Receiving initial message timed out";
localStrings["19"] = "Connect directly";
localStrings["20"] = "Connect through UDP";
localStrings["21"] = "Connect failed";
localStrings["22"] = "Starting direct connect to";
localStrings["23"] = "Starting direct connect through proxy to";

/// Localizes string
/// @param string string to localize
/// @return localized string
LocalizeString=function(string)
{
    try
    {
        var re = /\d+/;
        var key = re.exec(string);
        for (var keys in localStrings) 
        {
            if (keys == key) 
              return string.replace(re, localStrings[key]);
         }
    }
    catch(e)
    {
        try
        {
            AddLogMessage((new Date()).toLocaleTimeString()+"> Failed to localize string "+string+" due to "+e.description);
        }
        catch(e)
        {
        }
    }
    return string;
}

/// set of services. it is filled in admin
var servicesSet=new Object();
//servicesSet['RCViewer']=new CServiceInfo("RCUI.RCViewerAXCtrl",0,"RemoteControl","",0x0);
servicesSet['RCHost']=new CServiceInfo("RCUI.RCHostAXCtrl",1,"RemoteControl","",0x1);
servicesSet['RCPlayback']=new CServiceInfo("RCUI.RCViewerAXCtrl",2,"RemoteControl","",0x1);
servicesSet['RCFileAccessClient']=new CServiceInfo("FTUI.IFileAccessClientX",3,"FileAccess","",0x0);
servicesSet['RCFileAccessHost']=new CServiceInfo("FTUI.IFileAccessHost",4,"FileAccess","",0x0);
servicesSet['RCInstaller']=new CServiceInfo("RCInstaller.RCInstallerAXCtrl",5,"RCInstaller","",0x0);
// this is not service. for installation only.
//servicesSet['SupportMessenger']=new CServiceInfo("SupportMessenger",6,"SupportMessenger","",0x0);
//servicesSet['NetLogViewer']=new CServiceInfo("NetLogViewer",7,"NetLogViewer","",0x0);
//servicesSet['RCViewerDlg']=new CServiceInfo("RCUI.RCViewerAXDlg",8,"RemoteControl","",0x0);
servicesSet['RCViewerPlus']=new CServiceInfo("RCUI.RCViewerAXCtrl",9,"RemoteControl","",0x0);
/// CServiceInfo class - information of available service
/// param@ name service name
/// param@ type feature internal type type=0 RCViewer; type=1 RCHost; type=2 Playback;type=3 RCFileAccessClient; type=4 RCFileAccessHost; type=5 RCInstaller;
/// param@ fname feature name
/// param@ guid service GUID
function CServiceInfo(name,type,fname,guid,level)
{
	///	service name
	this.m_name=name;
	/// feature internal type type=0 RCViewer; type=1 RCHost; type=2 Playback;type=3 RCFileAccessClient; type=4 RCFileAccessHost; type=5 RCInstaller;
	this.m_type=type;
	/// feature name
	this.m_fname=fname;
	/// service GUID
	this.m_guid=guid;
	/// feature version
	//this.m_version=version;
	/// currently required feature version
	//this.m_reqVersion=reqVersion;
	/// install map of service
	this.m_installMap=level; 
}



//------------------------------------------------------------------------------------------
//			page functions
//------------------------------------------------------------------------------------------

// The iframes using contain complications in itself: 
// 1. Incorrect iframes contents reloading (F5 refresh, for example) - IE load other location different from iframe::src property.
//	Resolve: Before other action in body::onload event, all iframes location is checked and correct via checkIframe() js global function.
//	This function compares iframe::src property and iframe::window.document.location.href property. If they is not equal, 
//	iframe::window.ducument.location.href set to ifram::src value.
// 2. The iframes contents load asynchronously. The body::onload event may be fire, but not all iframes loadings are completed.
//	Resolve: In beginning of body::onload the iframes loading waiting has been added. OnLoadIframe() function is advised 
//	to iframe::onload event of all iframes on the page. The function decrement unloaded iframes counter's value (unloadedIframes).

/// unloaded iframes counter. It is used for synchronous onload event handle, after loading all iframes
var unloadedIframes=9;

/// The funtion check iframe::src and real location of iframe. if thay are not equal, set location to iframe::src value
/// @param iframe iframe html object
/// @return if location has been not changed return zero value, if it has been changed return one value.
//	TODO may be endless loop
function checkIframe(iframe)
{
	var ssrc=iframe.src;
	var shref=iframe.contentWindow.document.location.href;
	if(shref.search(ssrc)==-1)
	{
		iframe.contentWindow.document.location.href=ssrc;
		return 1;
	}
	else
		return 0;
}

/// The function decrement unloaded iframes counter value. Advised to onload event all iframes on the page
function OnLoadIframe()
{	
	--unloadedIframes;
}

function SystemInformation()
{
	//http://msdn2.microsoft.com/en-us/library/ms537503.aspx
	var str="";
	for(var prop in window.navigator)
		str+=prop+"="+window.navigator[prop]+"\n";
	str+="--------------------------\n";		
	for(var prop in window.clientInformation)
		str+=prop+"="+window.clientInformation[prop]+"\n";
	return str;
}


/// initialize page function
/// @param installMask page install mask, installMask=0x1 client, installMask=0x2 support
function InitPage(installMask)
{
	
	//alert(SystemInformation());
	//return;
	// correct location of iframs
	if(unloadedIframes==-1)
	{
		unloadedIframes=0;// correct iframes location no more 
		for(i=document.frames.length-1;i>=0;--i)
		{
			unloadedIframes+=checkIframe(document.frames(i).frameElement);//repair iframe location 
		}
	}
	// repaired iframes loading waiting
	if(unloadedIframes>0)
	{
		setTimeout(InitPage,1000);
		return;
	}
	
	if(installMask!=undefined&&installMask!=null)
		pageInstallMask=installMask;
	else 
		pageInstallMask=0;
	
	// AddToTrusted title
	if($("SSInitialInstaller").object==null)
		return;
	$("spanAddToTrusted").style.display="none";
	$("spanInstallation").innerText="RCInstaller is updating now. Please Wait ...";
	// for display by IE
	setTimeout(RCInstallerInstallation,1);
	function RCInstallerInstallation()
	{
		if (CheckIfComponentUpdateNeeded(RCInstallerComponentCode,RCInstallerAvailableVersion,$("SSInitialInstaller")))
		{
			var res=0;
			if (IsMajorUpdateNeeded(RCInstallerComponentCode,$("SSInitialInstaller")))
			{
				res=$("SSInitialInstaller").Install(1,relativeMsi,"ADDLOCAL=RCInstaller REINSTALLMODE=vomus /qn /L+ Install.log",0x0f000001 /* 0x01 - is synchronous call. See SSInstaller.h, Install() for details*/);
				if(res!=0)
					alert("SSInitialInstaller(ADDLOCAL=RCInstaller)="+res);

			} 
			else
			{
				res=$("SSInitialInstaller").Install(1,relativeMsi,"REINSTALL=RCInstaller REINSTALLMODE=vomus /qn /L+ Install.log",0x0f000001 /* 0x01 - is synchronous call. See SSInstaller.h, Install() for details*/);
				if(res!=0)
					alert("SSInitialInstaller(REINSTALL=RCInstaller)="+res);

			}
		}
		
		$("spanInstallerObject").innerHTML="<object id=\"RCInstallerObject\" classid=\"clsid:7B3BBD75-A77C-40D9-BD0E-943055093249\" width=\"0\" height=\"0\"> </object>	<object id=\"IMMediatorObject\" classid=\"clsid:3FE73D88-72F5-4526-A106-FAA12DE9A619\" width=\"0\" height=\"0\"> </object> <script language=\"JavaScript\">	$('IMMediatorObject').attachEvent(\"OnSendMessage\", OnIMSendMessage);</script>"
		if($('RCInstallerObject').object==null)
		{
			alert("InitPage(). RCInstallerObject wasn't created");
			return;
		}			
		$('IMMediatorObject').attachEvent("OnSendMessage", OnIMSendMessage);			
		$("spanInstallation").removeNode(true);
		// initializing
		Initializing();
	}
	function Initializing()
	{
		//creating installer session, which initialize RCInstallerObject. (attach event, getting GUID installed service, etc.)
		NewSession("Init","","Installer");
		sessions["InitInstaller"].m_service.AfterInstallAction=function()
		{
			CloseSession("InitInstaller");
		
			//+save user and peer id ondeactivate event of editbox
			$('userid').onfocusout=SaveUserPeerId;
			$('password').onfocusout=SaveUserPeerId;
			$('peerid').onfocusout=SaveUserPeerId;
			function SaveUserPeerId()
			{	
				date=new Date();
				date.setDate(date.getDate()+365);
				document.cookie="userid="+escape($('userid').value)+ "; expires=" + date.toGMTString()+"; path=/";
				document.cookie="password="+escape($('password').value)+"; expires=" + date.toGMTString()+"; path=/";
				document.cookie="peerid="+escape($('peerid').value) + "; expires=" + date.toGMTString()+"; path=/";
				
			}
			//-save user and peer id
			//+load saved earlier user and peer id
			var aCookie = document.cookie.split("; ");
			for (var i=0; i < aCookie.length; i++)
			  { 
				var aCrumb = aCookie[i].split("=");
				if(aCrumb[0]=="userid")$('userid').value=unescape(aCrumb[1]);
				else if(aCrumb[0]=="password")$('password').value=unescape(aCrumb[1]);
				else if(aCrumb[0]=="peerid")$('peerid').value=unescape(aCrumb[1]);
			  }
			//-load saved earlier user and peer id
		}
		sessions["InitInstaller"].m_service.InitInstall();
		//sessions["InitInstaller"].m_service.AfterInstallAction();
	}
}

/// Checks if component needed to be updated
/// @param componentCode msi code of component
/// @param availableVersion recent version of component on server 
/// @param SSInitialInstaller instalnce of initial installer object
/// @see version.js for all known codes and available versions
function CheckIfComponentUpdateNeeded(componentCode, availableVersion, SSInitialInstaller)
{
    try
	{
		if (SSInitialInstaller == null)
		{
			//addLogMessage('CheckIfComponentUpdateNeeded: SSInitialInstaller == null, assuming update needed',-1);
			return true;
		}
		var version = SSInitialInstaller.GetComponentVersion(	componentCode /*componentCode*/,
																componentCode /*product search key*/, 
																2 /*type of product search key. 2 means - product component*/);
		//addLogMessage("Version check for component (code:"+componentCode+") complete: current version("+version+") available version("+availableVersion+")",-1);
		/// Sice we will not upload on server old versions, if currentVersion != availableVersion
		/// than currentVersion is older, so update needed, no update needed elseway
		if (version == availableVersion)
			return false;
		else
			return true;
	}
	catch(e)
	{
		try
		{
		//	addLogMessage('Failed to get component version: '+e.description,-1);
		}
		catch(e)
		{
		//	addLogMessage('Unknown error happened at CheckIfComponentUpdateNeeded',-1);
		}
		return true;
	}
}

/// Checks if major update for product
/// @param componentCode msi code of component (at least 1 known product component needed)
/// @param SSInitialInstaller instalnce of initial installer object
/// @see version.js for all known codes and available versions
/// @return true - if component with componentCode isn't installed, or product major upgrade needed
function IsMajorUpdateNeeded(componentCode, SSInitialInstaller)
{
	try
	{
		var version = SSInitialInstaller.GetComponentVersion(	componentCode,		/*componentCode*/
																SSToolsProductCode,	/*product search key*/
																0					/*type of product search key. 0 means - product code*/);
		return false;
	}
	catch(e)
	{
		return true;
	}
}




/// if even one session connected, "prevent unexpected session stop" dialog 
/// advised to body::onbeforeunload event
function ConnectCheck()
{
	for(prop in sessions)
		if(sessions[prop]!=null&&sessions[prop].m_service.m_activeX!=null)
		{
			event.returnValue="One ore more connection established now. This action disconnect their.";
			break;
		}
}

/// deinitialize page function
function DeinitPage()
{
	//+delete session
	for(prop in sessions)
	{	
		CloseSession(prop);

	}
	//-delete session
	if(hostObject!=null)
	{
		hostObject.removeNode(true);
		hostObject=null;
	}
	
	//+delete frames (IE refresh button, reload() load incorrect iframe
	//var str="";
	//for(i=document.frames.length-1;i>=0;--i)
	//	checkIframe(document.frames(i).frameElement);
	//-delete frames
	
	if(installerObject!=null)
	{
		if(installerObject.DetachEvents!=null)
			installerObject.DetachEvents();
		installerObject=null;
	}
	document.getElementById("spanInstallerObject").innerHTML="";
	//CollectGarbage();
	setTimeout("CollectGarbage();",3000);
}



//------------------------------------------------------------------------------------------
//			others global function
//------------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------------
//			sessions manage function
//------------------------------------------------------------------------------------------

/// Creates new session
/// @param userID new session user identifier. 
/// @param password new session password for login to jabber server.
/// @param peerID new session peer identifier.
function NewSession(userId,password,peerId)
{	userId=TrimString(userId);
	peerId=TrimString(peerId);
	if(userId==""){alert("Incorrect userId.");return;}
	if(peerId==""){alert("Incorrect peerId.");return;}
	if(userId==peerId){alert("Incorrect userId or peerId.");return;}
	var name=userId+peerId;
	if(sessions[name]!=null)// name have existed in associative array yet
	{			// search unique name
		var i;
		for(i=1;sessions[name+i]!=null;++i);
		name+=i;
	}
	sessions[name]=new CSession(name,userId,password,peerId);
	//add new session in container panel 
	$("peerContainer").innerHTML+="<span id=\"spname"+name+"\" onclick=\"SelectSession('"+name+"');\"> "+name+" <br></span>";//TODO   Mouse Cursor
	SelectSession(name);
}

/// Closes session
/// @param sessionName session name
function CloseSession(sessionName)
{	
	if(sessionName!=null&&sessions[sessionName]!=null)
	{	
		SelectSession("");//TODO: nasty
		sessions[sessionName].foreDestroy();//release actions.
		sessions[sessionName]=null;//delete object CSession
		$("spname"+sessionName).removeNode(true);//delete panel
		for(prop in sessions)
			if(sessions[prop]!=null&&sessions[prop].m_name!=null)
			{
				SelectSession(sessions[prop].m_name);//set current session any valid session
				break;
			}
	}
}

/// Selects session from sessions pane
/// @param session session to select
function SelectSession(sessionName)
{	
	//deactivate old session
	if(sessions[currentSession]!=null)
	{	$("spname"+currentSession).style.backgroundColor='transparent';
		sessions[currentSession].Activate(false);//deactivate
	}
	//activate new session
	if(sessions[sessionName]!=null)
	{	
		$('userid').value=sessions[sessionName].m_userId;
		$('peerid').value=sessions[sessionName].m_peerId;
		$('bsclose').disabled=false;
		$("spname"+sessionName).style.backgroundColor='scrollbar';
		currentSession=sessionName;
		sessions[sessionName].Activate(true);// activate
	}
	else 
	{	
		sessionName=null;
		$('bsclose').disabled=true;
	}
	currentSession=sessionName;
}


//------------------------------------------------------------------------------------------
//			CSession 
//------------------------------------------------------------------------------------------

/// CSession class
/// @param name
/// @param userID new session user identifier. 
/// @param password new session password for login to jabber server.
/// @param peerID new session peer identifier.
function CSession(name,userId,password,peerId)
{	
	/// unique name session within html
	this.m_name=name;
	/// user identifier
	this.m_userId=userId;
	/// user password
	this.m_password=password;
	/// peer identifier
	this.m_peerId=peerId;
	/// selected option index of srtype combobox
	this.m_srtype=0;
	/// service type. type=0 RCViewer; type=1 RCHost; type=2 Playback;type=3 RCFileAccessClient; type=4 RCFileAccessHost; type=5 RCInstaller. For currently valid types see definition of servicesSet global variable (window object property).
	this.m_type=0;
	/// service object. type of object depend on m_type property. by default include CServiceRCViewer object.
	this.m_service=null;
	/// frame with UI elements
	this.m_frame=document.frames('frameSessions');//TODO
	//this.m_frame.session=this;//bind UI with session
	/// frame with UI elements
	this.m_frameProgress=document.frames('frameProgress');
	/// The method activate session object. The object set its state on the UI elements. Control frames for service (show/hide options frame, activeX frame, progress bar). Invoke from service when it require set new state of UI elements.
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	
			//occupier session and (service stoped or connected)
			if(g_connectingService==this&&(this.m_service.m_activeX==null||this.m_service.m_state))
			{
				g_connectingService=null;//release connect establish
				if(this.m_name!=currentSession)sessions[currentSession].Activate(true);
			}
			if(this.m_name==currentSession)//if this session active
			{
				this.m_frame.document.getElementById('cursession').value=this.m_name;
				this.m_frame.document.getElementById('srtype').selectedIndex=this.m_srtype;
				this.m_frame.session=this;//bind UI with service
				this.m_service.m_frameOptions.service=this.m_service;//bind UI with service
				
				this.m_service.m_frameOptions.frameElement.style.display="inline";
				if(this.m_service.m_activeX!=null)//activeX object is created
				{
					this.m_frame.document.getElementById("srtype").disabled=true;
					this.m_frame.document.getElementById("srstart").disabled=true;
					this.m_frame.document.getElementById("srstop").disabled=false;
					if(this.m_service.m_state)//connect is established
					{	
						this.m_service.m_frameActiveX.frameElement.width="100%";
						this.m_service.m_frameActiveX.frameElement.height="100%";
						this.m_service.m_activeX.width="100%";
						this.m_service.m_activeX.height="100%";
						this.m_frameProgress.frameElement.style.display="none";
					}
					else //connect hasn't established yet
					{
						//this.m_service.m_frameActiveX.frameElement.width="0%";
						//this.m_service.m_frameActiveX.frameElement.height="0%";
						//this.m_service.m_activeX.width=1;
						//this.m_service.m_activeX.height=1;
						this.m_frameProgress.document.getElementById('message').innerText=this.m_service.m_conStatus;
						this.m_frameProgress.document.getElementById('percent').style.width=this.m_service.m_conCompleted+"%";//percent;
						this.m_frameProgress.frameElement.style.display="inline";
					}
				}
				else//activeX object isn't created
				{
					this.m_frame.document.getElementById("srtype").disabled=false;
					this.m_frame.document.getElementById("srstart").disabled=g_connectingService!=null;//block start button if connect establish occupied
					this.m_frame.document.getElementById("srstop").disabled=true;
					this.m_service.m_frameActiveX.frameElement.width="0%";
					this.m_service.m_frameActiveX.frameElement.height="0%";
					this.m_frameProgress.frameElement.style.display="none";
				}
				this.m_service.Activate(flag);

			}
			
		}
		else//Deactivate
		{	
			this.m_service.m_frameOptions.frameElement.style.display="none";
			this.m_frame.document.getElementById("srtype").disabled=true;
			this.m_frame.document.getElementById("srstart").disabled=true;
			this.m_frame.document.getElementById("srstop").disabled=true;
			this.m_service.m_frameActiveX.frameElement.width="0%";
			this.m_service.m_frameActiveX.frameElement.height="0%";
			if(this.m_service.m_activeX!=null)
			{
				this.m_service.m_activeX.width=1;
				this.m_service.m_activeX.height=1;
			}
			this.m_frameProgress.frameElement.style.display="none";
			this.m_service.Activate(flag);
		}
		
	}
	/// method. start selected service
	this.Start=function()
	{
		if(g_connectingService==null)
		{
			g_connectingService=this;//for block other connect establish process
			this.m_service.Start();
		}
		else 
			AddLogMessage((new Date()).toLocaleTimeString()+"> Connect establish is occupied by ["+g_connectingService.m_name+"] session",-1);
		
	}
	///	method. stop selected service
	this.Stop=function(){this.m_service.Stop();}
	/// method. change type service. delete old and create new service object.
	this.ChangeType=function(type)
	{	
		if(this.m_service!=null)
		{
			this.Activate(false);
			this.m_service.foreDestroy();
			this.m_service=null;
		}
		this.m_srtype=type;
		this.m_type=this.m_frame.document.getElementById('srtype').options(type).value;
		if(this.m_type==1)this.m_service=new CServiceRCHost(this);// host service
		else if(this.m_type==2)this.m_service=new CServiceRCPlayback(this);//playback service
		else if(this.m_type==3)this.m_service=new CServiceRCFileAccessClient(this);//File Access Client service
		else if(this.m_type==4)this.m_service=new CServiceRCFileAccessHost(this);//File Access Host service
		else if(this.m_type==5)this.m_service=new CServiceRCInstaller(this);//Installer service
		else if(this.m_type==8)this.m_service=new CServiceRCViewerDlg(this);//Viewer Dialog service
		else if(this.m_type==9)this.m_service=new CServiceRCViewerPlus(this);
		else this.m_service=new CServiceRCViewer(this);//by default Viewer
		// setting UI frame in service
		this.m_service.m_frameOptions=document.frames(this.m_service.m_frameOptionsName);
		this.m_service.Activate=this.m_service.m_frameOptions.Activate;
		this.m_service.m_frameActiveX=document.frames(this.m_service.m_frameActiveXName);
		//alert(this.m_service.m_installerObject);
		this.Activate(true);
		//alert(this.m_service.m_installerObject);
	}
	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{	
		this.m_service.foreDestroy();//release service object
		if(g_connectingService==this)//occupier session 
				g_connectingService=null;//release connect establish
		//TODO this line lead to js error on the page 
		//this.m_service.m_frameOptions.service=null;
		//this.m_service=null;
		//this.m_frame=null;
		//this.m_frameProgress=null;
	}

	this.ChangeType(this.m_srtype);

}// CSession class



//------------------------------------------------------------------------------------------
//		CServiceRCViewer
//------------------------------------------------------------------------------------------

/// Remote Control Viewer service class
/// @param session parent-owner session object
function CServiceRCViewer(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object CRCViewerAXCtrl
	this.m_activeX=null;
	/// state of connection. true:connected; false:disconnected.
	this.m_state=false;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// current display mode
	this.m_displayMode=0;
	/// current alpha blend capturing
	this.m_captureAlphaBlend=0;
	/// user change viewer options set
	this.m_useOptions=false;
	/// viewer options set
	this.m_options=new CServiceRCViewerOpts();
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// recording file name
	this.m_recFileName="untitled";
	/// recording state
	this.m_recState=false;
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameViewerOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;
	
	/// start Viewer service. Create and connection ActiveX control object (CRCViewerAXCtrl).
	this.Start=function()
	{	
		//+get guid viewer activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==0)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid viewer activeX
		this.m_frameActiveX.document.getElementById('activeX').outerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"AX"+
			this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX=this.m_frameActiveX.document.getElementById("AX"+this.m_session.m_name);
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("CRCViewerAXCtrl creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}
		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		if(this.m_useOptions)
		{
			this.m_activeX.SetSessionOpts(	this.m_options.colorDepth,this.m_options.encoding,
											this.m_options.useCompressLevel,this.m_options.compressLevel,
											this.m_options.jpegCompress,this.m_options.jpegQualityLevel);
		}
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
		this.m_session.Activate(true);
	}
	
	/// stop Viewer service. Disconnect and delete ActiveX control object(CRCViewerAXCtrl).
	this.Stop=function()
	{	if(this.m_activeX!=null)
		{	this.m_activeX.Stop();
		}
	}
	
	/// Toggle Display mode between Window Scaling, Window Scrolling and Full Screen.
	/// @param mode new display mode. Where SCALE_MODE=0,SCROLL_MODE=1,FULLSCREEN_MODE=2
	this.SetDisplayMode=function(mode)
	{	this.m_displayMode=mode;
		if(this.m_activeX!=null&&this.m_state)
		{	this.m_activeX.SetDisplayMode(mode);
			if(this.m_displayMode==2)//FULLSCREEN_MODE
			{	this.m_displayMode=1;//after exit fullscreen_mode will scroll_mode
				this.Activate(true);
			}
		}
		//TODO bug RCE-104 Value of display mode combobox is setting incorrect.
	}
	
	/// Toggles alphablend capturing
	this.SetCaptureAlphaBlend=function(captureAlphaBlend)
	{	
		this.m_captureAlphaBlend=captureAlphaBlend;
		if(this.m_activeX!=null)
		{
			this.m_activeX.SetCaptureAlphaBlend(captureAlphaBlend);
		}
	}
	
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	//TODO if need change mode, but combobox set to destination mode
		this.m_access=mode;
		if(this.m_activeX!=null&&this.m_state)
		{	if(mode==0)//view only
			{	this.m_activeX.SetSessionMode(0,true);
				this.m_activeX.SetSessionMode(1,false);
			}
			else if(mode==1)//view only + visual pointer
			{	this.m_activeX.SetSessionMode(0,true);
				this.m_activeX.SetSessionMode(1,true);
			}
			else if(mode==2)//full control
			{	this.m_activeX.SetSessionMode(0,false);
			}
		}
	}
	
	/// Start and stop recording
	this.SetSessionRecording=function(fname,mode)
	{	
		if(this.m_activeX!=null)
		{	this.m_activeX.SetSessionRecording(fname,mode);
			this.m_recState=mode;
			this.Activate(true);
		}
	}
	
	// event disconnect viewer
	///TODO I don't what version better
	//first version
	//this.OnDisconnect=function()
	//{	//var ob=this.event;for(prop in ob)alert(prop+' > '+ob[prop]);//enumerate propertys 
	//	var _this=this;
	//	return function(reason){alert(_this.m_session.m_name+"OnDisconnect="+reason);}
	//}
	//attach event: this.m_activeX.attachEvent("OnDisconnect",this.OnDisconnect());
	//second version
	
	///Attach events handler to AcitveX viewer object
	this.AttachAXEvents=function()
	{	
		var _this=this;
		
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	
			_this.m_state=true;//diconnected
			_this.SetDisplayMode(_this.m_displayMode);//set display mode
			_this.SetCaptureAlphaBlend(_this.m_captureAlphaBlend);
			if(_this.m_access!=2)//not a full control TODO: make safe machanism set access mode on host side in this. now not always. inner viewer setaccess carry out after host do this.
				_this.SetAccessMode(_this.m_access);//set session mode
			_this.m_session.Activate(true);
		}
		
		
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	
			if(_this.m_recState&&reason==6)//if recording and OPENFILE_ERROR
			{	
				_this.m_recState=false;
				return;
			}
			DetachAXEvents();
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_recState=false;//recording
			_this.m_state=false;//diconnected
			_this.m_session.Activate(true);
			CollectGarbage(); //asd remove this
		}
		
		
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	
			_this.m_conCompleted=percent;	
			_this.m_conStatus=LocalizeString(message);
			_this.m_session.Activate(true);
		}
		//add other events handler here.
		
		//attach event
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);

		///detach events handler from AcitveX viewer object
		function DetachAXEvents()
		{
			_this.m_activeX.detachEvent("NotifySessionStart",OnNotifySessionStarted);
			_this.m_activeX.detachEvent("NotifySessionStop",OnNotifySessionStopped);
			_this.m_activeX.detachEvent("NotifyConnecting",OnNotifyConnecting);
		}
	}
	
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		this.Stop();//if viewer has sarted, stop it.
		if(this.m_activeX!=null)
		{
			this.m_activeX.removeNode(true);//delete activeX object
			this.m_activeX=null;
		}
		this.m_options=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
		this.Activate=null;
	}
	
} // CServiceRCViewer class

/// CServiceRCViewerOpts Remote Control Viewer service options class
function CServiceRCViewerOpts()
{	
	/// Colors count
	this.colorDepth=0;
	/// Preferred encoding
	this.encoding=0;
	/// Use custom zip/tight compression level
	this.useCompressLevel=false;
	/// Custom zip/tight compression level
	this.compressLevel=6;
	/// Use custom jpeg compression
	this.jpegCompress=false;
	/// Custom jpeg compression
	this.jpegQualityLevel=6;
}

//------------------------------------------------------------------------------------------
//		CServiceRCViewerPlus
//------------------------------------------------------------------------------------------
/// Remote Control Viewer service class
/// @param session parent-owner session object
function CServiceRCViewerPlus(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object CRCViewerAXCtrl
	this.m_activeX=null;
	/// state of connection. true:connected; false:disconnected.
	this.m_state=false;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// current display mode
	this.m_displayMode=0;
	/// current alpha blend capturing
	this.m_captureAlphaBlend=0;
	/// user change viewer options set
	this.m_useOptions=false;
	/// viewer options set
	this.m_options=new CServiceRCViewerOpts();
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// recording file name
	this.m_recFileName="untitled";
	/// recording state
	this.m_recState=false;
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameViewerOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;
	
	/// init activeX object with build-in UI
	this.Init=function()
	{
		if(this.m_activeX!=null)
		{
			AddLogMessage("CServiceRCViewerPlus::Init(). The object has initialized already",-1);
		}
		//+get guid viewer activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==9)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid viewer activeX
		this.m_frameActiveX.document.getElementById('activeX').outerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"AX"+
			this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX=this.m_frameActiveX.document.getElementById("AX"+this.m_session.m_name);
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("CRCViewerAXCtrl creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}
		this.AttachAXEvents();
		try
		{
			this.m_activeX.Init(this.m_session.m_peerId);
		}
		catch(e)
		{
			AddLogMessage("CRCViewerAXCtrl::Init() failed",-1);
		}
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.m_state=true;
		this.m_session.Activate(true);
		//+ status test
		var state=0;
		_this=this;
		function setstate()
		{
			if(state<100)
			{
				_this.m_activeX.SetUIStatus(state/10,"state="+state);
				state++;
				setTimeout(setstate,200);
			}
		}
		//setstate();
		//- status test

	}
	/// start Viewer service. Create and connection ActiveX control object (CRCViewerAXCtrl).
	this.Start=function()
	{	
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("CRCViewerAXCtrl has been not created",-1);
			return;
		}
		this.m_conCompleted=0;
		this.m_conStatus="start";
		if(this.m_useOptions)
		{
			this.m_activeX.SetSessionOpts(	this.m_options.colorDepth,this.m_options.encoding,
											this.m_options.useCompressLevel,this.m_options.compressLevel,
											this.m_options.jpegCompress,this.m_options.jpegQualityLevel);
		}
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
		this.m_state=true;
		this.m_session.Activate(true);
	}
	
	/// stop Viewer service. Disconnect and delete ActiveX control object(CRCViewerAXCtrl).
	this.Stop=function()
	{	if(this.m_activeX!=null)
		{	
			try
			{
				this.m_activeX.Stop();
			}
			catch(e)
			{
				AddLogMessage("CRCViewerAXCtrl::Stop() completed with error. "+(e.number&0xffff)+" "+e.name+" "+e.description+" "+e.message,-1);
			}
		}
	}
	
	/// Toggle Display mode between Window Scaling, Window Scrolling and Full Screen.
	/// @param mode new display mode. Where SCALE_MODE=0,SCROLL_MODE=1,FULLSCREEN_MODE=2
	this.SetDisplayMode=function(mode)
	{	this.m_displayMode=mode;
		if(this.m_activeX!=null&&this.m_state)
		{	this.m_activeX.SetDisplayMode(mode);
			if(this.m_displayMode==2)//FULLSCREEN_MODE
			{	this.m_displayMode=1;//after exit fullscreen_mode will scroll_mode
				this.Activate(true);
			}
		}
		//TODO bug RCE-104 Value of display mode combobox is setting incorrect.
	}
	
	/// Toggles alphablend capturing
	this.SetCaptureAlphaBlend=function(captureAlphaBlend)
	{	
		this.m_captureAlphaBlend=captureAlphaBlend;
		if(this.m_activeX!=null)
		{
			this.m_activeX.SetCaptureAlphaBlend(captureAlphaBlend);
		}
	}
	
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	//TODO if need change mode, but combobox set to destination mode
		this.m_access=mode;
		if(this.m_activeX!=null&&this.m_state)
		{	if(mode==0)//view only
			{	this.m_activeX.SetSessionMode(0,true);
				this.m_activeX.SetSessionMode(1,false);
			}
			else if(mode==1)//view only + visual pointer
			{	this.m_activeX.SetSessionMode(0,true);
				this.m_activeX.SetSessionMode(1,true);
			}
			else if(mode==2)//full control
			{	this.m_activeX.SetSessionMode(0,false);
			}
		}
	}
	
	/// Start and stop recording
	this.SetSessionRecording=function(fname,mode)
	{	
		if(this.m_activeX!=null)
		{	this.m_activeX.SetSessionRecording(fname,mode);
			this.m_recState=mode;
			this.Activate(true);
		}
	}
	
	// event disconnect viewer
	///TODO I don't what version better
	//first version
	//this.OnDisconnect=function()
	//{	//var ob=this.event;for(prop in ob)alert(prop+' > '+ob[prop]);//enumerate propertys 
	//	var _this=this;
	//	return function(reason){alert(_this.m_session.m_name+"OnDisconnect="+reason);}
	//}
	//attach event: this.m_activeX.attachEvent("OnDisconnect",this.OnDisconnect());
	//second version
	
	///Attach events handler to AcitveX viewer object
	this.AttachAXEvents=function()
	{	
		var _this=this;
		
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	
			_this.m_state=true;//diconnected
			_this.SetDisplayMode(_this.m_displayMode);//set display mode
			_this.SetCaptureAlphaBlend(_this.m_captureAlphaBlend);
			if(_this.m_access!=2)//not a full control TODO: make safe machanism set access mode on host side in this. now not always. inner viewer setaccess carry out after host do this.
				_this.SetAccessMode(_this.m_access);//set session mode
			_this.m_session.Activate(true);
			_this.m_activeX.SetUIStatus(8,"Desktop Sharing - On");
		}
		
		
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	
			if(_this.m_recState&&reason==6)//if recording and OPENFILE_ERROR
			{	
				_this.m_recState=false;
				return;
			}
			//DetachAXEvents();
			//_this.m_activeX.removeNode(true);//delete activeX object
			//_this.m_activeX=null;
			//_this.m_recState=false;//recording
			//_this.m_state=false;//diconnected
			//_this.m_session.Activate(true);
			//CollectGarbage(); //asd remove this
			if(reason==0)
				_this.m_activeX.SetUIStatus(6,"Desktop Sharing - Off");
			else 
				_this.m_activeX.SetUIStatus(9,"Connecting failed with reason = "+reason);
		}
		
		
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	
			_this.m_conCompleted=percent;	
			_this.m_conStatus=LocalizeString(message);
			_this.m_session.Activate(true);
			_this.m_activeX.SetUIStatus(7,_this.m_conStatus);
		}
		/// NotifySessionStartQuery event handler
		function OnNotifySessionStartQuery()
		{
			_this.m_state=false;
			_this.m_activeX.Start(_this.m_session.m_userId,_this.m_session.m_password,_this.m_session.m_peerId,relayServer,30000);
			_this.m_session.Activate(true);
		}
		
		/// NotifyUIEvent event handler
		function OnNotifyUIEvent(e,param)
		{
			if(e==0)
			{
				//_this.m_state=false;
				_this.m_access=param;
				_this.m_activeX.Start(_this.m_session.m_userId,_this.m_session.m_password,_this.m_session.m_peerId,relayServer,30000);
				_this.m_session.Activate(true);
			}
			else if(e==1)
			{
				_this.Stop();
			}
			else if(e==2)
			{
				_this.SetAccessMode(param);//set session mode
			}
			else
				alert("OnNotifyUIEvent="+e+" "+param);
		}


		//add other events handler here.
		
		//attach event
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);
		//this.m_activeX.attachEvent("NotifySessionStartQuery",OnNotifySessionStartQuery);
		this.m_activeX.attachEvent("NotifyUIEvent",OnNotifyUIEvent);

		///detach events handler from AcitveX viewer object
		function DetachAXEvents()
		{
			_this.m_activeX.detachEvent("NotifySessionStart",OnNotifySessionStarted);
			_this.m_activeX.detachEvent("NotifySessionStop",OnNotifySessionStopped);
			_this.m_activeX.detachEvent("NotifyConnecting",OnNotifyConnecting);
			//_this.m_activeX.detachEvent("NotifySessionStartQuery",OnNotifySessionStartQuery);
			_this.m_activeX.detachEvent("NotifyUIEvent",OnNotifyUIEvent);
		}
	}
	
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		this.Stop();//if viewer has sarted, stop it.
		if(this.m_activeX!=null)
		{
			this.m_activeX.removeNode(true);//delete activeX object
			this.m_activeX=null;
		}
		this.m_options=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
		this.Activate=null;
	}
	
} // CServiceRCViewer class

/// CServiceRCViewerOpts Remote Control Viewer service options class
function CServiceRCViewerOpts()
{	
	/// Colors count
	this.colorDepth=0;
	/// Preferred encoding
	this.encoding=0;
	/// Use custom zip/tight compression level
	this.useCompressLevel=false;
	/// Custom zip/tight compression level
	this.compressLevel=6;
	/// Use custom jpeg compression
	this.jpegCompress=false;
	/// Custom jpeg compression
	this.jpegQualityLevel=6;
}

//------------------------------------------------------------------------------------------
//		CServiceRCViewerDlg
//------------------------------------------------------------------------------------------

/// Remote Control Viewer Dialog service class
/// @param session parent-owner session object
function CServiceRCViewerDlg(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object CRCViewerAXDlg
	this.m_activeX=null;
	/// state of connection. true:connected; false:disconnected.
	this.m_state=false;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// current display mode
	this.m_displayMode=0;
	/// current alpha blend capturing
	this.m_captureAlphaBlend=0;
	/// user change viewer options set
	this.m_useOptions=false;
	/// viewer options set
	this.m_options=new CServiceRCViewerOpts();
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// recording file name
	this.m_recFileName="untitled";
	/// recording state
	this.m_recState=false;
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameViewerOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;
	
	/// start Viewer service. Create and connection ActiveX control object (CRCViewerAXCtrl).
	this.Start=function()
	{	
		//+get guid viewer activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==8)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid viewer activeX
		this.m_frameActiveX.document.getElementById('activeX').outerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"AX"+
			this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX=this.m_frameActiveX.document.getElementById("AX"+this.m_session.m_name);
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("CRCViewerAXDlg creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}
		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		this.m_state=true;
		//if(this.m_useOptions)
		//{
		//	this.m_activeX.SetSessionOpts(	this.m_options.colorDepth,this.m_options.encoding,
		//									this.m_options.useCompressLevel,this.m_options.compressLevel,
		//									this.m_options.jpegCompress,this.m_options.jpegQualityLevel);
		//}
		this.m_activeX.Mediator = $('IMMediatorObject');
		alert(this.m_activeX.Mediator);
		//this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
		this.m_session.Activate(true);
	}
	
	/// stop Viewer service. Disconnect and delete ActiveX control object(CRCViewerAXCtrl).
	this.Stop=function()
	{	if(this.m_activeX!=null)
		{	this.m_activeX.Stop();
		}
	}
	
	/// Toggle Display mode between Window Scaling, Window Scrolling and Full Screen.
	/// @param mode new display mode. Where SCALE_MODE=0,SCROLL_MODE=1,FULLSCREEN_MODE=2
	this.SetDisplayMode=function(mode)
	{	this.m_displayMode=mode;
		if(this.m_activeX!=null&&this.m_state)
		{	this.m_activeX.SetDisplayMode(mode);
			if(this.m_displayMode==2)//FULLSCREEN_MODE
			{	this.m_displayMode=1;//after exit fullscreen_mode will scroll_mode
				this.Activate(true);
			}
		}
		//TODO bug RCE-104 Value of display mode combobox is setting incorrect.
	}
	
	/// Toggles alphablend capturing
	this.SetCaptureAlphaBlend=function(captureAlphaBlend)
	{	
		this.m_captureAlphaBlend=captureAlphaBlend;
		if(this.m_activeX!=null)
		{
			this.m_activeX.SetCaptureAlphaBlend(captureAlphaBlend);
		}
	}
	
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	//TODO if need change mode, but combobox set to destination mode
		this.m_access=mode;
		if(this.m_activeX!=null&&this.m_state)
		{	if(mode==0)//view only
			{	this.m_activeX.SetSessionMode(0,true);
				this.m_activeX.SetSessionMode(1,false);
			}
			else if(mode==1)//view only + visual pointer
			{	this.m_activeX.SetSessionMode(0,true);
				this.m_activeX.SetSessionMode(1,true);
			}
			else if(mode==2)//full control
			{	this.m_activeX.SetSessionMode(0,false);
			}
		}
	}
	
	/// Start and stop recording
	this.SetSessionRecording=function(fname,mode)
	{	
		if(this.m_activeX!=null)
		{	this.m_activeX.SetSessionRecording(fname,mode);
			this.m_recState=mode;
			this.Activate(true);
		}
	}
	
	///Attach events handler to AcitveX viewer object
	this.AttachAXEvents=function()
	{	
		var _this=this;
		
		/// NotifyStartInitiated event handler
		function OnNotifyStartInitiated()
		{	
			function _start()
			{ 
				_this.m_activeX.Start(_this.m_session.m_userId,_this.m_session.m_password,_this.m_session.m_peerId,relayServer,30000);
			}
			
			setTimeout(_start,1000);
		}
		
		//attach event
		this.m_activeX.attachEvent("NotifyStartInitiated",OnNotifyStartInitiated);
		///detach events handler from AcitveX viewer object
		function DetachAXEvents()
		{
			_this.m_activeX.detachEvent("NotifyStartInitiated",OnNotifyStartInitiated);
		}
	}
	
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		if(this.m_activeX!=null)
		{
			this.m_activeX.removeNode(true);//delete activeX object
			this.m_activeX=null;
		}
		this.m_options=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
		this.Activate=null;
	}
	
} // CServiceRCViewerDlg class
//------------------------------------------------------------------------------------------
//		CServiceRCPlayback
//------------------------------------------------------------------------------------------

/// Remote Control Playback service class
/// @param session parent-owner session object
function CServiceRCPlayback(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object CRCViewerAXCtrl
	this.m_activeX=null;
	/// current display mode
	this.m_displayMode=0;
	/// playback file name
	this.m_fileName="untitled";
	/// delay factor
	this.m_delayFactor=4;
	/// state of connection. true:connected; false:disconnected. It are using as stub in this class.
	this.m_state=false;
	/// percent accomplish connetion. It are used for stub in this class.
	this.m_conCompleted=0;
	/// current connection message. It are used for stub in this class.
	this.m_conStatus=" ";
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="framePlaybackOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;

	/// start Playback service. Create and connection ActiveX control object (CRCViewerAXCtrl).
	this.Start=function()
	{	
		//+get guid playback activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==2)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid playback activeX
		this.m_frameActiveX.document.getElementById('activeX').outerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"AX"+
			this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";
		this.m_activeX=this.m_frameActiveX.document.getElementById("AX"+this.m_session.m_name);
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("CRCViewerAXCtrl creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}

		this.AttachAXEvents();
		this.m_activeX.StartPlayback(this.m_fileName);
		this.m_session.Activate(true);
	}
	/// stop Playback service. Disconnect and delete ActiveX control object(CRCViewerAXCtrl).
	this.Stop=function()
	{	if(this.m_activeX!=null)
		{	this.m_activeX.Stop();
		}
	}
	/// Toggle Display mode between Window Scaling, Window Scrolling and Full Screen.
	/// @param mode new display mode. Where SCALE_MODE=0,SCROLL_MODE=1,FULLSCREEN_MODE=2
	this.SetDisplayMode=function(mode)
	{	this.m_displayMode=mode;
		if(this.m_activeX!=null)
		{	this.m_activeX.SetDisplayMode(mode);
			if(this.m_displayMode==2)//FULLSCREEN_MODE
			{	this.m_displayMode=1;//after exit fullscreen_mode will scroll_mode
				this.Activate(true);
			}
		}
	}
	
	/// Set Delay factor
	this.SetDelayFactor=function(factor)
	{	this.m_delayFactor=factor;
		if(this.m_activeX!=null)
		{	//TODO fac=1.0 not pass to activeX method. in activeX as 0.00000
			var fac=1.0000001;
			if(factor==0)fac=16.0000001;
			else if(factor==1)fac=8.0000001;
			else if(factor==2)fac=4.0000001;
			else if(factor==3)fac=2.0000001;
			else if(factor==5)fac=0.5;
			else if(factor==6)fac=0.25;
			else if(factor==7)fac=0.125;
			else if(factor==8)fac=0.0625;
			this.m_activeX.SetDelayFactor(fac);
		}
	}

	///Attach events handler to AcitveX viewer object
	this.AttachAXEvents=function()
	{	var _this=this;
		
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{
			_this.m_state=true;
			_this.SetDisplayMode(_this.m_displayMode);//set display mode
			_this.SetDelayFactor(_this.m_delayFactor);//set delay factor
			if(_this.m_session.m_name==currentSession)//this active session
			_this.m_session.Activate(true);
		}
		
		/// NotifySessionStop event handler.
		/// @param reason stop reason code. LOCAL_STOP=0, REMOTE_STOP=1, STREAM_ERROR=2, PROTOCOL_ERROR=3, CHANGE_DISPLAY_MODE=4,CONNECTING_ERROR=5, OPENFILE_ERROR=6. See _ESessionStopReason definition for last version of reason code.
		function OnNotifySessionStopped(reason)
		{	
			DetachAXEvents();
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_recState=false;//recording
			_this.m_state=false;//diconnected
			_this.m_session.Activate(true);
		}
		
		//add other events handler here.
		
		//attach events
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);

		function DetachAXEvents()
		{
			_this.m_activeX.detachEvent("NotifySessionStart",OnNotifySessionStarted);
			_this.m_activeX.detachEvent("NotifySessionStop",OnNotifySessionStopped);
		}
	}
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		this.Stop();//if viewer has sarted, stop it.
		if(this.m_activeX!=null)
		{
			this.m_activeX.removeNode(true);//delete activeX object
			this.m_activeX=null;
		}
		this.m_options=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
		this.Activate=null;
	}
} // CServiceRCPlayback class



//------------------------------------------------------------------------------------------
//			CServiceRCHost
//------------------------------------------------------------------------------------------

/// Remote Control Host service class
//all methods and properties which relate to singleton object (all host sessions) add to hostObject object in
//hostObjectInit function. It invoke by first created CServiceHost object. CServiceRCHost include methods and properties
//related to specific session, although invoke global singleton object method.
/// @param session parent-owner session object
function CServiceRCHost(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object. It is stub in this class.
	this.m_activeX=null;
	/// internal id session
	this.m_sid=null;
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// state of connection. true:connected; false:disconnected.
	this.m_state=false;
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameHostOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;//function(){$('frameViewerOptions').style.display="inline";}
	
	if(hostObject==null)hostObjectInit();//create for demand activeX object
	/// pointer on global singleton Host object
	this.m_hostObject=hostObject;

	/// start Host service. Create new connection. (CRCHostAXCtrl).
	this.Start=function()
	{	
		this.m_conCompleted=0;
		this.m_conStatus="start";
		this.m_activeX=new Object();//stub
		this.m_sid=this.m_hostObject.StartClient(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);
		this.m_session.Activate(true);
	}
	/// stop Host service. Disconnect. (CRCHostAXCtrl).
	this.Stop=function()
	{	
		if(this.m_sid!=null) this.m_hostObject.StopClient(this.m_sid);
	}
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	this.m_access=mode;
		if(this.m_sid!=null&this.m_sid!=-2)
		{	if(mode==0)//view only
			{	this.m_hostObject.SetSessionMode(this.m_sid,0,true);
				this.m_hostObject.SetSessionMode(this.m_sid,1,false);
			}
			else if(mode==1)//view only + visual pointer
			{	this.m_hostObject.SetSessionMode(this.m_sid,0,true);
				this.m_hostObject.SetSessionMode(this.m_sid,1,true);
			}
			else if(mode==2)//full control
			{	this.m_hostObject.SetSessionMode(this.m_sid,0,false);
			}
		}
	}
	
	/// NotifySessionStarted event handler
	this.OnNotifySessionStart=function()
		{	
			this.m_state=true;
			this.SetAccessMode(this.m_access);//set session mode
			this.m_session.Activate(true);
		}
	/// NotifySessionStopped event handler.
	/// @param reason stop reason code.
	this.OnNotifySessionStop=function(reason)
		{	
			this.m_activeX=null;
			this.m_state=false;
			this.m_session.Activate(true);
			this.m_sid=null;
		}
	
	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		//+stop recording if this last CServiceHost object
		var srv=null;
		if(this.m_hostObject.m_recState)
		{
			for(prop in sessions) // search the rest CServiceHost
				if(sessions[prop]!=null&&sessions[prop].m_type==1&&prop!=this.m_session.m_name)//service type equal CServiceHost
					srv=sessions[prop].m_service;
			if(srv==null)// last CServiceHost object
				this.m_hostObject.statSetSessionRecording("",false)
		}
		//-stop recording if this last CServiceHost object
		this.Stop();//if viewer has sarted, stop it.
		this.m_hostObject=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
	 }
	///static method of CServiceRCHost class (static - here - mean that its relate to singleton host object only, not to specific session). 
	function hostObjectInit()
	{	
		//+get guid host activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==1)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid host activeX
		$('spanHostObject').innerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"hostObject\" width=\"0%\" height=\"0%\" ></OBJECT>";
		// Init HOST object veriable
		hostObject=$('hostObject');
		if(hostObject==null||hostObject.object==null)
		{
			AddLogMessage("CRCHostAXCtrl creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}
	
		/// recording state
		hostObject.m_recState=false;
		
		/// Start and stop recording
		hostObject.statSetSessionRecording=function(fname,mode)
		{	
			hostObject.SetSessionRecording(fname,mode);
			hostObject.m_recState=mode;
			for(prop in sessions) 
				if(sessions[prop]!=null&&sessions[prop].m_type==1)//service type equal CServiceHost
					sessions[prop].m_service.Activate(true);//state to UI all CServiceHost object
		}
		
		///Client started event handler to AcitveX host object
		/// @param clientId corresponding cliend identifier
		hostObject.statNotifySessionStart=function(clientId)
		{	for(prop in sessions)
				if(sessions[prop]!=null&&sessions[prop].m_type==1&&sessions[prop].m_service.m_sid==-2)//searching host service with temporary id
				{	
					sessions[prop].m_service.m_sid=clientId;//set valid id
					sessions[prop].m_service.OnNotifySessionStart();//call event correspond service object 
					return;
				}
		}
		
		/// Client stopped event handler to AcitveX host object
		/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)LOCAL_STOP=0, REMOTE_STOP=1, STREAM_ERROR=2, PROTOCOL_ERROR=3, CHANGE_DISPLAY_MODE=4,CONNECTING_ERROR=5, OPENFILE_ERROR=6. See _ESessionStopReason definition for last version of reason code.
		/// @param clientId corresponding cliend identifier
		//TODO correct reason codes later here and viewer service. 
		hostObject.statNotifySessionStop=function(clientId,reason)
		{	
			if(clientId==-1&&reason==6&&hostObject.m_recState)//if recording and OPENFILE_ERROR
			{	hostObject.m_recState=false;
				for(prop in sessions) 
					if(sessions[prop]!=null&&sessions[prop].m_type==1)//service type equal CServiceHost
						sessions[prop].m_service.Activate(true);//state to UI all CServiceHost object
			}
			for(prop in sessions)
				if(sessions[prop]!=null&&sessions[prop].m_type==1&&sessions[prop].m_service.m_sid==clientId)
				{	
					sessions[prop].m_service.OnNotifySessionStop(reason);//call event correspond service object 
					return;
				}
		}
		
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		hostObject.statNotifyConnecting=function(percent,message)
		{
			for(prop in sessions)
				if(sessions[prop]!=null&&sessions[prop].m_type == 1
					&&sessions[prop].m_service.m_sid == -2)//searching host service with temporary id
				{	
					sessions[prop].m_service.m_conCompleted = percent;
					sessions[prop].m_service.m_conStatus = LocalizeString(message);
					sessions[prop].Activate(true);
					return;
				}
		}

		// Set mediator
		hostObject.Mediator = $('IMMediatorObject');

		//attachEvent hostObject
		hostObject.attachEvent("NotifySessionStart",hostObject.statNotifySessionStart);
		hostObject.attachEvent("NotifySessionStop",hostObject.statNotifySessionStop);
		hostObject.attachEvent("NotifyConnecting",hostObject.statNotifyConnecting);
		//TODO it is necessary that do dettachEvent deleting object? dettachEvent don't nowhere.
	}//function hostObjectInit()
} // CServiceRCHost class
//------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
//			CServiceRCFileAccessClient
//------------------------------------------------------------------------------------------

/// Remote Control FileAccessClient service class
/// @param session parent-owner session object
function CServiceRCFileAccessClient(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object 
	this.m_activeX=null;
	/// state of connection. true:connected; false:disconnected.
	this.m_state=false;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameFileAccessClientOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;

	/// start FileAccessClient service. Create and connection ActiveX control object ().
	this.Start=function()
	{	
		//+get guid FileAccessClient activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==3)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid FileAccessClient activeX
		this.m_frameActiveX.document.getElementById('activeX').outerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"AX"+this.m_session.m_name+"\" width=\"100%\" height=\"100%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX=this.m_frameActiveX.document.getElementById("AX"+this.m_session.m_name);
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("IFileAccessClientX creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}

		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);
		this.m_session.Activate(true);
	}
	
	/// stop FileAccessClient service. Disconnect and delete ActiveX control object().
	this.Stop=function()
	{	if(this.m_activeX!=null)
		{	this.m_activeX.Stop();
		}
	}
	
	
	///Attach events handler to AcitveX FileAccessClient object
	this.AttachAXEvents=function()
	{	
		var _this=this;
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	
			_this.m_state=true;//connected
			_this.m_session.Activate(true);
		}
		
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	
			DetachAXEvents();
			_this.m_activeX.removeNode(true);//delete activeX object			
			_this.m_activeX=null;
			//CollectGarbage();
			_this.m_state=false;//diconnected
			_this.m_session.Activate(true);
		}
		
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	
			_this.m_conCompleted=percent;	
			_this.m_conStatus=LocalizeString(message);
			_this.m_session.Activate(true);
		}
		//add other events handler here.
		
		//attach events
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);
		
		///detach events handler from AcitveX viewer object
		function DetachAXEvents()
		{
			_this.m_activeX.detachEvent("NotifySessionStart",OnNotifySessionStarted);
			_this.m_activeX.detachEvent("NotifySessionStop",OnNotifySessionStopped);
			_this.m_activeX.detachEvent("NotifyConnecting",OnNotifyConnecting);
		}
	}
	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		this.Stop();//if service has sarted, stop it.
		if(this.m_activeX!=null)
		{
			this.m_activeX.removeNode(true);//delete activeX object
			this.m_activeX=null;
		}
		this.m_options=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
		this.Activate=null;
	}
	
} // CServiceRCFileClient class



//------------------------------------------------------------------------------------------
//			CServiceRCFileAccessHost
//------------------------------------------------------------------------------------------

/// Remote Control FileAccessHost service class
/// @param session parent-owner session object
function CServiceRCFileAccessHost(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object 
	this.m_activeX=null;
	/// state of connection. true:connected; false:disconnected.
	this.m_state=false;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameFileAccessHostOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;//function(){$('frameViewerOptions').style.display="inline";}

	/// start FileAccessHost service. Create and connection ActiveX control object ().
	this.Start=function()
	{	
		//+get guid FileAccessHost activeX
		var guid="";
		for(prop in servicesSet)
			if(servicesSet[prop].m_type==4)
			{
				guid=servicesSet[prop].m_guid;
				break;
			}
		//-get guid FileAccessHost activeX
		this.m_frameActiveX.document.getElementById('activeX').outerHTML+="<OBJECT classid=\"clsid:"+guid+"\" id=\"AX"+
			this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";
		this.m_activeX=this.m_frameActiveX.document.getElementById("AX"+this.m_session.m_name);
		if(this.m_activeX==null||this.m_activeX.object==null)
		{
			AddLogMessage("IFileAccessHost creation failed",-1);
			this.m_activeX.removeNode(true);
			this.m_activeX=null;
			return;
		}

		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.SetAccessMode(this.m_access);//set session mode
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
		this.m_session.Activate(true);
	}
	/// stop FileAccessClient service. Disconnect and delete ActiveX control object().
	this.Stop=function()
	{	if(this.m_activeX!=null)
		{	this.m_activeX.Stop();
		}
	}
	
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	this.m_access=mode;
		if(this.m_activeX!=null)
		{	//0 list_drives,
			//1 list_files,
			//2 send_file,
			//3 retrieve_file,
			//4 delete_file,
			//5 delete_directory,
			//6 rename_file,
			//7 create_directory
			var fro=false,fuo=false,ffa=false;
			switch(mode)
			{	case 0:fro=true;break;
				case 1:fuo=true;break;
				case 2:ffa=true;break;
				default: ;
			}
			this.m_activeX.SetAuthorization(0,true);
			this.m_activeX.SetAuthorization(1,true);
			this.m_activeX.SetAuthorization(2,fro||ffa);
			this.m_activeX.SetAuthorization(3,fuo||ffa);
			this.m_activeX.SetAuthorization(4,ffa);
			this.m_activeX.SetAuthorization(5,ffa);
			this.m_activeX.SetAuthorization(6,ffa);
			this.m_activeX.SetAuthorization(7,fuo||ffa);
		}
	}
	
	///Attach events handler to AcitveX FileAccessClient object
	this.AttachAXEvents=function()
	{	
		var _this=this;
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	
			_this.m_state=true;//connected
			_this.m_session.Activate(true);
		}
		
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	
			DetachAXEvents();
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_state=false;//diconnected
			_this.m_session.Activate(true);
		}
		
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	
			_this.m_conCompleted=percent;	
			_this.m_conStatus=LocalizeString(message);
			_this.m_session.Activate(true);
		}
		// add other events handler here.

		// attach event
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);

		/// detach events handler from AcitveX viewer object
		function DetachAXEvents()
		{
			_this.m_activeX.detachEvent("NotifySessionStart",OnNotifySessionStarted);
			_this.m_activeX.detachEvent("NotifySessionStop",OnNotifySessionStopped);
			_this.m_activeX.detachEvent("NotifyConnecting",OnNotifyConnecting);
		}
	}
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{
		this.Stop();//if viewer has sarted, stop it.
		if(this.m_activeX!=null)
		{
			this.m_activeX.removeNode(true);//delete activeX object
			this.m_activeX=null;
		}
		this.m_options=null;
		this.m_frameOptions=null;
		this.m_frameActiveX=null;
		this.Activate=null;
	}
	
} // CServiceRCFileClient class



//------------------------------------------------------------------------------------------
//			CServiceRCInstaller
//------------------------------------------------------------------------------------------

/// Remote Control Installer service class.
/// @param session parent-owner session object
function CServiceRCInstaller(session)
{	
	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object CRCInstallerAXCtrl. Are used as stub.
	this.m_activeX=null;
	/// state of connection. true:connected; false:disconnected. Are used as stub.
	this.m_state=false;
	/// percent accomplish installing
	this.m_conCompleted=0;
	/// current installing message
	this.m_conStatus=" ";
	/// frame with properties UI elements 
	this.m_frameOptions=null;
	/// frame with ActiveX UI elements
	this.m_frameActiveX=null;
	/// name of frame with properties UI elements
	this.m_frameOptionsName="frameInstallerOptions";
	/// name of frame with ActiveX UI elements
	this.m_frameActiveXName="frameActiveX";
	/// The method activate service object. The object set its state on the UI elements. Set after create.
	this.Activate=null;//function(){$('frameViewerOptions').style.display="inline";}
	/// feature name to install
	this.m_feature=null;
	/// start Installer service. download and install new feature.
	this.Start=function()
	{	
		this.m_activeX=new Object();//stub
		var cmdLine="";
		//+has feature installed 
		//TODO WI request installed feature?
		for(prop in servicesSet)
		{
			if(servicesSet[prop].m_fname==this.m_feature&&servicesSet[prop].m_guid!="")
			{	
				cmdLine="REINSTALL="+this.m_feature+" REINSTALLMODE=vamus";
				//cmdLine="REINSTALLMODE=vamus REINSTALL=ALL";
				this.m_conStatus="Preparing to reinstall...";
			}
		}
		//-has feature installed 
		if(cmdLine=="")//feature has not installed yet
		{
			cmdLine="ADDLOCAL="+this.m_feature;
			this.m_conStatus="Preparing to install...";
		}
		// RCInstaller is installed due to SSInitialInstaller
		if(this.m_feature=="RCInstaller")
		{
			//$("spanInstallerObject").removeNode(true);
			//CollectGarbage();
			//alert($("SSInitialInstaller"));
			//DeinitPage();
			$("SSInitialInstaller").Install(1,relativeMsi,cmdLine,0x02);//asynchronous call
			//document.location.reload(true);
			return;
		}
		try	
		{	
			this.m_conCompleted=2;
			AddLogMessage((new Date()).toLocaleTimeString()+"> installation cmd line: "+cmdLine,-1);
			this.m_installerObject.execInstaller=this;//pointer to service, it is used in event handler
			this.m_session.Activate(true);
			this.m_installerObject.ConfigureProductEx(cmdLine);//prepare installation
		}
		catch(e)
		{	
			this.m_conCompleted=0;
			this.m_conStatus=e.name+" "+e.description+" "+(e.number&0xFFFF);
			AddLogMessage((new Date()).toLocaleTimeString()+"> installation error: "+this.m_conStatus,-1);
			this.m_session.Activate(true);
		}
		
	}
	/// stop Installer service. Abort installation process
	this.Stop=function()
	{	
		this.m_installerObject.CancelInstalling();
	}
		
	/// method. it is nonautomatic destructer.
	this.foreDestroy=function()
	{
		this.Stop();//if viewer has sarted, stop it.
		if(this.m_installerObject.execInstaller!=null)
			this.m_installerObject.execInstaller.AfterInstallAction=null;
		this.m_installerObject.execInstaller=null;
		this.m_installerObject=null;
		this.m_frameOptions.service=null;
		this.m_frameOptions=null;
	}
		
	/// inner pointer on activeX installer object
	this.m_installerObject=installerObject;
	
	/// Initializing function. Must by call before use the object. (Now, it is called by first activation.)
	this.Init=function()	
	{
		//+INIT SINGLETON OBJECT ACTIONS
		this.m_installerObject=$('RCInstallerObject');
		if(this.m_installerObject.object==null) 
		{
			alert("RCInstaller::Init() RCInstallerObject wasn't created");
			return;
		}
		//init global installer activeX object
		installerObject=this.m_installerObject;
		// Set mediator
		//TODO this.m_installerObject.Mediator = $('IMMediatorObject');
		/// pointer to execute Start() service object
		this.m_installerObject.execInstaller=null;
		// logger span
		var logger=document.frames("frameLogger").document.getElementById("logger");
		///log event handler
		function OnNotifyLogMessage(message,severity)
		{	var strseverity="";
			/*@if (@_debug)@*/
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
			/*@end @*/
			logger.innerText=strseverity+message+"\n"+logger.innerText;
			//TODO create clean up old message and big blank space between submessage
		}
		
		// NotifyFeatureInstalled event handler// @param result if equal 0 feacher installed success, other wise install failed
		function OnNotifyFeatureInstalled(result)
		{	
			installerObject.execInstaller.m_activeX=null;
			installerObject.execInstaller.m_session.Activate(true);
			if(installerObject.execInstaller.AfterInstallAction!=null)
			{
				installerObject.execInstaller.AfterInstallAction();
				if(installerObject!=null&&installerObject.execInstaller!=null)
				{
					installerObject.execInstaller.AfterInstallAction=null;
					installerObject.execInstaller=null;
				}
			}
			else
			{
				// change srtype select and existed session
				installerObject.execInstaller.MapInstalledService();
				installerObject.execInstaller=null;
				//window.location.reload(false);//Reloads the page from the browser cache
			}
			//TODO add in log if install action failed
		}
		// NotifyInstalling event handler./// @param percent percent completed./// @param message hint current process connecting.
		function OnNotifyInstalling(percent,message)
		{	
			//AddLogMessage(percent+">"+message,-1);
			if(percent>=0&&percent<=100)//TODO BUG with XP WIv2
			{
				installerObject.execInstaller.m_conCompleted=percent;
				installerObject.execInstaller.m_conStatus=message;
				installerObject.execInstaller.m_session.Activate(true);
			}
		}
		//attach Logger Event 
		//TODO Log idea!
		this.m_installerObject.attachEvent("NotifyLogMessage",OnNotifyLogMessage);
		this.m_installerObject.attachEvent("NotifyFeatureInstalled",OnNotifyFeatureInstalled);
		this.m_installerObject.attachEvent("NotifyInstalling",OnNotifyInstalling);
		//return;
		//installerObject.m_eventOnNotifyLogMessage=OnNotifyLogMessage;
		//installerObject.m_eventOnNotifyFeatureInstalled=OnNotifyFeatureInstalled;
		//installerObject.m_eventOnNotifyInstalling=OnNotifyInstalling;
		//this.m_installerObject.DetachEvents=function()
		//{
		//	installerObject.detachEvent("NotifyLogMessage",installerObject.m_eventOnNotifyLogMessage);
		//	installerObject.detachEvent("NotifyFeatureInstalled",installerObject.m_eventOnNotifyFeatureInstalled);
		//	installerObject.detachEvent("NotifyInstalling",installerObject.m_eventOnNotifyInstalling);
		//}
		//return;
		//installerObject.attachEvent("NotifyLogMessage",installerObject.m_eventOnNotifyLogMessage);
		//installerObject.attachEvent("NotifyFeatureInstalled",installerObject.m_eventOnNotifyFeatureInstalled);
		//installerObject.attachEvent("NotifyInstalling",installerObject.m_eventOnNotifyInstalling);
		
		AddLogMessage=OnNotifyLogMessage;//init global adding log message function
		//TODO it is necessary that do dettachEvent deleting object? dettachEvent don't nowhere.
		
		this.m_installerObject.SetInternalUI(0x2);//set UI level 
		this.m_installerObject.EnableLog(0x3fff,"RCInstaller",1);//turn on log //installerObject.EnableLog(0,0);
		//TODO
		//$('frameActiveXcon').style.display="inline"
		//ShowConCompleted(2,"Preparing to update ...");
		//installerObject.ReinstallFeature("ALL",0x404);// msiexec /fov
		//installerObject.InstallProduct("REINSTALL=ALL REINSTALLMODE=ov");// msiexec /fov
		//ConfigureProductEx
		//installerObject.m_conCompleted=0;
		//installerObject.m_conStatus="Update completed.";
		//ShowConCompleted(installerObject.m_conCompleted,installerObject.m_conStatus);
		//-INIT SINGLETON OBJECT ACTIONS
		
		
		
		// fill servicesSet combobox
		var uniqFeature=new Array();//array of uniqFeature name
		this.m_frameOptions.document.getElementById('srvInstallerFeatures').options.length=0;
		for(prop in servicesSet)
		{	
			var i;
			for(i=0;i<uniqFeature.length&&uniqFeature[i]!=servicesSet[prop].m_fname;i++);
			if(i==uniqFeature.length)//new feature name unique 
			{
				uniqFeature[uniqFeature.length]=servicesSet[prop].m_fname;
				var oOption=this.m_frameOptions.document.createElement("OPTION");
				this.m_frameOptions.document.getElementById('srvInstallerFeatures').options.add(oOption);
				oOption.innerText=servicesSet[prop].m_fname;
			}
		}
		// search installed service
		this.MapInstalledService();
		if (this.m_installerObject.IsProcessUnderUIPI())
		{
			alert('Seems like IE is running in protected mode. RCHost with full controll may not run properly. It\'s highly recommended to add site into trusted zone, or turn off UIPI');
		}	
	}//this.Init()
	
	/// search installed service, fill 'srtype' select html object, correct session.m_srtype in sessions array
	this.MapInstalledService=function()
	{	
		this.m_session.m_frame.document.getElementById('srtype').options.length=0;
		for(prop in servicesSet)
		{	
			var guid="";
			try
			{	
				guid=this.m_installerObject.GetServiceID(servicesSet[prop].m_fname,servicesSet[prop].m_name);
			}
			catch(e)
			{
				continue;
			}
			if(guid!="")
			{
				servicesSet[prop].m_guid=guid;
				{
					var oOption=this.m_session.m_frame.document.createElement("OPTION");
					this.m_session.m_frame.document.getElementById('srtype').options.add(oOption);
					oOption.innerText=prop;
					oOption.value=servicesSet[prop].m_type;
					AddLogMessage((new Date()).toLocaleTimeString()+"> "+servicesSet[prop].m_name+" "+servicesSet[prop].m_guid+" ",-1);		
					//correct indexes of options in 'srtype' select mapping service type
					for(var sess in sessions)
					{
						if(sessions[sess]!=null&&sessions[sess].m_type==servicesSet[prop].m_type)
							sessions[sess].m_srtype=oOption.index;
					}
				}
			}
		}
		SelectSession(this.m_session.m_name);
		//this.m_session.m_frame.document.getElementById('srtype').selectedIndex
	}
	
	/// force initialization installation
	this.InitInstall=function()
	{
		var installFeature=new Array();//array of feature to installation 
		for(prop in servicesSet)
		{
			if(servicesSet[prop]!=null&&servicesSet[prop].m_installMap&pageInstallMask)
			{
				var guid="";
				try
				{
					guid=this.m_installerObject.GetServiceID(servicesSet[prop].m_fname,servicesSet[prop].m_name);
				}
				catch(e)
					{;}
				if(guid=="")
					installFeature[servicesSet[prop].m_fname]=1;//feature to install
			}
		}
		var cmdLine="";
		for(prop in installFeature)
		{
			if(installFeature[prop]!=null)cmdLine+=prop+",";
		}
		if(cmdLine!="")
		{
			cmdLine="ADDLOCAL="+cmdLine+" ";
		}
		// RCComponents update
		cmdLine+="REINSTALL=ALL REINSTALLMODE=vomus";
		this.m_conStatus="Preparing to install...";
		this.m_conCompleted=2;
		AddLogMessage((new Date()).toLocaleTimeString()+"> installation cmd line: "+cmdLine,-1);
		this.m_installerObject.execInstaller=this;//pointer to service, it is used in event handler
		this.m_activeX=new Object();
		this.m_session.Activate(true);
		this.m_installerObject.ConfigureProductEx(cmdLine);//prepare installation
	}
} // CServiceRCInstaller class
//------------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
