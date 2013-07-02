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

function TrimString(str)
{	if(str=="")return "";
	var i,j;
	for(i=0;i<str.length&&str.charAt(i)==' ';++i);//trim left
	if(i==str.length)return "";//blank string. white space only.
	for(j=str.length-1;j>=0&&str.charAt(j)==' ';--j);//trim right
	return str.substring(i,j+1);
}

//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------

/*@cc_on @*/
/*@set @_debug=false @*/
/*@if (@_debug)@*/
   //alert("debug");
/*@end@*/



/// A global variable and function (Document class)

/// hard-coded IP address of the relay server
//var relayServer='192.168.0.135';
var relayServer='213.8.114.131';

/// Current session
var currentSession=null;
/// Sessions (associative) array
var sessions = new Object();
/// global index for generating activeX id
//var globalIndex = 0; TODO unclear what a reason.

/// the host activeX object (CRCHostAXCtrl)
//var hostObject=new CreateObject('RCUI.RCHostAXCtrl');TODO thus is beautiful, but i don't know how attach event handler
var hostObject=null;


/// Creates new session
/// @param peerID new sessiosn peerID
/// session is created only if we haven't such peerID 
function NewSession(userId,password,peerId)
{	userId=TrimString(userId);
	peerId=TrimString(peerId);
	if(userId==""){alert("Incorrect userId.");return;}
	if(peerId==""){alert("Incorrect peerId.");return;}
	if(userId==peerId){alert("Incorrect userId or peerId.");return;}
	var name=userId+peerId;
	if(sessions[name]!=null)// name have existed in associative array yet
	{	// search unique name
		var i;
		for(i=1;sessions[name+i]!=null;++i);
		name+=i;
	}
	//alert(name);
	sessions[name]=new CSession(name,userId,password,peerId);
	//add new session in container panel 
	//$("peerContainer").innerHTML+="<span id=\""+name+"\"><a href=\"JavaScript:SelectSession('"+name+"')\">"+name+"<br><br></a></span>";//TODO href replace by onclick event.
	$("peerContainer").innerHTML+="<span id=\"spname"+name+"\" onclick=\"SelectSession('"+name+"');\"> "+name+" <br><br></span>";//TODO   Mouse Cursor
	SelectSession(name);
}

/// Closes session
/// @param session 
function CloseSession(sessionName)
{	if(sessionName!=null)
	{	SelectSession("");//TODO: nasty
		sessions[sessionName].foreDestroy();//release actions.
		sessions[sessionName]=null;//delete object CSession
		$("spname"+sessionName).removeNode(true);//delete panel
		for(prop in sessions)
			if(sessions[prop]!=null&&sessions[prop].m_name!=null){SelectSession(sessions[prop].m_name);break;}//set current session any valid session
	}
	//else alert("select session");//TODO: nasty.
}

/// Selects session from sessions pane
/// @param session session to select
function SelectSession(sessionName)
{	//deactivate old session
	if(sessions[currentSession]!=null)
	{	$("spname"+currentSession).style.backgroundColor='transparent';
		sessions[currentSession].Activate(false);//deactivate
	}
	//activate new session
	if(sessions[sessionName]!=null)
	{	$('cursession').value=sessionName;
		$('userid').value=sessions[sessionName].m_userId;
		$('peerid').value=sessions[sessionName].m_peerId;
		$('bsclose').disabled=false;
		$("spname"+sessionName).style.backgroundColor='scrollbar';
		sessions[sessionName].Activate(true);// activate
	}
	else //TODO nasty. This are used when delete session. Need devise anything
	{	sessionName=null;
		$('cursession').value="";
		//$('userid').value="";
		//$('peerid').value="";
		$('bsclose').disabled=true;
	}
	currentSession=sessionName;
}

/// show new connection process values 
ShowConCompleted=function(percent,message)
{	$('activeXconMessage').innerText=message;
	$('activeXconPercent').style.width=percent+"%";//percent;
}

//------------------------------------------------------------------------------------------

