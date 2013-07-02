	
	var DEBUG=false;
	var gTimer;
	var gCurrentView = "CMCallsListView";
    var gCurrentPannel = "CMLoginPanel";
    var gShowFooterPannels = ['CMLoginPanel','CMInboxPanel']
    var gHideFooterPannels = ['CMSettingsPanel','CMAboutPanel']
    var gPasswordChanged = false;
    
    /********************** COMMON FUNCTIONS ************************/
   
    function inArray(elt,arr)
    {
        return (arr.inspect().indexOf(elt) >= 0);
    }
    
    /*** 
     * switchView: Show the div and hide all its siblings     
     * @param {string} viewID - id of the div to show 
     */
    function switchView(viewID)
    {        
        gCurrentView = viewID;
    	
    	$(viewID).show();
        $(viewID).siblings().each(Element.hide)
    }
    
    function switchPanel(viewID)
    {
        gLastPanel = gCurrentPannel;
        gCurrentPannel = viewID;        
        switchView(viewID)
        
        if (inArray(viewID,gShowFooterPannels))
            showFooterPannel()
        if (inArray(viewID,gHideFooterPannels))
            hideFooterPannel()
    }
    
    function hideFooterPannel()
    {
       $('trFooter').hide()
       $('CMLoginFooterStart').style.visibility = 'hidden'        
       switchView("CMLoginFooterStart");
    }
    
    function showFooterPannel()
    {
       $('trFooter').show()
       $('CMLoginFooterStart').style.visibility = 'visible'        
       switchView("CMLoginFooterStart");
    }

   	function setHelpAboutUI(version)
    {
        $('dCMVersion').innerHTML = version; 
    	switchPanel("CMAboutPanel");
    }
    
    function showLastPanel()
    {
        switchPanel(gLastPanel);
    }

    function initTexts(sData)
    {
       var texts = eval(sData);
       for (var key in texts)
       {
           var tmp = texts[key];
       }
    }
    
	function MsngerBodyOnLoad()
	{
        CALLBACK_PageLoaded(); // Tell to Shuttle that the page is loaded
        Event.observe('cPortOpened','click',setConnectionPortStateUI);
        Event.observe('cPortAutoconfigure','click',function(){setConnectionTestStateUI('configure','Use UPnP to open the TCP port and test it')});
	}

    /*** Launched when clicking on Body ***/
    function onBodyClicked()
    {
        closeMenuStatuses()    
    }

    /********
     * scrollUp:manage a scroll up action
     * @param {string} divID
     */
    function scrollUp(divID)
    {
    
    	var div = $(divID);
    	if (!div) return;
    	
    	clearInterval(gTimer);
    	
    	var dScrollUp = $(divID+"Up");
    	var dScrollDown = $(divID+"Down");
    	
    	showObject(dScrollDown, true);
    	if (div.scrollTop>0)
    	{
    		div.scrollTop -= 15;
    		showObject(dScrollUp, true);
    		gTimer = window.setInterval("scrollUp(\'"+divID+"\')",200);
    	}
    	if (div.scrollTop<=0)
    		showObject(dScrollUp, false);
    }
    
    /********
     * scrollDown:manage a scroll down action
     * @param {string} divID
     */
    function scrollDown(divID)
    {
    	var div = $(divID);
    	if (!div) return;
    
    	clearInterval(gTimer);
    
    	var dScrollUp = $(divID+"Up");
    	var dScrollDown = $(divID+"Down");
    	
    	showObject(dScrollUp, true);
    	//window.status = (div.scrollTop + div.offsetHeight) + ' - '+ div.scrollHeight;
    	if (div.scrollTop + div.offsetHeight < div.scrollHeight)
    	{
    		div.scrollTop += 15;
    		showObject(dScrollDown, true);
    		gTimer = window.setInterval("scrollDown(\'"+divID+"\')",200);
    	}
    	if (div.scrollTop + div.offsetHeight >= div.scrollHeight)
    		showObject(dScrollDown, false);	
    }
    /*******
     * stopScroll:stop the scrolling
     */
    function stopScroll()
    {
    	clearInterval(gTimer);
    }

    /********************  LOGIN FUNCTIONS  *********************/
    
    function OnLoginStartClicked()
    {
        $('startBtn').disabled = true;
		$('startBtn').title = 'Sign in';
        switchView("CMLoginFooterConnecting");

		var sEmail = $('loginEmail').value;
		var sPassword = $('loginPwd').value;
		var sloginRememberMe = $('loginRememberMe').checked;
		var sLoginStatus = '' //$('loginStatus').value;
		
        $('CMLoginFormError').innerHTML = '&nbsp;';

		CALLBACK_LoginPageStart(sEmail,sPassword,sloginRememberMe,sLoginStatus);
    }
   
    function setLoginInfoUI(email, rememberme)
    {
        $('loginEmail').value = email;
        if (rememberme)
        {
            $('loginRememberMe').checked = true;
            $('loginPwd').value = '*#@*$&_()[.]';
        }
    }

    function setLoginState(state)
    {
		var iState = parseInt(state);

        var bubbleImg = $('CMLoginFooterConnecting').down(iState-1);
        if (bubbleImg)
            bubbleImg.src = 'img/btn_status_menu_busy_14x14.gif';
        if (iState == 5)
            MsngerOnLogin()
    }

    function setLogoutUI(sErrDescMsg)
    {
    	switchPanel("CMLoginPanel");
        $('startBtn').disabled = false;
		$('startBtn').title = 'Sign in';

		if($('loginRememberMe').checked == false)
            $('loginPwd').value = '';

        $('CMLoginFooterConnecting').immediateDescendants().each(function(obj){obj.src='img/btn_status_menu_away_14x14.gif'})    
		
		if(sErrDescMsg != "")
			$('CMLoginFormError').innerHTML = sErrDescMsg;
    }


    function MsngerOnLogin()
    {
        $('startBtn').disabled = false;
		$('startBtn').title = 'Sign in';
        $('CMLoginFooterConnecting').immediateDescendants().each(function(obj){obj.src='img/btn_status_menu_away_14x14.gif'})         
    }
    
    

    
    /***********************  BUILDING HTML FUNCTIONS  ****************************/
	function MsngerBuildCallItem(oCallItem)
	{
		var html = '';
		
		var iCallUID = oCallItem.iCallUID;
		var sCallDirection = oCallItem.sCallDirection;
		var sCallType = oCallItem.sCallType;
		var sCustomerName = oCallItem.oCustomer.sCustomerName;
		var sCallProblemDescription = oCallItem.sCallProblemDescription;
		var sCallCategoryPath = oCallItem.sCallCategoryPath;
		var sElapsedTime = oCallItem.sElapsedTime;
		var sSnoozeTimer = oCallItem.sSnoozeTimer;
		var sCallFlagStatus = oCallItem.sCallFlagStatus;
				
		html += '\
		<div id="dCallItem_'+iCallUID+'" class="dCallItem" onmouseenter="showActions('+iCallUID+',true)" onmouseleave="showActions('+iCallUID+',false)">\
			<div id="dCallItemActions_'+iCallUID+'" class="dCallItemActions" style="display:none">'+
				actionHeaderHTML(iCallUID, sCustomerName, sCallFlagStatus)+'\
			</div>\
			<div style="margin-top:5px;">\
				<table id="tableCallItem_'+iCallUID+'" width="100%" border="0" CELLSPACING="0" CELLPADDING="0">\
				<tr>\
					<td class="tdCallRow1" valign="top">\
						<div class="dCallDirectionType">\
							<!--div class="dCallDirection"></div>\
							<div class="dCallType"></div-->\
							<img src="img/btn_direct_call_26x16.gif" alt=""/>\
						</div>\
						<div class="dTimers">\
							<div class="dElapsedTimer"><div class="dElapsedClock"></div><div class="dElapsedTime">00:12:15</div></div>\
							<div class="dSnoozeTimer"><div class="dSnoozeClock"></div><div class="dSnoozeTime">00:01:40</div></div>\
						</div>\
					</td>\
					<td class="tdCallRow2">\
						<div class="dNameFlag">\
							<table border="0" CELLSPACING="0" CELLPADDING="0">\
							<tr>\
								<td class="dCustomerName">'+sCustomerName+'</td>\
								<td class="dCustomerFlag"><img name="Flag" id="imgFlag_'+iCallUID+'" src="'+gFlagsImgs[sCallFlagStatus]+'"></td>\
							</tr>\
							</table>\
						</div>\
						<div class="dCallProblemDescription">'+sCallProblemDescription+'</div>\
						<div class="dCallCategoryPath">'+sCallCategoryPath+'</div>\
					</td>\
				</tr>\
				</table>\
			</div>\
		</div>';
	
		return html;
	}
	
	function MsngerDisplayCallItems()
	{
		var html = '';
		var CMCallsListView = $("CMCallsListView");
		
		for (var i=0;i<gCallItemsList.length;i++)
		{
			html += MsngerBuildCallItem(gCallItemsList[i])
		}
		
		CMCallsListView.innerHTML = html;
	}
	
	function deleteCallUI(iCallUID)
	{
		if ($('dCallItem_'+iCallUID))
            Element.remove('dCallItem_'+iCallUID);
        //CMInboxNumber.innerHTML = gCallItemsList.length;

		//CALLBACK_NewCallDeleted(iCallUID);
	}
    
    function dropCallsUI()
    {
        $("CMCallsListView").innerHTML = '';
    }
	
	function addCallUI(oCallItem, sCalls)
	{
		var CMCallsListView = $("CMCallsListView");
		var html = MsngerBuildCallItem(oCallItem);
		
		CMCallsListView.innerHTML = html + CMCallsListView.innerHTML;
		       
        CMInboxNumber.innerHTML = gCallItemsList.length
	}
    
    function moveCalltoInSession(iCallUID)
    {
        var dCallItem = $('dCallItem_'+iCallUID);
		//document.getElementById('dCallItem_'+iCallUID).style.display = "none";
		if (dCallItem)
			dCallItem.outerHTML = '';
        CMInboxNumber.innerHTML = gCallItemsList.length;        
    }
	
	/**********
	 * 
	 * @param {int} iCallUID - CallItem id
	 * @param {boolean} showit - show/hide
	 */
	function showActions(iCallUID, showit)
	{
		window.status = window.event.srcElement.className.match("dCallItemActions") + ' -- ' + showit
		/*if (window.event.srcElement.className.match("dCallItemActions") && showit)
			return;
		*/
/*			if (isAncestor('dCallItem_'+iCallUID, window.event.srcElement) && (showit))
			return
*/
		var dCallItem = $('dCallItem_'+iCallUID);
		var dCallItemActions = $('dCallItemActions_' + iCallUID);
		var tableCallItem = $('tableCallItem_' + iCallUID);
		
		dCallItem.filters[0].apply();
        if (showit) dCallItemActions.show(); else dCallItemActions.hide();
        if (showit) tableCallItem.hide(); else tableCallItem.show();
		dCallItem.filters[0].play();
		
	}
	          
    
    /***
     * 
     * @param {Object} sAction
     * @param {Object} forceActionToNextReopen
     */
	function funcActionMouseClick(sAction,forceActionToNextReopen)
	{
		var CMWindow = $("CMWindow")
		if (CMWindow)
			CMWindow.style.display ="";

		CMWindow.style.visibility="hidden";
		CMWindow.filters.item("DXImageTransform.Microsoft.Fade").Duration=2;
		CMWindow.filters[0].Apply();
		CMWindow.style.visibility="visible";
		CMWindow.filters[0].Play();
			
		var o=$("trRequestAction"+sAction+"Body");
		var oi=$("img"+sAction+"ActionButton");
	}

    
    /*****
     * displayCallsListView: Display the Calls List
     */
    function displayCallsListOrPbInfoView(iCallUID)
    {
    	if (gCurrentView == "CMCallsListView")
    		displayPbInfoView(iCallUID)
    	else
    		switchView("CMCallsListView");
    }
    
    // START: PB info info info info info info info info info info info info info info info info info info info info info info info info ..............
    
    /****
     * 
     * @param {Object} iCallUID
     */
    function displayPbInfoView(iCallUID)
    {
    	var html = '';
    	var oCallItem = CallItem.getCallItemById(iCallUID);
    	var sCustomerName = oCallItem.oCustomer.sCustomerName;
    	var sCallFlagStatus = oCallItem.sCallFlagStatus;
    	
    	html += actionHeaderHTML(iCallUID,sCustomerName,sCallFlagStatus)+
    			'<table class="VSBT" border="1" width="100%" CELLSPACING="0" CELLPADDING="0">\
    			<tr>\
    				<td colspan="2" class="VSBBT">Problem Information:</td>\
    			</tr>\
    			<tr>\
    				<td valign="top" class="VSBBT">Date:</td>\
    				<td>'+oCallItem.sCallArrivalDate+'</td>\
    			</tr>\
    			<tr>\
    				<td valign="top" class="VSBBT">Time:</td>\
    				<td>'+oCallItem.sCallArrivalTime+'</td>\
    			</tr>\
    			<tr>\
    				<td valign="top" width="50" class="VSBBT">Category:</td>\
    				<td>'+oCallItem.sCallCategoryPath+'</td>\
    			</tr>\
    			<tr>\
    				<td valign="top" width="50" class="VSBBT">Problem:</td>\
    				<td>'+oCallItem.sCallProblemDescription+'</td>\
    			</tr>\
    			<tr>\
    				<td valign="top" width="50" class="VSBBT">Action:</td>\
    				<td>'+oCallItem.iLastActionTaken+'</td>\
    			</tr>\
    			</table>';
    	
    	var CMPbInfoView = $("CMPbInfoView");		
    	CMPbInfoView.innerHTML = html; 
    	switchView("CMPbInfoView");
    }

