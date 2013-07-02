function CallItem(oCall) {
	if (oCall)
		this.copyCall(oCall);
	else
	{
		this.iCallUID = -1;                          // unique ID of call

		this.resumeSession = false;	                        // Session resume flag
		this.iCallDirection = -1;                    // the call direction: 1=Direct, 2=Indirect, 3=Forwarded, 4=Viewer
		this.iCallType = -1;                         // the type of the call: 1=Scheduled, 2=House call, 3=Watchdog, 4=Chat, 5=Phone, 6=SMS, 7=Message

		this.sCallArrivalDate = '01/01/2007';	    // the date of the arrival of the call in the format DD/MM/YYYY
		this.sCallArrivalTime = '00:00:00';	        // the time of the arrival of the call in the format HH:MM:SS

		this.sCallTimerStartDate = '01/01/2007';	// the date when timer was started DD/MM/YYYY - "" if none
		this.sCallTimerStartTime = '00:00:00';	    // the time timer was started HH:MM:SS - "" if none
		this.iCallTimerSecondes = 0;	            // total number of seconds timer should count - 0 if none
			
		this.sCallCategoryPath = '';                // the path of the category of the problem - "" if none

		this.sCallProblemDescription = '';	        // the description of the problem - "" if none

		this.iLastActionTaken = -1;	                // last action taken by supporter: 1=Replied, 2=Forwarded, 3=Snoozed, 4=Picked, 5=Viewed
		this.iLastActionTakenParam = -1;            // extra param of last action: 1=Replied->Reply String UID, 2=Forwarded->Entity UID, 3=Snoozed->number of Minutes, 4=Picked->None, 5=Viewed->None

		this.sCallFlagStatus = '';	                // status of the flag: "Red", "Blue", "Orange"... or "Completed" or "Cleared" - default is "Cleared"
	
		this.oCustomer = new Customer();            // customer information
	}
}

CallItem.prototype = new Object;

CallItem.prototype.copyCall = function(oCall) 
{
		this.iCallUID = oCall.iCallUID;	                                // unique ID of call

		this.resumeSession = oCall.resumeSession;	                        // Session resume flag

		this.iCallDirection = oCall.iCallDirection;	                    // the call direction: 1=Direct, 2=Indirect, 3=Forwarded, 4=Viewer
		this.iCallType = oCall.iCallType;	                            // the type of the call: 1=Scheduled, 2=House call, 3=Watchdog, 4=Chat, 5=Phone, 6=SMS, 7=Message

		this.sCallArrivalDate = oCall.sCallArrivalDate;	                // the date of the arrival of the call in the format DD/MM/YYYY
		this.sCallArrivalTime = oCall.sCallArrivalTime;	                // the time of the arrival of the call in the format HH:MM:SS

		this.sCallTimerStartDate = oCall.sCallTimerStartDate;	        // the date when timer was started DD/MM/YYYY - "" if none
		this.sCallTimerStartTime = oCall.sCallTimerStartTime;           // the time timer was started HH:MM:SS - "" if none
		this.iCallTimerSecondes  = oCall.iCallTimerSecondes;	        // total number of seconds timer should count - 0 if none
			
		this.sCallCategoryPath = oCall.sCallCategoryPath;               // the path of the category of the problem - "" if none

		this.sCallProblemDescription = oCall.sCallProblemDescription;   // the description of the problem - "" if none

		this.iLastActionTaken = oCall.iLastActionTaken;	                // last action taken by supporter: 1=Replied, 2=Forwarded, 3=Snoozed, 4=Picked, 5=Viewed
		this.iLastActionTakenParam = oCall.iLastActionTakenParam;       // extra param of last action: 1=Replied->Reply String UID, 2=Forwarded->Entity UID, 3=Snoozed->number of Minutes, 4=Picked->None, 5=Viewed->None

		this.sCallFlagStatus = oCall.sCallFlagStatus;	                // status of the flag: "Red", "Blue", "Orange"... or "Completed" or "Cleared" - default is "Cleared"
	
		this.oCustomer = new Customer();                                // customer information
		this.oCustomer.copyCustomer(oCall.oCustomer);
}

CallItem.prototype.getCall = function(jsonCall) 
{
		this.iCallUID = jsonCall.id;	                                // unique ID of call

		this.iCallDirection = jsonCall.supportRequestSubmissionMode;	// the call direction: 1=Direct, 2=Indirect, 3=Forwarded, 4=Viewer
		this.iCallType = 4;	                                            // Not implemented in Sprint1 - the type of the call: 1=Scheduled, 2=House call, 3=Watchdog, 4=Chat, 5=Phone, 6=SMS, 7=Message

		this.resumeSession = jsonCall.resumeSession;
        this.oSubmissionDate  = new S2Date();
        this.oSubmissionDate.getDate(jsonCall.submissionDate);
		this.sCallArrivalDate = this.oSubmissionDate.getDateStr();	    // the date of the arrival of the call in the format DD/MM/YYYY
		this.sCallArrivalTime = this.oSubmissionDate.getTimeStr();	    // the time of the arrival of the call in the format HH:MM:SS

		this.sCallTimerStartDate = 0;                                   // Not implemented in Sprint1 - the date when timer was started DD/MM/YYYY - "" if none
		this.sCallTimerStartTime = 0;                                   // Not implemented in Sprint1 - the time timer was started HH:MM:SS - "" if none
		this.iCallTimerSecondes  = 0;                                   // Not implemented in Sprint1 - total number of seconds timer should count - 0 if none
			
		this.sCallCategoryPath = "";	                    // Not implemented in Sprint1 - the path of the category of the problem - "" if none

		this.sCallProblemDescription = jsonCall.problemDescription;	// the description of the problem - "" if none

		this.iLastActionTaken = 2;    	                                // Not implemented in Sprint1 - last action taken by supporter: 1=Replied, 2=Forwarded, 3=Snoozed, 4=Picked, 5=Viewed
		this.iLastActionTakenParam = 10;	                            // Not implemented in Sprint1 - extra param of last action: 1=Replied->Reply String UID, 2=Forwarded->Entity UID, 3=Snoozed->number of Minutes, 4=Picked->None, 5=Viewed->None

		this.sCallFlagStatus = "Red";	                                // Not implemented in Sprint1 - status of the flag: "Red", "Blue", "Orange"... or "Completed" or "Cleared" - default is "Cleared"
	
		this.oCustomer = new Customer();					            // customer information
		this.oCustomer.getCustomer(jsonCall.customer);
}

CallItem.prototype.isEmpty = function()
{
    return (this.iCallUID == -1)
}

//Static functions
CallItem.getCallItemById = function(iCallUID)
{
	if (!gCallItemsList) return null;
	
	for (var i=0 ; i<gCallItemsList.length; i++ )
	{
		if (gCallItemsList[i].iCallUID == iCallUID)
			return gCallItemsList[i];			
	}
	return null;
}