/// CSession class
function CSession(name,userId,password,peerId)
{	/// unique name session within html
	this.m_name=name;
	/// user identifier
	this.m_userId=userId;
	/// user password
	this.m_password=password;
	/// peer identifier
	this.m_peerId=peerId;
	/// service type. type=0 RCViewer; type=1 RCHost; type=2 Playback
	this.m_type=0;
	/// service object. type of object depend on m_type property. by default include CServiceRCViewer object.
	this.m_service=new CServiceRCViewer(this);
	/// method. activate session object - the object show ifsalf ...
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	$('sraccess').disabled=false;
			$('srtype').selectedIndex=this.m_type;
			this.m_service.Activate(true);
		}
		else//Deactivate
		{	$('sraccess').disabled=true;
			this.m_service.Activate(false);
		}
	}
	/// method. start selected service
	this.Start=function(){this.m_service.Start();}
	///	method. stop selected service
	this.Stop=function(){this.m_service.Stop();}
	/// method. change type service. delete old and create new service object.
	this.ChangeType=function(type)
	{	this.m_service.Activate(false);
		this.m_type=type;
		if(this.m_type==1)this.m_service=new CServiceRCHost(this);// host service
		else if(this.m_type==2)this.m_service=new CServiceRCPlayback(this);//playback service
		else if(this.m_type==3)this.m_service=new CServiceRCFileAccessClient(this);//File Access Client service
		else if(this.m_type==4)this.m_service=new CServiceRCFileAccessHost(this);//File Access Host service
		else this.m_service=new CServiceRCViewer(this);//by default Viewer
		this.m_service.Activate(true);
	}
	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function()
	{	this.m_service.foreDestroy();//release service object
	}
}// CSession class