// END: PB info info info info info info info info info info info info info info info info info info info info info info info info ..............

// START: Customer info customer customer customer customer customer customer customer customer customer customer customer customer ..............

	function displayCustInfoView(iCallUID)
	{
		var html = '';
		var oCallItem = CallItem.getCallItemById(iCallUID);
		var sCallFlagStatus = oCallItem.sCallFlagStatus
		var oCust = oCallItem.oCustomer;
		var sCustomerName = oCust.sCustomerName;
		var sCustomerRatingValue = oCust.sCustomerRatingValue;
		var sCustomerCharacter = oCust.sCustomerCharacter;
		var sCustomerSkills = oCust.sCustomerSkills;
		var sCustomerInstalledFeatures = oCust.sCustomerInstalledFeatures;
		var iCustomerTimeZone = oCust.iCustomerTimeZone;
		var sCustomerVesinity = oCust.iCustomerVesinity + ' ' + oCust.sCustomerVesinityUnit;
		var sCustomerLanguages = oCust.sCustomerLanguages;
		var iCustomerUID = oCust.iCustomerUID;
		
		html += actionHeaderHTML(iCallUID,sCustomerName,sCallFlagStatus)+
				'<table class="VSBT" border="1" width="100%" CELLSPACING="0" CELLPADDING="0">\
				<tr>\
					<td colspan="2" class="VSBBT">Customer Details:</td>\
				</tr>\
				<tr>\
					<td valign="top" class="VSBBT">Name:</td>\
					<td>'+sCustomerName+'</td>\
				</tr>\
				<tr>\
					<td valign="top" class="VSBBT">Rating:</td>\
					<td>'+sCustomerRatingValue+'</td>\
				</tr>\
				<tr>\
					<td valign="top" width="50" class="VSBBT">Character:</td>\
					<td>'+sCustomerCharacter+'</td>\
				</tr>\
				<tr>\
					<td valign="top" width="50" class="VSBBT">Skills:</td>\
					<td>'+sCustomerSkills+'</td>\
				</tr>\
				<tr>\
					<td valign="top" width="50" class="VSBBT">Tools:</td>\
					<td>'+sCustomerInstalledFeatures+'</td>\
				</tr>\
				<tr>\
					<td valign="top" class="VSBBT">Local time:</td>\
					<td>'+iCustomerTimeZone+'</td>\
				</tr>\
				<tr>\
					<td valign="top" width="50" class="VSBBT">Visinity:</td>\
					<td>'+sCustomerVesinity+'</td>\
				</tr>\
				<tr>\
					<td valign="top" width="50" class="VSBBT">Languages:</td>\
					<td>'+sCustomerLanguages+'</td>\
				</tr>\
				</table>';
		
		var CMCustDetailsView = $("CMCustDetailsView");		
		CMCustDetailsView.innerHTML = html; 
		switchView("CMCustDetailsView");
		
		CALLBACK_GoToCustomerProfile(iCallUID,iCustomerUID);
	}
	
