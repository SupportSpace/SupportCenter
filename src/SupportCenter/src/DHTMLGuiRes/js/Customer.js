function Customer() {
	this.sCustomerName = '';			// the name of the user that requests the support - "" if none
	this.iCustomerUID = 0;				// UID of customer

	this.sCustomerRatingValue = '';		// x out of 10
	this.sCustomerCharacter = '';		// string full with characteristics of the user Nice, Friendly, etc.
	this.sCustomerSkills = '';			// string full with technical skills of customer
	this.sCustomerInstalledFeatures = '';	// string with installed featuers on customer's machine

	this.iCustomerTimeZone = 0;			// relative value of hours to GMT (GMT-2) etc.

	this.iCustomerVesinity = 0;			// Approx vesinity between supporter address and customer address
	this.sCustomerVesinityUnit = 'Miles';	// "KM" or "Miles"

	this.sCustomerLanguages = '';		// the languages the customer understands
}

Customer.prototype = new Object;

Customer.prototype.copyCustomer = function(oCust)  {
	this.sCustomerName = oCust.sCustomerName;				// the name of the user that requests the support - "" if none
	this.iCustomerUID = oCust.iCustomerUID;					// UID of customer

	this.sCustomerRatingValue = oCust.sCustomerRatingValue;	// x out of 10
	this.sCustomerCharacter = oCust.sCustomerCharacter;		// string full with characteristics of the user Nice, Friendly, etc.
	this.sCustomerSkills = oCust.sCustomerSkills;			// string full with technical skills of customer
	this.sCustomerInstalledFeatures = oCust.sCustomerInstalledFeatures;	// string with installed featuers on customer's machine

	this.iCustomerTimeZone = oCust.iCustomerTimeZone;		// relative value of hours to GMT (GMT-2) etc.

	this.iCustomerVesinity = oCust.iCustomerVesinity;		// Approx vesinity between supporter address and customer address
	this.sCustomerVesinityUnit = oCust.sCustomerVesinityUnit;// "KM" or "Miles"

	this.sCustomerLanguages = oCust.sCustomerLanguages;		// the languages the customer understands
}

Customer.prototype.getCustomer = function(jsonCust)  {
	this.sCustomerName = jsonCust.displayUserName;	                // the name of the user that requests the support - "" if none
	this.iCustomerUID = jsonCust.id;	                    // UID of customer

	this.sCustomerRatingValue = 5;                          // Not implemented in Sprint1 - x out of 10
	this.sCustomerCharacter = 'nice';	                    // Not implemented in Sprint1 -  string full with characteristics of the user Nice, Friendly, etc.
	this.sCustomerSkills = 'novice';	                    // Not implemented in Sprint1 -  string full with technical skills of customer
	this.sCustomerInstalledFeatures = 'rc, ft';	            // Not implemented in Sprint1 -  string with installed features on customer's machine

	this.iCustomerTimeZone = 0;	                            // Not implemented in Sprint1 -  relative value of hours to GMT (GMT-2) etc.

	this.iCustomerVesinity = 0;	                            // Not implemented in Sprint1 -  Approx vesinity between supporter address and customer address
	this.sCustomerVesinityUnit = "KM";	                    // Not implemented in Sprint1 -  "KM" or "Miles"

	this.sCustomerLanguages = "English,French,German";      // Not implemented in Sprint1 -  the languages the customer understands
}