//------------------------------------------------------------------------------------------
/// Remote Control Viewer service class
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
	/// method. activate service object - the object show ifsalf ...
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	$('srtype').disabled=false;
			$('srViewerOptions').style.display="inline";
			if(this.m_activeX!=null)
			{	$('srtype').disabled=true;
				$('bsrstop').disabled=false;
				if(this.m_state){this.m_activeX.style.display="inline";}
				else {$('activeXcon').style.display="inline";ShowConCompleted(this.m_conCompleted,this.m_conStatus);}
			}
			else	
			{	$('bsrstart').disabled=false;$('srtype').disabled=false;
			}
			
			$('srvDisplayMode').selectedIndex=this.m_displayMode;
			//+options
			$('srvColors').selectedIndex=this.m_options.colorDepth;
			$('srvEncoding').selectedIndex=this.m_options.encoding;
			$('srvuCompressLevel').checked=this.m_options.useCompressLevel;
			$('srvCompressLevel').value=this.m_options.compressLevel;
			$('srvuJpeg').checked=this.m_options.jpegCompress;
			$('srvJpeg').value=this.m_options.jpegQualityLevel;
			
			var f=this.m_activeX!=null;//viewer service started
			$('srvColors').disabled=f;
			$('srvEncoding').disabled=f;
			$('srvuCompressLevel').disabled=f;
			$('srvCompressLevel').disabled=f;
			$('srvuJpeg').disabled=f;
			$('srvJpeg').disabled=f;
			//-options
			//alert($('sraccess').outerHTML);
			//$('sraccess').innerHTML="";
			//alert($('sraccess').outerHTML);
			$('sraccess').outerHTML="<select id=\"sraccess\" style=\"width: 150px\" onclick=\"sessions[currentSession].m_service.SetAccessMode($('sraccess').selectedIndex);\" ><option id=\"0\">View Only</option><option id=\"1\">View Only + Visual Pointer</option><option id=\"2\">Full Control</option></select>";
			//alert($('sraccess').outerHTML);
			$('sraccess').selectedIndex=this.m_access;
			//recording
			$('srvRecord').value=this.m_recFileName;
			if(this.m_activeX!=null&&this.m_state)
			{	$('srvRecordStart').disabled=this.m_recState;
				$('srvRecordStop').disabled=!this.m_recState;
			}
			else
			{	$('srvRecordStart').disabled=true;
				$('srvRecordStop').disabled=true;
			}
		}
		else//Deactivate
		{	$('srtype').disabled=true;
			$('srViewerOptions').style.display="none";
			if(this.m_activeX!=null)this.m_activeX.style.display="none";
			$('bsrstart').disabled=true;
			$('bsrstop').disabled=true;
			$('sraccess').disabled=true;
			this.m_recFileName=$('srvRecord').value;
			$('srvRecordStart').disabled=true;
			$('srvRecordStop').disabled=true;
			$('activeXcon').style.display="none";
		}
	}
	/// start Viewer service. Create and connection ActiveX control object (CRCViewerAXCtrl).
	this.Start=function()
	{	if(this.m_activeX!=null){alert("activeX is not delete");return;}
		$('activeX').outerHTML+="<OBJECT classid=\"clsid:DA4679AA-3239-43EF-8D59-6E82AEF6F081\" id=\"AX"+this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX = $("AX"+this.m_session.m_name);
		//this.m_activeX.attachEvent("OnDisconnect",this.OnDisconnect());
		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		$('activeXcon').style.display="inline"
		ShowConCompleted(this.m_conCompleted,this.m_conStatus);
		$('bsrstart').disabled=true;
		if(this.m_useOptions)this.m_activeX.SetSessionOpts(this.m_options.colorDepth,this.m_options.encoding,this.m_options.useCompressLevel,this.m_options.compressLevel,this.m_options.jpegCompress,this.m_options.jpegQualityLevel);
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
		$('bsrstop').disabled=false;

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
		if(this.m_activeX!=null)
		{	this.m_activeX.SetDisplayMode(mode);
			if(this.m_displayMode==2)//FULLSCREEN_MODE
			{	this.m_displayMode=1;//after exit fullscreen_mode will scroll_mode
				$('srvDisplayMode').selectedIndex=this.m_displayMode;	
			}
		}

		//TODO
	}
	/// Set custom options by user control element
	this.SetOptions=function()
	{	this.m_useOptions=true;//user change options. jast befor next start viewer will set custom options
		this.m_options.colorDepth=$('srvColors').selectedIndex;
		this.m_options.encoding=$('srvEncoding').selectedIndex;
		this.m_options.useCompressLevel=$('srvuCompressLevel').checked;
		this.m_options.compressLevel=$('srvCompressLevel').value;
		this.m_options.jpegCompress=$('srvuJpeg').checked;
		this.m_options.jpegQualityLevel=$('srvJpeg').value;
	}
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	//TODO if need change mode, but combobox set to destination mode
		this.m_access=mode;
		if(this.m_activeX!=null)
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
	{	if(this.m_activeX!=null)
		{	this.m_activeX.SetSessionRecording(fname,mode);
			this.m_recState=mode;
			$('srvRecordStart').disabled=this.m_recState;
			$('srvRecordStop').disabled=!this.m_recState;
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
	{	var _this=this;
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnConnect");
			/*@end@*/
			_this.m_state=true;//diconnected
			_this.m_activeX.height="100%";
			_this.m_activeX.width="100%";
			_this.SetDisplayMode($('srvDisplayMode').selectedIndex);//set display mode
			if(_this.m_access!=2)//not a full control TODO: make safe machanism set access mode on host side in this. now not always. inner viewer setaccess carry out after host do this.
				_this.SetAccessMode(_this.m_access);//set session mode
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=true;// TODO revise by eventing
				//$('bsrstop').disabled=false;
				$('srtype').disabled=true;
				$('srvColors').disabled=true;
				$('srvEncoding').disabled=true;
				$('srvuCompressLevel').disabled=true;
				$('srvCompressLevel').disabled=true;
				$('srvuJpeg').disabled=true;
				$('srvJpeg').disabled=true;
				$('srvRecordStart').disabled=false;
				$('activeXcon').style.display="none";
			}
		}
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			if(_this.m_recState&&reason==6)//if recording and OPENFILE_ERROR
			{	$('srvRecordStart').disabled=false;
				$('srvRecordStop').disabled=true;
				_this.m_recState=false;
				return;
			}
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_recState=false;//recording
			_this.m_state=false;//diconnected
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=false;//TODO revise by eventing
				$('bsrstop').disabled=true;
				$('srtype').disabled=false;
				$('srvColors').disabled=false;
				$('srvEncoding').disabled=false;
				$('srvuCompressLevel').disabled=false;
				$('srvCompressLevel').disabled=false;
				$('srvuJpeg').disabled=false;
				$('srvJpeg').disabled=false;
				$('srvRecordStart').disabled=true;
				$('srvRecordStop').disabled=true;
				$('activeXcon').style.display="none";
			}
		}
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	/*@if (@_debug)@*/
			//alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			_this.m_conCompleted=percent;	
			_this.m_conStatus=message;
			ShowConCompleted(percent,message);
		}
		//add other events handler here.
	}
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function(){this.Stop();}//if viewer has sarted, stop it.
	
} // CServiceRCViewer class

