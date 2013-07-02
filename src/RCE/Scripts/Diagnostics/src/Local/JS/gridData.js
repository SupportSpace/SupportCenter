function GridData() {
}

GridData.prototype = new Object;

GridData.prototype.copyGridData = function(oGridData) 
{
		this.rows = oGridData.rows;
		this.columns = oGridData.columns;
		this.categoryName = oGridData.categoryName;
		this.background = oGridData.background;
		this.nosort = oGridData.nosort;
}