// END: Customer info customer customer customer customer customer customer customer customer customer customer customer customer ..............

// START: flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags

    function displayFlagsView(iCallUID)
    {
    	var html = '';
    	var oCallItem = CallItem.getCallItemById(iCallUID);
    	var sCallFlagStatus = oCallItem.sCallFlagStatus;
    	var sCustomerName = oCallItem.oCustomer.sCustomerName;
    	
    	html += actionHeaderHTML(iCallUID,sCustomerName,sCallFlagStatus)+
    			'<div>\
    				<table class="VSBT" border="1" width="100%" CELLSPACING="0" CELLPADDING="0">\
    				<tr>\
    					<td colspan="2" class="VSBBT">Mark with flag:</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" class="VSBBT">Flags:</td>\
    					<td>'+buildFlags(iCallUID)+'</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" class="VSBBT">Other:</td>\
    					<td>\
    						<div class="dScrollItem" onmouseover="addClass(this,\'dScrollItemHover\')" onmouseout="removeClass(this,\'dScrollItemHover\')" onclick="flagMenuSelected('+iCallUID+',\'Complete\')"><div style="float:left;"><img src="img/Icon-Complete-15x15.gif"></div><div>Flag complete</div></div>\
    						<div class="dScrollItem" onmouseover="addClass(this,\'dScrollItemHover\')" onmouseout="removeClass(this,\'dScrollItemHover\')" onclick="flagMenuSelected('+iCallUID+',\'Cleared\')"><div style="float:left;"><img src="img/Icon-White-15x15.gif"></div><div>Clear Flag</div></div>\
    					</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" width="50" class="VSBBT">Actions:</td>\
    					<td><span class="sManageAction" onclick="flagMenuSelected('+iCallUID+',\'Manage\')">Manage Flags</span></td>\
    				</tr>\
    				</table>\
    			</div>';
    
    	var CMFlagsView = $("CMFlagsView")		
    	CMFlagsView.innerHTML = html;
    	switchView("CMFlagsView");
    }
    
    /*******
     * buildFlags: build color flags...
     */
    function buildFlags(iCallUID)
    {
    	var html = '';
    	var iDisplayedFlags = 7;
    	var iFlagsLength = gFlags.length;	
    	var oversized= (iDisplayedFlags<iFlagsLength);
    	
    	var h = iDisplayedFlags*15;
    	if (iFlagsLength < 7)
    		var h = iFlagsLength*15;	
    	
    	if (oversized)
    		html += '<div id="dFlagsUp" class="unvisible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollUp(\'dFlags\')"><button style="width:100px;">UP</button></div>';	
    	html += '<div id="dFlags" class="dScrollList" style="height:'+h+'">';
    	for (var i=0;i<gFlags.length;i++)
    		html += '<div class="dScrollItem" onmouseover="addClass(this,\'dScrollItemHover\')" onmouseout="removeClass(this,\'dScrollItemHover\')" onclick="flagMenuSelected('+iCallUID+',\''+gFlags[i].color+'\')"><div style="float:left;"><img src="img/Icon-'+gFlags[i].color+'-15x15.gif"></div><div>'+gFlags[i].label+'</div></div>' 
    	html += '</div>'
    	if (oversized)
    		html += '<div id="dFlagsDown" class="visible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollDown(\'dFlags\')"><button  style="width:100px;">DOWN</button></div>';
    	
    	return html;
    }
    
    function flagMenuSelected(iCallUID,sFlagStatus)
    {
    	var oCallItem = CallItem.getCallItemById(iCallUID);
    	if (sFlagStatus != "Manage")
    	oCallItem.sCallFlagStatus = sFlagStatus;
    	
    	switch (sFlagStatus)
    	{
    		case "Manage":
    		{
    			CALLBACK_GoToManageFlags(iCallUID)
    			break;
    		}
    
    		case "Cleared":
    		{
    			updateFlag("img/Icon-White-15x15.gif",iCallUID);
    			CALLBACK_FlagChanged(iCallUID,"Cleared");
    
    			break;
    		}
    
    		default: // complete and other colors
    		{
    			updateFlag("img/Icon-"+sFlagStatus+"-15x15.gif",iCallUID);
    			CALLBACK_FlagChanged(iCallUID,sFlagStatus);
    
    			break;
    		}
    	}
    
    	//funcActionMouseClick("Mark",null,null);
    }
    
    function updateFlag(src,iCallUID)
    {
    	var flags = document.getElementsByName("Flag");
    	for (var i=0;i<flags.length;i++)
    	{
    		var flag = flags[i];
    		if (flag.id.indexOf(iCallUID) > 0)
    			flag.src = src;
    	}
    }
    
    // END: flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags flags
    
    // START: forward forward forward forward forward forward forward forward forward forward forward forward forward forward 
    
    function displayForwardView(iCallUID)
    {
    	var html = '';
    	var oCallItem = CallItem.getCallItemById(iCallUID);
    	var sCallFlagStatus = oCallItem.sCallFlagStatus
    	var sCustomerName = oCallItem.oCustomer.sCustomerName;
    	
    	html += actionHeaderHTML(iCallUID,sCustomerName, sCallFlagStatus)+
    			'<div>\
    				<table class="VSBT" border="1" width="100%" CELLSPACING="0" CELLPADDING="0">\
    				<tr>\
    					<td colspan="2" class="VSBBT">Forward this call to:</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" class="VSBBT">Contacts:</td>\
    					<td>'+buildFwdContacts(iCallUID)+'</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" width="50" class="VSBBT">Actions:</td>\
    					<td><span class="sManageAction" onclick="fwdMenuSelected(\'Manage\','+iCallUID+')">Manage Forward</span></td>\
    				</tr>\
    				</table>\
    			</div>';
    	
    	var CMForwardView = $("CMForwardView")		
    	CMForwardView.innerHTML = html; 
    	switchView("CMForwardView");
    }
    
    function buildFwdContacts(iCallUID)
    {
    	var html = '';
    	var iDisplayedContacts = 6;
    	var iContactsLength = gContacts.length;	
    	var oversized= (iDisplayedContacts<iContactsLength);
    	
    	var h = iDisplayedContacts*15;
    	if (iContactsLength < 7)
    		var h = iContactsLength*15;	
    	
    	if (oversized)
    		html += '<div id="dContactsUp" class="unvisible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollUp(\'dContacts\')"><button style="width:100px;">UP</button></div>';	
    	html += '<div id="dContacts" class="dScrollList" style="height:'+h+'">';
    	for (var i=0;i<gContacts.length;i++)
    		html += '<div class="dScrollItem" onclick="fwdMenuSelected(\'Forward\','+iCallUID+','+gContacts[i].id+')" onmouseover="addClass(this,\'dScrollItemHover\')" onmouseout="removeClass(this,\'dScrollItemHover\')" ><div style="float:left;"><img src="img/Icon-Favorite_Blue-15x15.gif"></div><div>'+gContacts[i].name+'</div></div>' 
    	html += '</div>'
    	if (oversized)
    		html += '<div id="dContactsDown" class="visible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollDown(\'dContacts\')"><button  style="width:100px;">DOWN</button></div>';
    	
    	return html;
    }
    
    	function fwdMenuSelected(sCmd, iCallUID, iCustID)
    	{
    		switch (sCmd)
    		{
    			case "Manage":
    			{
    				CALLBACK_GoToManageContacts(iCallUID);
    				break;
    			}
    
    			default: // the user it has been forwarded to
    			{
    				CALLBACK_FwdToContact(iCallUID, iCustID)
    				break;
    			}
    		}
    
    		//funcActionMouseClick("Forward",null,null);
    	}
    
    // END: forward forward forward forward forward forward forward forward forward forward forward forward forward forward 
    
    // START: snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze 
    
    function displaySnoozeView(iCallUID)
    {
    	var html = '';
    	var oCallItem = CallItem.getCallItemById(iCallUID);
    	var sCallFlagStatus = oCallItem.sCallFlagStatus;
    	var sCustomerName = oCallItem.oCustomer.sCustomerName;
    	
    	html += actionHeaderHTML(iCallUID,sCustomerName, sCallFlagStatus)+
    			'<div>\
    				<table class="VSBT" border="1" width="100%" CELLSPACING="0" CELLPADDING="0">\
    				<tr>\
    					<td colspan="2" class="VSBBT">Snooze Call by:</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" class="VSBBT">Snooze:</td>\
    					<td>'+buildSnoozes()+'</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" width="50" class="VSBBT">Actions:</td>\
    					<td><span class="sManageAction" onclick="snoozeMenuSelected(\'Calender\','+iCallUID+')">Schedule Request</span></td>\
    				</tr>\
    				</table>\
    			</div>';
    	
    	var CMSnoozeView = $("CMSnoozeView")		
    	CMSnoozeView.innerHTML = html; 
    	switchView("CMSnoozeView"); 
    }
    
    function buildSnoozes(iCallUID)
    {
    	var html = '';
    	var iDisplayedSnoozes = 5;
    	var iSnoozesLength = gSnoozes.length;	
    	var oversized= (iDisplayedSnoozes<iSnoozesLength);
    	
    	var h = iDisplayedSnoozes*15;
    	if (iSnoozesLength < 7)
    		var h = iSnoozesLength*15;	
    	
    	if (oversized)
    		html += '<div id="dSnoozesUp" class="unvisible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollUp(\'dSnoozes\')"><button style="width:100px;">UP</button></div>';	
    	html += '<div id="dSnoozes" class="dScrollList" style="height:'+h+'">';
    	for (var i=0;i<gSnoozes.length;i++)
    	{
    		if (gSnoozes[i].time<3600)
    			var img = 'Icon-TimerMinutes-15x15.gif';
    		else 	
    			var img = 'Icon-TimerHours-15x15.gif';
    		
    		html += '<div class="dScrollItem" onclick="snoozeMenuSelected(\'Snooze\','+iCallUID+','+gSnoozes[i].time+')" onmouseover="addClass(this,\'dScrollItemHover\')" onmouseout="removeClass(this,\'dScrollItemHover\')" ><div style="float:left;"><img src="img/'+img+'"></div><div>'+gSnoozes[i].label+'</div></div>'
    	} 
    	html += '</div>'
    	if (oversized)
    		html += '<div id="dSnoozesDown" class="visible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollDown(\'dSnoozes\')"><button  style="width:100px;">DOWN</button></div>';
    	
    	return html;
    }
    
    	function snoozeMenuSelected(sCmd, iCallUID, iSec)
    	{
    		switch (sCmd)
    		{
    			case "Calender":
    			{
    				CALLBACK_GoToCalender(iCallUID);
    				break;
    			}
    
    			default: // complete and other colors
    			{
    				CALLBACK_Snooze(iCallUID,iSec);
    				break;
    			}
    		}
    
    		//funcActionMouseClick("Later",null,null);
    	}
    
    // END: snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze snooze 
    
    // START: reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply
    
    function displayReplyView(iCallUID)
    {
    	var html = '';
    	var oCallItem = CallItem.getCallItemById(iCallUID);
    	var sCallFlagStatus = oCallItem.sCallFlagStatus;
    	var sCustomerName = oCallItem.oCustomer.sCustomerName;
    	
    	html += actionHeaderHTML(iCallUID,sCustomerName,sCallFlagStatus)+
    			'<div>\
    				<table class="VSBT" border="1" width="100%" CELLSPACING="0" CELLPADDING="0">\
    				<tr>\
    					<td colspan="2" class="VSBBT">Reject call and send reply:</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" class="VSBBT">Replies:</td>\
    					<td>'+buildReplies(iCallUID)+'</td>\
    				</tr>\
    				<tr>\
    					<td valign="top" class="VSBBT">Custom:</td>\
    					<td><input id="customReply" type="text" style="width:70px;font-size:10px;" value="Type reply..." \>&nbsp;<button onclick="replyMenuSelected(\'CustomReply\','+iCallUID+')">Send</button></td>\
    				</tr>\
    				<tr>\
    					<td valign="top" width="50" class="VSBBT">Actions:</td>\
    					<td><span class="sManageAction" onclick="replyMenuSelected(\'Manage\','+iCallUID+')">Manage Replies</span></td>\
    				</tr>\
    				</table>\
    			</div>';
    	
    	var CMReplyView = $("CMReplyView")		
    	CMReplyView.innerHTML = html; 
    	switchView("CMReplyView"); 		
    }
    
    function buildReplies(iCallUID)
    {
    	var html = '';
    	var iDisplayedReplies = 5;
    	var iRepliesLength = gReplies.length;		
    	var oversized= (iDisplayedReplies<iRepliesLength);
    	
    	var h = iDisplayedReplies*15;
    	if (iRepliesLength < 7)
    		var h = iRepliesLength*15;	
    
    	if (oversized)
    		html += '<div id="dRepliesUp" class="unvisible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollUp(\'dReplies\')"><button style="width:100px;">UP</button></div>';	
    	html += '<div id="dReplies" class="dScrollList" style="height:'+h+'">';
    	for (var i=0;i<gReplies.length;i++)
    		html += '<div class="dScrollItem" onclick="replyMenuSelected(\'Reply\','+iCallUID+','+gReplies[i].id+')" onmouseover="addClass(this,\'dScrollItemHover\')" onmouseout="removeClass(this,\'dScrollItemHover\')" ><div style="float:left;"><img src="img/Icon-'+gReplies[i].color+'-15x15.gif"></div><div>'+gReplies[i].label+'</div></div>' 
    	html += '</div>'
    	if (oversized)
    		html += '<div id="dRepliesDown" class="visible" onmouseout="stopScroll()" onmouseup="stopScroll()" onmousedown="scrollDown(\'dReplies\')"><button  style="width:100px;">DOWN</button></div>';
    	
    	return html;
    }
    
    /***
     * 
     * @param {Object} sCmd
     * @param {Object} iCallUID
     * @param {Object} iReplyUID
     */
    function replyMenuSelected(sCmd,iCallUID,iReplyUID)
    {
    	switch (sCmd)
    	{
    		case "Manage":
    		{
    			CALLBACK_GoToManageReplies(iCallUID);
    			break;
    		}
    		
    		case "CustomReply":
    		{
    			CALLBACK_SendCustomReply(iCallUID,customReply.value);
    			break;
    		}
    
    		default: // Generic answers
    		{
    			CALLBACK_SendReply(iCallUID,iReplyUID);
    			break;
    		}
    	}
    
    	//funcActionMouseClick("Reply",null,null);
    }
    
    // END: reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply reply
    
    // START: pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup .............
    
    function pickUpCall(iCallUID)
    {
    	CALLBACK_PickupCall(iCallUID, "CustomerDisplayName");//todo
    }
    // END: pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup pickup .............
    
    
    	/*********** COMMON DISPLAY FUNCTIONS *********/	
    	
    		
    /**********
     * actionHeaderHTML : returns the first part HTML for the actions
     * @param {Object} iCallUID
     * @param {Object} sCustomerName
     */
    function actionHeaderHTML(iCallUID,sCustomerName,sCallFlagStatus)
    {
    	return('\
    	<div>\
    		<div style="border:1px solid green;margin-top:6px;">\
    			<div class="dCallDirectionType2"></div>\
    			<div class="dCustomerName2">'+sCustomerName+'</div>\
    			<div class="dCustomerFlag"><img name="Flag" id="imgFlag_'+iCallUID+'" src="'+gFlagsImgs[sCallFlagStatus]+'"></div>\
    		</div>\
    		<div style="clear:both;"></div>\
    		<div style="margin-top:5px;text-align:center;width:100%;border:1px solid green;">\
    			<div id="dCallItemActionExpand_'+iCallUID+'" onclick="displayCallsListOrPbInfoView('+iCallUID+')" class="dCallItemAction">E</div>\
    			<div id="dCallItemActionPbInfo_'+iCallUID+'" onclick="displayPbInfoView('+iCallUID+')" class="dCallItemAction">Pb</div>\
    			<div id="dCallItemActionCustInfo_'+iCallUID+'" onclick="displayCustInfoView('+iCallUID+')" class="dCallItemAction">C</div>\
    			<div id="dCallItemActionFlag_'+iCallUID+'" onclick="displayFlagsView('+iCallUID+')" class="dCallItemAction">F</div>\
    			<div id="dCallItemActionForward_'+iCallUID+'" onclick="displayForwardView('+iCallUID+')" class="dCallItemAction">Fw</div>\
    			<div id="dCallItemActionSnooze_'+iCallUID+'" onclick="displaySnoozeView('+iCallUID+')" class="dCallItemAction">S</div>\
    			<div id="dCallItemActionReply_'+iCallUID+'" onclick="displayReplyView('+iCallUID+')" class="dCallItemAction">R</div>\
    			<div id="dCallItemActionPickup_'+iCallUID+'" onclick="pickUpCall('+iCallUID+')" class="dCallItemAction">Pk</div>\
    		</div>\
    	</div>');
    }
    
    var openInboxHeight = 'auto';
    function OnExpandInboxClicked()
    {
        var oCMInboxContent = $('CMInboxContent').toggle();
        if (oCMInboxContent.visible)
            $('CMPanel1').style.height = openInboxHeight
        else
        {
            openInboxHeight = $('CMPanel1').style.height  
            $('CMPanel1').style.height = '36px'
        }
    }
    
    function OnExpandInSessionClicked()
    {
        $('CMInSessionContent').toggle();
    }

    
    
    function checkEnterAndStartLogin(e) //e is event object passed from function invocation
    {
    	if( e.keyCode == 13) //ENTER
    	{ 
    		OnLoginStartClicked();
    		return false 
    	}
    	else
    	{
    		return true 
    	}
    }
    
    function OnUpdateVersion(sData)
    {
		var oData = eval(sData);
    	var sProductVersion = oData.version;

    	CALLBACK_UpdateVersion(sProductVersion);
    }
   
    /**** todo sprint 4
    Session start:   "state":"PICKED"    "0nly from Inbox Picked to InSession"
    Session end:     "state":"ENDED"	 
    Missed call:     "state":"TIMEOUT"
    Canceled call:   "state":"CANCELED"
    ****/
    function requestStatusChangedUI(sRequest)
    {
    	var oRequest = eval(sRequest);
    	var iCallUID = oRequest.supportRequest.id;
    	var state = oRequest.state;
		var bStayOnline4Customer = oRequest.stayOnline4Customer;
		var sCustomerName = oRequest.supportRequest.customer.displayUserName;

    	deleteCallFromInbox(sRequest);
		CALLBACK_RequestStatusChanged(iCallUID,state,bStayOnline4Customer,sCustomerName);
		/*
    	if ($('dCallItem_'+iCallUID))
        {
    		Element.remove('dCallItem_'+iCallUID);
    		CALLBACK_RequestStatusChanged(iCallUID,state);
    	}
    	else
    	{
    		//todo Sprint 4. Temporary if call is not in Inbox UID is 0 - temporary 
    		CALLBACK_RequestStatusChanged(0,state);
    	}
	    //CMInboxNumber.innerHTML = gCallItemsList.length;
		*/
    }
    
    function deleteCallsFromInbox(sCalls)
    {
    	var aCalls = eval(sCalls);
                
    		for (var i=0;i<aCalls.length;i++)
    		{
    			for (var j=0;j<gCallItemsList.length;j++)
    			{
    				if (aCalls.supportRequest.id == gCallItemsList[j].iCallUID)
                           gCallItemsList.splice(j,1)
    			}				
    			deleteCallUI(aCalls.supportRequest.id);
    		}
    }
    function deleteCallFromInbox(sCall)
    {
		var oCall = eval(sCall)
		for (var j=0;j<gCallItemsList.length;j++)
		{
			if (oCall.supportRequest.id == gCallItemsList[j].iCallUID)
				   gCallItemsList.splice(j,1)
		}
		CALLBACK_AddDhtmlLog("deleteCallFromInbox(). gCallItemsList.length=" + gCallItemsList.length, "MESSAGE");	
			
		//deleteCallUI(oCall.supportRequest.id);		
	}
    
    /***
     * Open the menu to choose the messenger statuses
     */
    function openMenuStatuses()
    {
        event.cancelBubble = true
        $('CMSupporterStatusMenu').toggle();
    }
    function closeMenuStatuses()
    {
        $('CMSupporterStatusMenu').hide();
    }
    
    function changeStatus(stus)
    {
        alert('status changed to '+stus)
    }
    