/// CServiceRCViewerOpts Remote Control Viewer service options class
function CServiceRCViewerOpts()
{	/// Colors count
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
/// Remote Control Playback service class
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
	/// method. activate service object - the object show ifsalf ...
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	$('srtype').disabled=false;
			$('srPlaybackOptions').style.display="inline";
			if(this.m_activeX!=null)this.m_activeX.style.display="inline";
			if(this.m_activeX==null){$('bsrstart').disabled=false;$('srtype').disabled=false;}//if viewer has not connected start button is enabled
			else {$('bsrstop').disabled=false;$('srtype').disabled=true;}//if viewer has connected stop button is enabled
			$('srvPlayDisplayMode').selectedIndex=this.m_displayMode;
			$('sraccess').outerHTML="<select id=\"sraccess\" style=\"width: 150px\" disabled=\"true\" ><option id=\"0\"></option></select>";
			$('srvPlayFileName').value=this.m_fileName;
			$('srvPlayDelayFactor').selectedIndex=this.m_delayFactor;
			var f=this.m_activeX==null;
			$('srvPlayStart').disabled=f;
			$('srvPlayPause').disabled=f;
			$('srvPlayStop').disabled=f;
		}
		else//Deactivate
		{	$('srtype').disabled=true;
			$('srPlaybackOptions').style.display="none";
			if(this.m_activeX!=null)this.m_activeX.style.display="none";
			$('bsrstart').disabled=true;
			$('bsrstop').disabled=true;
			$('sraccess').disabled=true;
			this.m_fileName=$('srvPlayFileName').value;
			this.m_delayFactor=$('srvPlayDelayFactor').selectedIndex;
			$('srvPlayStart').disabled=true;
			$('srvPlayPause').disabled=true;
			$('srvPlayStop').disabled=true;
		}
	}
	/// start Playback service. Create and connection ActiveX control object (CRCViewerAXCtrl).
	this.Start=function()
	{	if(this.m_activeX!=null){alert("activeX is not delete");return;}
		$('activeX').outerHTML+="<OBJECT classid=\"clsid:DA4679AA-3239-43EF-8D59-6E82AEF6F081\" id=\"AX"+this.m_session.m_name+"\" width=\"100%\" height=\"100%\" ></OBJECT>";
		this.m_activeX = $("AX"+this.m_session.m_name);
		//this.m_activeX.attachEvent("OnDisconnect",this.OnDisconnect());
		this.AttachAXEvents();
		this.m_activeX.StartPlayback($('srvPlayFileName').value);		
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
				$('srvPlayDisplayMode').selectedIndex=this.m_displayMode;	
			}
		}

		//TODO
	}
	/// Set Delay factor
	this.SetDelayFactor=function(factor)
	{	m_delayFactor=factor;
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
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnConnect");
			/*@end@*/
			_this.SetDisplayMode($('srvPlayDisplayMode').selectedIndex);//set display mode
			_this.SetDelayFactor($('srvPlayDelayFactor').selectedIndex);//set delay factor
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=true;// TODO revise by eventing
				$('bsrstop').disabled=false;
				$('srtype').disabled=true;
				$('srvPlayStart').disabled=false;
				$('srvPlayPause').disabled=false;
				$('srvPlayStop').disabled=false;
			}
		}
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_recState=false;//recording
			//if(reason==6)alert('Open file failed.');//TODO log
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=false;//TODO revise by eventing
				$('bsrstop').disabled=true;
				$('srtype').disabled=false;
				$('srvPlayStart').disabled=true;
				$('srvPlayPause').disabled=true;
				$('srvPlayStop').disabled=true;
			}
		}
		
		//add other events handler here.
	}
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function(){this.Stop();}//if viewer has sarted, stop it.
	
} // CServiceRCPlayback class

