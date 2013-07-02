/**
 * @author noame
 */

function S2Date()
{
	this.month = 0;
	this.day = 0;
	this.year = 0;
	this.date = 0;
	this.hours = 0;
	this.minutes = 0;
	this.seconds = 0;	
	this.timezoneOffset = 0;
	this.time = 0;	
}

S2Date.prototype = new Object;

S2Date.prototype.getDate = function(jsonDate)
{
	this.month = jsonDate.month;
	this.day = jsonDate.day;
	this.year = jsonDate.year;
	this.date = jsonDate.date;
	this.hours = jsonDate.hours;
	this.minutes = jsonDate.minutes;
	this.seconds = jsonDate.seconds;	
	this.timezoneOffset = jsonDate.timezoneOffset;
	this.time = jsonDate.time;
}

S2Date.prototype.getDateStr = function()
{
	return this.month+'/'+this.day+'/'+this.year;
}

S2Date.prototype.getTimeStr = function()
{
	return this.month+'/'+this.day+'/'+this.year;
}