/******************** SETTINGS FUNCTIONS *****************************/

	 function setSettingsUI(sData)
    {
		var oSettings = eval(sData);
			
		$("cAutomaticallyRun").checked = oSettings[0].cAutomaticallyRun;

		$("cPortOpened").checked = oSettings[0].cPortOpened;
        if (oSettings[0].sPortOpened !== undefined)
		    $("sPortOpened").value = oSettings[0].sPortOpened;

		$("cScreenSaverIsOn").checked = oSettings[0].cScreenSaverIsOn;

    	//$("cOpenMainWindowOnWindowsStartUp").checked = cOpenMainWindowOnWindowsStartUp ; 
    	$("cShowAway").checked = oSettings[0].cShowAway; 
		$("sShowAway").value = oSettings[0].sShowAway; 
    	//$("cHandleCallsDisplayBusy").checked = cHandleCallsDisplayBusy; 
    	//$("sHandleCallsDisplayBusy").value = sHandleCallsDisplayBusy; 
    	//$("cOnIncomingCallsShowTrayMessage").checked = cOnIncomingCallsShowTrayMessage; 
    	$("cOnIncomingCallsAnimateTrayIcon").checked = oSettings[0].cOnIncomingCallsAnimateTrayIcon; 
    	$("cPromptOnItemsOnLogout").checked = oSettings[0].cPromptOnItemsOnLogout;									//TODO or both Inbox
    	//$("cPromptAboutSnoozingItemsOnLogout").checked = cPromptAboutSnoozingItemsOnLogout; 
    	$("cPlaySoundUponIncomingCall").checked =oSettings[0].cPlaySoundUponIncomingCall; 
    	$("cPlaySoundUponConnectingToCustomer").checked = oSettings[0].cPlaySoundUponConnectingToCustomer; 
    	//$("cDisplayItemsAtTime").checked = cDisplayItemsAtTime; 
    	//$("sDisplayItemsAtTime").value = sDisplayItemsAtTime; 	
        $('sDefaultPortOpened').innerHTML = oSettings[0].sDefaultPortOpened;

    	switchPanel("CMSettingsPanel");
        setConnectionTestStateUI('init','Test your TCP port settings')
        setConnectionPortStateUI();
    }
    
    function SettingsOk()
    {
    	var 	cOnIncomingCallsShowTrayMessage	= true;				// not for beta
    	var		cOpenMainWindowOnWindowsStartUp	= true;				// not for beta ??? why not for beta, what defaul not for beta?
    	var		cOnIncomingCallsShowTrayMessage = true;				// not for beta
    	var		cPromptOnItemsUpdate			= true;				// not for beta
    	var		cPromptAboutSnoozingItemsOnLogout = true;			// not for beta
        var     sDisplayItemsAtTime = '100';                        // not for beta
        var     cHandleCallsDisplayBusy = false;                    // not for beta

    	CALLBACK_SettingsOk(
    			cOnIncomingCallsShowTrayMessage,
    			$('cAutomaticallyRun').checked,
    			cOpenMainWindowOnWindowsStartUp,
    			$('cShowAway').checked,
    			$('sShowAway').value,
    			cHandleCallsDisplayBusy,
    			'',
    			cOnIncomingCallsShowTrayMessage,
    			$('cOnIncomingCallsAnimateTrayIcon').checked,
    			$('cPromptOnItemsOnLogout').checked,
    			cPromptAboutSnoozingItemsOnLogout,
    			$('cPlaySoundUponIncomingCall').checked,
    			$('cPlaySoundUponConnectingToCustomer').checked,
    			sDisplayItemsAtTime,
				$("cPortOpened").checked,
				$("sPortOpened").value,
				$("cScreenSaverIsOn").checked
    		);
    
        //SettingsClose();
    }
    
    function SettingsClose()
    {
        //showLastPanel()
        CALLBACK_CloseMessenger();
    }
    
    function setConnectionTestStateUI(state,tltptext)
    {
		var undefined;
        var oTestImg = $('CMSettingsConnectionTest').down('img');
        var oTestText =  $('CMSettingsConnectionTest').down('span');
        if (!oTestImg || oTestImg.tagName != 'IMG')
            return;

		document.body.style.cursor = 'default';
            
		if (state==undefined) state='';
		if (tltptext==undefined) tltptext='';
        oTestText.className = state;
		oTestText.title = tltptext;
		
        switch (state)
        {
            case 'init':
                oTestImg.src = 'img/test_test_icon.gif';
                oTestText.innerHTML = 'Test'
            break;
			case 'testing':
				oTestImg.src = 'img/test_test_icon.gif';
                oTestText.innerHTML = 'Testing...'
				document.body.style.cursor = 'progress';
				setTimeout(CALLBACK_SettingsTest,1000);
			break;
            case 'failed':
                oTestImg.src = 'img/test_fail_icon.gif';
                oTestText.innerHTML = 'Test Again'                
            break;
            case 'success':
                oTestImg.src = 'img/test_ok_icon.gif';
                oTestText.innerHTML = 'Test Succeed'
            break;
            case 'configure':
				oTestImg.src = 'img/test_test_icon.gif';
                if ($('cPortAutoconfigure').checked)
                    oTestText.innerHTML = 'Test & Configure'
                else
                    oTestText.innerHTML = 'Test'
            break;
        }
    }
    
    function setConnectionPortStateUI()
    {
        var elt = $('cPortOpened');
        $('cPortAutoconfigure').up().disabled = !elt.checked;
        $('cPortAutoconfigure').up().checked = false; 
    }

	function CheckParamShowAway(val) 
	{
		IsParamNum(val);

		if(val.value!="")
		{
			var iVal = parseInt(val.value);

			if(iVal > 60)
			{
				val.value = "10";
				CALLBACK_ShowMessageBox("You may appear offline after 1-60 minutes of being idle. Please select a number within this range");
			}
		}
	}

	function CheckParamPortOpened(val) 
	{
		IsParamNum(val);
	}

	function IsParamNum(val) 
	{
		if(val.value=="")
			return;
		
		var strPass = val.value;
		var strLength = strPass.length;
		var lchar = val.value.charAt((strLength) - 1);

		while( IsN(val.value)==false )
		{
			var tst = val.value.substring(0, (strLength) - 1);
			val.value = tst;
			strLength = val.value.length;
		}
	}

	function IsN(St) 
	{ 
		return !/\D/.test(St) 
	}