//------------------------------------------------------------------------------------------
/// Remote Control Host service class
function CServiceRCHost(session)
{	/// session object. it's owner.
	this.m_session=session;
	/// activeX control object CRCHostAXCtrl
	this.m_activeX=hostObject;
	/// internal id session
	this.m_sid=null;
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// percent accomplish connetion
	this.m_conCompleted=0;
	/// current connection message
	this.m_conStatus=" ";
	/// method. activate service object - the object show ifsalf ...
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	//$('srHostOptions').style.display="inline";
			//if(this.m_sid==null){$('bsrstart').disabled=false;$('srtype').disabled=false;}//if service has not connected start button is enabled
			//else {$('bsrstop').disabled=false;$('srtype').disabled=true;}//if service has connected stop button is enabled
			if(this.m_sid!=null)
			{	$('srtype').disabled=true;
				$('bsrstop').disabled=false;
				if(this.m_sid==-2){$('activeXcon').style.display="inline";ShowConCompleted(this.m_conCompleted,this.m_conStatus);}
			}
			else	
			{	$('bsrstart').disabled=false;$('srtype').disabled=false;
			}
			
			
			
			$('sraccess').outerHTML="<select id=\"sraccess\" style=\"width: 150px\" onchange=\"sessions[currentSession].m_service.SetAccessMode($('sraccess').selectedIndex);\" ><option id=\"0\">View Only</option><option id=\"1\">View Only + Visual Pointer</option><option id=\"2\">Full Control</option></select>";
			$('sraccess').selectedIndex=this.m_access;
			
		}
		else//Deactivate
		{	$('srtype').disabled=true;
			$('srHostOptions').style.display="none";
			$('bsrstart').disabled=true;
			$('bsrstop').disabled=true;
			$('sraccess').disabled=true;
			$('activeXcon').style.display="none";
		}
	}
	/// start Host service. Create new connection. (CRCHostAXCtrl).
	this.Start=function()
	{	if(this.m_sid!=null){alert("It have connected already!");return;}
		//this.m_sid=this.m_activeX.StartClient(this.m_session.m_userId,"blah",this.m_session.m_peerId,6000);
		this.m_sid=hostObject.StartClient(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);
		this.m_conCompleted=0;
		this.m_conStatus="start";
		$('activeXcon').style.display="inline"
		ShowConCompleted(this.m_conCompleted,this.m_conStatus);
		$('bsrstart').disabled=true;
		$('bsrstop').disabled=false;
		//if(this.m_sid===-1)this.m_sid=null;//invalid client id
	}
	/// stop Host service. Disconnect. (CRCHostAXCtrl).
	this.Stop=function()
	{	//this.m_activeX.StopClient(this.m_sid);
		/*@if (@_debug)@*///alert("stop");
		/*@end@*/
		if(this.m_sid!=null)hostObject.StopClient(this.m_sid);
	}
	/// Set access mode
	this.SetAccessMode=function(mode)
	{	this.m_access=mode;
		if(this.m_sid!=null)
		{	if(mode==0)//view only
			{	hostObject.SetSessionMode(this.m_sid,0,true);
				hostObject.SetSessionMode(this.m_sid,1,false);
			}
			else if(mode==1)//view only + visual pointer
			{	hostObject.SetSessionMode(this.m_sid,0,true);
				hostObject.SetSessionMode(this.m_sid,1,true);
			}
			else if(mode==2)//full control
			{	hostObject.SetSessionMode(this.m_sid,0,false);
			}
		}
	}
	
	/// NotifySessionStarted event handler
	this.OnNotifySessionStart=function()
		{	this.SetAccessMode(this.m_access);//set session mode
			if(this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=true;
				//$('bsrstop').disabled=false;
				$('srtype').disabled=true;
				$('activeXcon').style.display="none";
			}
		}
	/// NotifySessionStopped event handler.
	/// @param reason stop reason code.
	this.OnNotifySessionStop=function(reason)
		{	
			if(this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=false;
				$('bsrstop').disabled=true;
				$('srtype').disabled=false;
				if(this.m_sid==-2)$('activeXcon').style.display="none";
			}
			this.m_sid=null;
		}
	
	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function(){this.Stop();}//if viewer has sarted, stop it.
} // CServiceRCHost class
//------------------------------------------------------------------------------------------
//+static method CServiceRCHost class
/// recording file name
//hostObject.m_recFileName="untitled";//TODO
function hostObjectInit()
{	
	/// recording state
	hostObject.m_recState=false;
	/// Start and stop recording
	hostObject.statSetSessionRecording=function(fname,mode)
	{	hostObject.SetSessionRecording(fname,mode);
		hostObject.m_recState=mode;
		$('srvHostRecordStop').disabled=!hostObject.m_recState;
	}
	///Client started event handler to AcitveX host object
	/// @param clientId corresponding cliend identifier
	hostObject.statNotifySessionStart=function(clientId)
	{	for(prop in sessions)
			if(sessions[prop]!=null&&sessions[prop].m_type==1&&sessions[prop].m_service.m_sid==-2)//searching host service with temporary id
			{	/*@if (@_debug)@*/alert("hostObjectStarted "+clientId+" "+sessions[prop].m_name);/*@end@*/
				sessions[prop].m_service.m_sid=clientId;//set valid id
				sessions[prop].m_service.OnNotifySessionStart();//call event correspond service object 
				return;}
		/*@if (@_debug)@*/alert("hostObjectStarted "+clientId);/*@end@*/
	}
	/// Client stopped event handler to AcitveX host object
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	/// @param clientId corresponding cliend identifier
	//TODO correct reason codes later here and viewer service. 
	hostObject.statNotifySessionStop=function(clientId,reason)
	{	if(clientId==-1&&reason==6&&hostObject.m_recState)//if recording and OPENFILE_ERROR
		{	hostObject.m_recState=false;
			$('srvHostRecordStop').disabled=true;
		}
		for(prop in sessions)
			if(sessions[prop]!=null&&sessions[prop].m_type==1&&sessions[prop].m_service.m_sid==clientId)
			{	/*@if(@_debug)@*/
				alert("hostObjectStopped "+clientId+" "+reason+" "+sessions[prop].m_name);
				/*@end @*/
				sessions[prop].m_service.OnNotifySessionStop(reason);//call event correspond service object 
				return;}
	/*@if (@_debug)@*/
		alert("hostObjectStopped "+clientId+" "+reason);
	/*@end@*/
	}
	hostObject.statNotifyConnecting=function(percent,message)
		{	for(prop in sessions)
				if(sessions[prop]!=null&&sessions[prop].m_type==1&&sessions[prop].m_service.m_sid==-2)//searching host service with temporary id
				{	sessions[prop].m_service.m_conCompleted=percent;
					sessions[prop].m_service.m_conStatus=message;
					ShowConCompleted(percent,message);
					return;}
		}

	hostObject.statNotifyLogMessage=function(message,severity)
	{	var strseverity="";
		/*@if (@_debug)@*/
		switch(severity)
		{	case 0: strseverity="_EXC ";break;
			case 1: strseverity="_ERR ";break;
			case 2: strseverity="_WAR ";break;
			case 3: strseverity="_MES ";break;
			case 4: strseverity="_SUC ";break;
			default:strseverity="unkn ";
		}
		/*@end @*/
		$('logger').innerText=strseverity+message+"\n"+$('logger').innerText;




		//TODO create clean up old message and big blank space between submessage
	}
	
	
	//TODO it is necessary that do dettachEvent deleting object? dettachEvent don't nowhere.
}





//static method CServiceRCHost class









//------------------------------------------------------------------------------------------
/// Remote Control FileAccessClient service class
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
	/// service access property 0:view only, 1:view only + visual pointer, 2:full control
	this.m_access=0;
	/// method. activate service object - the object show ifsalf ...
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	$('srtype').disabled=false;
			$('srFileAccessClientOptions').style.display="inline";
			if(this.m_activeX!=null)
			{	$('srtype').disabled=true;
				if(this.m_state){this.m_activeX.style.display="inline";$('bsrstop').disabled=false;}
				else {$('activeXcon').style.display="inline";ShowConCompleted(this.m_conCompleted,this.m_conStatus);}
			}
			else	
			{	$('bsrstart').disabled=false;$('srtype').disabled=false;
			}
			
			$('sraccess').outerHTML="<select id=\"sraccess\" style=\"width: 150px\" onclick=\"sessions[currentSession].m_service.SetAccessMode($('sraccess').selectedIndex);\" ><option id=\"0\">Read Only</option><option id=\"1\">Upload Only</option><option id=\"2\">Full Access</option></select>";
			$('sraccess').selectedIndex=this.m_access;
			
		}
		else//Deactivate
		{	$('srtype').disabled=true;
			$('srFileAccessClientOptions').style.display="none";
			if(this.m_activeX!=null)this.m_activeX.style.display="none";
			$('bsrstart').disabled=true;
			$('bsrstop').disabled=true;
			$('sraccess').disabled=true;
			$('activeXcon').style.display="none";
		}
	}
	/// start FileAccessClient service. Create and connection ActiveX control object ().
	this.Start=function()
	{	if(this.m_activeX!=null){alert("activeX is not delete");return;}
		$('activeX').outerHTML+="<OBJECT classid=\"clsid:E7AC108B-14CF-48CB-8B7F-8974F6E6C2AC\" id=\"AX"+this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX = $("AX"+this.m_session.m_name);
		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		$('activeXcon').style.display="inline"
		ShowConCompleted(this.m_conCompleted,this.m_conStatus);
		$('bsrstart').disabled=true;
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
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
		//if(this.m_activeX!=null)
		//{	if(mode==0)//view only
		//	{	this.m_activeX.SetSessionMode(0,true);
		//	}
		//	else if(mode==1)//view only + visual pointer
		//	{	this.m_activeX.SetSessionMode(0,true);
		//		this.m_activeX.SetSessionMode(1,true);
		//	}
		//	else if(mode==2)//full control
		//	{	this.m_activeX.SetSessionMode(0,false);
		//	}
		//}
	}
	///Attach events handler to AcitveX FileAccessClient object
	this.AttachAXEvents=function()
	{	var _this=this;
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnConnect");
			/*@end@*/
			_this.m_state=true;//diconnected
			_this.m_activeX.height="100%";
			_this.m_activeX.width="100%";
			//if(_this.m_access!=2)//not a full control TODO: make safe machanism set access mode on host side in this. now not always. inner viewer setaccess carry out after host do this.
			//	_this.SetAccessMode(_this.m_access);//set session mode
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=true;// TODO revise by eventing
				$('bsrstop').disabled=false;
				$('srtype').disabled=true;
				$('activeXcon').style.display="none";
			}
		}
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_recState=false;//recording
			_this.m_state=false;//diconnected
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=false;//TODO revise by eventing
				$('bsrstop').disabled=true;
				$('srtype').disabled=false;
				$('activeXcon').style.display="none";
			}
		}
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	/*@if (@_debug)@*/
			//alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			_this.m_conCompleted=percent;	
			_this.m_conStatus=message;
			ShowConCompleted(percent,message);
		}
		//add other events handler here.
	}
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function(){this.Stop();}//if viewer has sarted, stop it.
	
} // CServiceRCFileClient class





//------------------------------------------------------------------------------------------
/// Remote Control FileAccessHost service class

//TODO change in comment FileAccessClient on FileAccessHost

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
	/// method. activate service object - the object show ifsalf ...
	/// @param flag bool parameter. if it's true - session activates, else session deactivetes.
	this.Activate=function(flag)
	{	if(flag)//Activate
		{	$('srtype').disabled=false;
			$('srFileAccessHostOptions').style.display="inline";
			if(this.m_activeX!=null)
			{	$('srtype').disabled=true;
				if(this.m_state){this.m_activeX.style.display="inline";$('bsrstop').disabled=false;}
				else {$('activeXcon').style.display="inline";ShowConCompleted(this.m_conCompleted,this.m_conStatus);}
			}
			else	
			{	$('bsrstart').disabled=false;$('srtype').disabled=false;
			}
			
			$('sraccess').outerHTML="<select id=\"sraccess\" style=\"width: 150px\" onclick=\"sessions[currentSession].m_service.SetAccessMode($('sraccess').selectedIndex);\" ><option id=\"0\">Read Only</option><option id=\"1\">Upload Only</option><option id=\"2\">Full Access</option></select>";
			$('sraccess').selectedIndex=this.m_access;
			
		}
		else//Deactivate
		{	$('srtype').disabled=true;
			$('srFileAccessHostOptions').style.display="none";
			if(this.m_activeX!=null)this.m_activeX.style.display="none";
			$('bsrstart').disabled=true;
			$('bsrstop').disabled=true;
			$('sraccess').disabled=true;
			$('activeXcon').style.display="none";
		}
	}
	/// start FileAccessClient service. Create and connection ActiveX control object ().
	this.Start=function()
	{	if(this.m_activeX!=null){alert("activeX is not delete");return;}
		$('activeX').outerHTML+="<OBJECT classid=\"clsid:BFC3A266-A3A0-4F25-901C-730CF6DC3554\" id=\"AX"+this.m_session.m_name+"\" width=\"0%\" height=\"0%\" ></OBJECT>";//style=\"display:none\"
		this.m_activeX = $("AX"+this.m_session.m_name);
		this.AttachAXEvents();
		this.m_conCompleted=0;
		this.m_conStatus="start";
		$('activeXcon').style.display="inline"
		ShowConCompleted(this.m_conCompleted,this.m_conStatus);
		$('bsrstart').disabled=true;
		this.m_activeX.Mediator = $('IMMediatorObject');
		this.m_activeX.Start(this.m_session.m_userId,this.m_session.m_password,this.m_session.m_peerId,relayServer,30000);		
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
		//if(this.m_activeX!=null)
		//{	if(mode==0)//view only
		//	{	this.m_activeX.SetSessionMode(0,true);
		//	}
		//	else if(mode==1)//view only + visual pointer
		//	{	this.m_activeX.SetSessionMode(0,true);
		//		this.m_activeX.SetSessionMode(1,true);
		//	}
		//	else if(mode==2)//full control
		//	{	this.m_activeX.SetSessionMode(0,false);
		//	}
		//}
	}
	///Attach events handler to AcitveX FileAccessClient object
	this.AttachAXEvents=function()
	{	var _this=this;
		this.m_activeX.attachEvent("NotifySessionStart",OnNotifySessionStarted);
		/// NotifySessionStart event handler
		function OnNotifySessionStarted()
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnConnect");
			/*@end@*/
			_this.m_state=true;//diconnected
			_this.m_activeX.height="100%";
			_this.m_activeX.width="100%";
			//if(_this.m_access!=2)//not a full control TODO: make safe machanism set access mode on host side in this. now not always. inner viewer setaccess carry out after host do this.
			//	_this.SetAccessMode(_this.m_access);//set session mode
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=true;// TODO revise by eventing
				$('bsrstop').disabled=false;
				$('srtype').disabled=true;
				$('activeXcon').style.display="none";
			}
		}
		this.m_activeX.attachEvent("NotifySessionStop",OnNotifySessionStopped);
		/// NotifySessionStop event handler.
		/// @param reason stop reason code.
		function OnNotifySessionStopped(reason)
		{	/*@if (@_debug)@*/
			alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			_this.m_activeX.removeNode(true);//delete activeX object
			_this.m_activeX=null;
			_this.m_recState=false;//recording
			_this.m_state=false;//diconnected
			if(_this.m_session.m_name==currentSession)//this active session
			{	$('bsrstart').disabled=false;//TODO revise by eventing
				$('bsrstop').disabled=true;
				$('srtype').disabled=false;
				$('activeXcon').style.display="none";
			}
		}
		this.m_activeX.attachEvent("NotifyConnecting",OnNotifyConnecting);
		/// NotifyConnecting event handler.
		/// @param percent percent completed.
		/// @param message hint current process connecting.
		function OnNotifyConnecting(percent,message)
		{	/*@if (@_debug)@*/
			//alert("Session '"+_this.m_session.m_name+"'\nOnDisconnect="+reason);
			/*@end@*/
			_this.m_conCompleted=percent;	
			_this.m_conStatus=message;
			ShowConCompleted(percent,message);
		}
		//add other events handler here.
	}
	

	/// method. it is nonautomatic destructer. A user mast to invoke it befor delete object.
	this.foreDestroy=function(){this.Stop();}//if viewer has sarted, stop it.
	
} // CServiceRCFileClient class

//------------------------------------------------------------------------------------------









//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------