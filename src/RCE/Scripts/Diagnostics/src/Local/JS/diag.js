var grid;
var theStore;
var theRetrieveButton;

var g_ItemIdToShow = null; // Item to show now
var g_ItemIdLastSelected = "SystemSummary"; //Item last selected; 
var g_ItemIdRequestedFromRemote = null; //Item Retrieving From Remote
var g_ItemIdWaitTobeRequestedFromRemote = null; //Item Retrieving From Remote
var g_OSystem = null;//OS system retrieved from Customer's machine
var g_UnSupportedOSLanguage = null;	//if customer's OS system language is unsupported this will be not null

/******
*		get_grid_date() return get_grid_date
*/ 
function get_grid_date()
{
	var itemToShow = theStore._getItemByIdentity(g_ItemIdToShow); 
	return itemToShow.data[0];
}

/******
*		show_table() in the grid. specific case for SystemSummary only
*/ 
function show_table()
{
	if(g_ItemIdToShow=="SystemSummary")
	{
		updateSystemSummary(get_grid_date().rows);
		showInfoPaneSystemSummary();
	}
	else
	{
		updateGridTable();
	}
}

/******
*		RetrieveClicked() always retrieve currently selected item from Remote machine
*/ 
function RetrieveButtonClicked(){

	var item = theStore._getItemByIdentity(g_ItemIdLastSelected); 
	RetrieveFromRemote(item);
}

/******
*		RetrieveClicked() may send request via Script Engine and chamhe application state to WaitingSEHostResponse
*/ 
function RetrieveFromRemote(mySelectedItem){

	if(mySelectedItem==null)
	{
		if(g_bWaitingForCustomerAproval==false)
			UpdateStatusBar(gStatusBarMessage.SelectSubcategory, "");

		return;
	}

	// 1 case: nothing to do with non SubCcategory
	var nonSubCcategory = mySelectedItem ? treeTestStore.getValue(mySelectedItem, "nonSubCcategory") : "";
	if(nonSubCcategory== "1")
	{
		showInfoPaneStatus();
		if(g_bWaitingForCustomerAproval==false)
		{
			updateInfoPaneStatus(gInfoPaneMessage.SelectSubcategory);
			UpdateStatusBar(gStatusBarMessage.SelectSubcategory,"");
		}
		return;
	}

	showInfoPaneStatus();

	if(g_bWaitingForCustomerAproval==false)
	{
		updateInfoPaneStatus(gInfoPaneMessage.Refreshing);
		UpdateStatusBar(gStatusBarMessage.Refreshing,"");
	}

	// 2 case: we need to put in the waiting list what about critical sections in js?
	if(g_ItemIdRequestedFromRemote!=null)
	{
		g_ItemIdWaitTobeRequestedFromRemote = mySelectedItem ? treeTestStore.getValue(mySelectedItem, "id") : "";
		if(g_bWaitingForCustomerAproval==false)
			UpdateStatusBar(gStatusBarMessage.WarningWaitCompletion,"");
		return;
	}

	// 3 case: main case 
	var categoryId = mySelectedItem ? treeTestStore.getValue(mySelectedItem, "id") : "";
	var categoryName = mySelectedItem ? treeTestStore.getValue(mySelectedItem, "name") : "";
	g_ItemIdRequestedFromRemote = categoryId;

	// debugger;
	// http://www.experts-exchange.com/Programming/Languages/Visual_Basic/Q_20890694.html

	if(localVBS==false)
	{
		DeployAndRunScriptOnRemoteMachine(g_ItemIdRequestedFromRemote);
	}
	else
	{
//		debugger;
		testJsonTable = BuildJsonFileByDiagCategoryLocal(g_ItemIdRequestedFromRemote, g_bRunFirstTime);
		OnRemoteDataRetrieved(g_ItemIdRequestedFromRemote, testJsonTable ) //todo to work localy
	}
}

function OnRemoteDataRetrieved(id, testTable)
{

	testJsonTable = eval('(' + testTable + ')');

	g_ItemIdRequestedFromRemote = null;

	//
	//	prepeare retrieved data to store
	//
	var view = {
		noscroll: false,
		rows: [ testJsonTable.Table.columns ]
	};

	var itemToShow = theStore._getItemByIdentity(testJsonTable.Table.id);

	var gridData = new GridData;

	gridData.rows = testJsonTable.Table.rows;
	gridData.columns = view;
	gridData.background = itemToShow ? treeTestStore.getValue(itemToShow, "background") : "";
	gridData.nosort = itemToShow ? treeTestStore.getValue(itemToShow, "nosort") : "";
	gridData.categoryName = itemToShow ? treeTestStore.getValue(itemToShow, "name") : "";

	theStore.setValue(itemToShow, "data", gridData);
	

	// OnRemoteDataRetrieved(testJsonTable.Table.id);
	
	
	UpdateStatusBar(gStatusBarMessage.RequestesdDataRetrieved, ":" + gridData.categoryName);

	//
	//	compare with retrieved ID and decide if to show 
	//
	if(id != g_ItemIdLastSelected) 
	{
		UpdateStatusBar( gStatusBarMessage.DataRequestedBeforeRetrieved, ":" + gridData.categoryName);

		if(g_ItemIdWaitTobeRequestedFromRemote!=null)
		{
			var item = theStore._getItemByIdentity(g_ItemIdWaitTobeRequestedFromRemote); 
			var categoryName = item ? treeTestStore.getValue(item, "name") : "";

			UpdateStatusBar(gStatusBarMessage.StartedRetrievingFromWaitingList, ":" + categoryName);

			RetrieveFromRemote(item);
		}
		else
		{
			UpdateStatusBar(gStatusBarMessage.DataRequestedBeforeRetrieved, "");
		}

		return;
	}

	g_ItemIdToShow = id; // Item to show now
	show_table();
}

/******
*		RetrieveFromLocal() may check in the tree if data already stored in the data node
*/ 
function RetrieveFromLocal(mySelectedItem){
	//
	//	actually not possible situation 
	//
	if(mySelectedItem==null)
	{
		if(g_bWaitingForCustomerAproval==false)
			UpdateStatusBar(gStatusBarMessage.SelectSubcategory, "");
	
		return true;
	}

	//	
	//	check if there is no slected node is not stored in the Tree node and return true
	//  
	if(mySelectedItem.data!= "undefined" && mySelectedItem.data!=null && mySelectedItem.data[0]!="")
	{
		g_ItemIdToShow = mySelectedItem.id; 
		g_ItemIdWaitTobeRequestedFromRemote = null; //do not need to retrieve something that was in waiting list

		UpdateStatusBar(gStatusBarMessage.LocalDataRetrieved, "");
		show_table();
		return true;
	}

	return false;
}

/******
*		onClick() is called when customer selected a node in the tree including subcategory
*/ 
function onClick(item){

	if(item==null)
		return;

	item.activity = "INACTIVE";

	g_ItemIdLastSelected = item.id; //Item last selected; 

	if( RetrieveFromLocal(item)==false )
	{
		RetrieveFromRemote(item);
	}
	// dndController="dijit._tree.dndSource"
	// Attribute has 2 problems root null is null
	// draging of node strange See also //@import "dojo/tests/dnd/dndDefault.css"; 
	// http://www.alexatnet.com/node/96
	// http://dojotoolkit.org/book/dojo-book-0-9/part-2-dijit/advanced-editing-and-display/tree
	// http://dojotoolkit.org/forum/dijit-dijit-0-9/dijit-development-discussion/tree-and-selection 
}

/******
*		createGrid() may be called once to create instance of Grid
*/ 
function createGrid() {

	var data = [];
	var layout = [];

	model = new dojox.grid.data.Table(null, data);

	grid = new dojox.Grid({
		"id": "grid",
		"model": model,
		"structure": layout,
		canSort:disableSelect
		//onCellClick: dojo.stopEvent, //need for selection and copy/paste
		//onStyleRow: onMyStleRow,
		//onHeaderClick: setInputsSelectable, //need for selection and copy/paste
		//onHeaderMouseOut : setInputsSelectable,
		//onHeaderCellMouseOver : setInputsSelectable
		//onHeaderCellClick : setInputsSelectable,
	});

	dojo.byId("gridContainer").appendChild(grid.domNode);
	//grid.render();
	//dojo.connect(dijit.byId('grid'),'update',setGridSelectable);//need for selection and copy/paste
	//dojo.connect(dijit.byId('grid'),'sort',setGridSelectable);//need for selection and copy/paste
}

/******
*		onMyStleRow() not in use just to keep in mind option 
*/ 
function onMyStleRow()
{
	return;
}

/******
*		updateGridTable() updated already created table using createGrid
*/ 
function updateGridTable()
{
	// must be called firts for some Dojo reason 
	showInfoPaneGridTable();

	var gridData = get_grid_date();

	// for each category show its background
	var backImage = gridData.background;
	changeBGImage(backImage);

	var layout2 = [ gridData.columns ];
	grid.setStructure(layout2);

	var data2 = gridData.rows;
	var model2 = new dojox.grid.data.Table(null,data2);

	dojo.connect(dijit.byId('grid'),'update',setGridSelectable);//need for selection and copy/paste

	grid.setModel(model2);
	grid.refresh();
	grid.update();

	$('textCategoryName').innerHTML = gridData.categoryName;

	// comments to be deleted later todo
	// dojo.connect(dijit.byId('grid'),'sort',setGridSelectable);//need for selection and copy/paste
	// add something like this in the structure/layout parameters of the last cell 
	// (or possibly in any/all cell(s) you don't mind auto-sizing):// ,width:'auto',noresize:false
	// {name: 'Column 8', width:'auto', noresize:'false' }
	// also see possible sizes in test_sizing.html and to adopt coumns to container see 
	// vbscript sJsonObjColumns = sJsonObjColumns & "{" & "name:" & "'" & Child.nodeName & "'" & ", width:'auto', noresize:false" & "}"
	// grid.autoWidth = true;
	// grid.autoHeight = true;
	// dojo.connect(dijit.byId('grid'),'canSort', myCanSort);//need for selection and copy/paste
	// dojo.connect(dijit.byId('grid'),'onHeaderCellClick', blockSort);//need for selection and copy/paste
	// http://www.sitepen.com/blog/2007/11/13/dojo-grids-diving-deeper/
	// Includes custom sort...
	// Also, working example in tests:
	// http://archive.dojotoolkit.org/nightly/dojotoolkit/dojox/grid/tests/test...
	// dijit.byId("grid").setSortIndex(-1); how to disable sorting 
	// http://www.dojotoolkit.org/forum/dojox-dojox/dojox-support/grid-inputs-mouseover-and-click#comment-9069
	// http://www.dojotoolkit.org/forum/dojox-dojox/dojox-support/grid-inputs-mouseover-and-click#comment-9895
}

/******
*		disableSelect() may check in the tree if data already stored in the data node
*/ 
function disableSelect(index) {
	var gridData = get_grid_date();
	if(gridData.nosort == "1")
		return false;
	else
		return true;
	//  example to do more	
	//  index = Math.abs(index); 
	//  if(index == 3) return false;  //where "3" is the 1-based column for which you wish to disable sorting.
	//		else return true;
}

/******
*		setGridSelectable() must be called to workaround making selectable rows and columns
*      dojo.connect(dijit.byId('grid'),'update',setGridSelectable);//need for selection and copy/paste
*/ 
function setGridSelectable()
{
	dojo.setSelectable(dijit.byId('grid').domNode,true);
}

/******
*		
*      showInfoPaneGridTable switch view of info pane to show Grid
*/
function showInfoPaneGridTable()
{
	$('infoPaneStatus').hide();
	$('systemSummaryDiv').hide();

	$('gridContainer').show(); 
	$('textCategoryName').show(); 
}

/******
*		
*      showInfoPaneSystemSummary switch view of info pane to show SystemSummary - not like Grid
*/
function showInfoPaneSystemSummary()
{ 
	$('gridContainer').hide(); 
	$('textCategoryName').hide(); 
	$('infoPaneStatus').hide();

	$('systemSummaryDiv').show();
}

/******
*		
*      showInfoPaneStatus switch view of info pane to show Status message 
*/
function showInfoPaneStatus()
{ 
	$('gridContainer').hide(); 
	$('textCategoryName').hide(); 
	$('systemSummaryDiv').hide();

	$('infoPaneStatus').show();
}

/******
*		
*      updateInfoPaneStatus 
*/
function updateInfoPaneStatus(newText)
{
	$('infoPaneStatus').innerHTML = newText;
}

function updateSystemSummary(jsonData)
{
	g_OSystem = jsonData.OSystem;
	g_UnSupportedOSLanguage = jsonData.UnSupportedOSLanguage;

	if(g_UnSupportedOSLanguage!=null && g_UnSupportedOSLanguage!="")
		UpdateStatusBar(gStatusBarMessage.RequestesdDataRetrievedOSLanguageWarning, "" + g_UnSupportedOSLanguage );

	//  getBG(screen.width)
	changeBGImage(get_grid_date().background);
	
	document.getElementById('sOSName').innerHTML = jsonData.OSName;	
	document.getElementById('sOSVersion').innerHTML = jsonData.Version;
	document.getElementById('sOSManufactor').innerHTML = jsonData.OSManufacturer;
	document.getElementById('sDirectory').innerHTML = jsonData.WindowsDirectory; //todo to check
	document.getElementById('sOSDirectory').innerHTML = jsonData.SystemDirectory;
	document.getElementById('sHWname').innerHTML = jsonData.SystemName;
	document.getElementById('sHWManufactor').innerHTML = jsonData.SystemManufacturer;
	document.getElementById('sHWModel').innerHTML = jsonData.SystemModel;
	document.getElementById('sHWType').innerHTML = jsonData.SystemType;
	document.getElementById('sHWProcessor').innerHTML = jsonData.Processor;
	document.getElementById('sHWBiosVerDate').innerHTML = jsonData.BIOSVersionDate; //todo
	document.getElementById('sHWSMBiosVerDate').innerHTML = jsonData.SMBIOSVersion;
	document.getElementById('sHWBootDevice').innerHTML = jsonData.BootDevice;

	document.getElementById('sHWAbstactionLayer').innerHTML = jsonData.HardwareAbstractionLayer;
	document.getElementById('sHWUsername').innerHTML = jsonData.UserName;

	document.getElementById('sMemTotalPhysicalMemory').innerHTML = jsonData.TotalPhysicalMemory;
	document.getElementById('sMemAvailiblePhMemory').innerHTML = jsonData.AvailablePhysicalMemory;
	document.getElementById('sMemTotalVirtualMemory').innerHTML = jsonData.TotalVirtualMemory;
	document.getElementById('sMemAvailableVirtualMemory').innerHTML = jsonData.AvailableVirtualMemory;

	document.getElementById('sMemPageFile').innerHTML = jsonData.PageFile;
	document.getElementById('sMemPageFileSpace').innerHTML = jsonData.PageFileSpace;
		
	document.getElementById('sOtherTimeZone').innerHTML = jsonData.TimeZone;
	document.getElementById('sOtherLocale').innerHTML = jsonData.Locale;
	document.getElementById('sOSLanguage').innerHTML = jsonData.OSLanguage;
}

function changeBGImage(whichImage){
	var elt = $('grid') && $('gridContainer').visible() ? $('grid') : $('bottomRight');
	elt.setStyle({background:'url('+whichImage+') transparent',backgroundRepeat:'no-repeat',backgroundPosition:'bottom right'});
/*
	if ($('gridContainer')){
		$('gridContainer').style.backgroundImage.src = whichImage;
	}
*/
//	$('gridContainer').setStyle({ backgroundColor: url('/images/SysInfo.JPG')});
	//var divTest = $('gridContainer');
	//debugger;
	//divTest.setStyle({ backgroundColor: url('/images/SysInfo.JPG')});
	//divTest.style.backgroundImage =  "images/SysInfo.JPG";
	//document.getElementById('gridContainer').style.backgroundImage = url('images/system_info.gif'); 
	//document.getElementById('grid').style.backgroundImage="url('images/system_info.gif')";
}

function deleteItem(){
	var store = dijit.byId("myTree").store;
	store.deleteItem(selectedItem);
	resetForms();
}

function addItem(){
	var store = dijit.byId("myTree").store;
	var pInfo = selectedItem ? {parent: selectedItem, attribute:"children"} : null;
	console.debug(pInfo);
	store.newItem({id: dojo.byId('newId').value,name:dojo.byId("label").value,someProperty:dojo.byId("someProperty").value},pInfo);
	resetForms();
}

function updateItem(){
	console.log("Updating Item");
	var store = dijit.byId("myTree").store;

	if (selectedItem!=null){
		if (dojo.byId("uLabel").value != store.getValue(selectedItem, "name")){
			store.setValue(selectedItem, "name", dojo.byId("uLabel").value);
		}

	if (dojo.byId("uSomeProperty").value != store.getValue(selectedItem, "someProperty")){
		store.setValue(selectedItem, "someProperty", dojo.byId("uSomeProperty").value);
	}
	}else{
		console.error("Can't update the tree root");
	}
}

/******
*		getIcon() 
*/ 
function getIcon(item) {
		return "noteIcon";
		/*
			if(item == null)
				return "noteIcon";

			if(item.activity == null)
				return "noteIcon";

			alert("getIcon");

			if(item.activity == "INACTIVE")
				return "uncheckedIcon";
			else 
				return "noteIcon";
		*/	
}

/******
*		callback will call this mehtod when init failed like Declined
*/ 
function OnRemoteInitFailed()
{
	g_ItemIdRequestedFromRemote = null; //Item Retrieving From Remote
	g_ItemIdWaitTobeRequestedFromRemote = null; //Item Retrieving From Remote
}

/******
*		
*      UpdateStatusBar(newText, color, img) 
*
*	   params:
*	   newText	text to be shown
*	   color    color to be shown
*      img		image to be shown
*/
function UpdateStatusBar(event_msg, opt_param)
{
	if(event_msg.text == undefined)
	{
		return;
	}

	switch(event_msg.color)
	{
		case "orange":
			$('bottomBarText').style.color = "#FE572C"; // orange
			break;
		case "blue":
		    $('bottomBarText').style.color = "#1488D3"; // blue 
			break;
		case "green":
			$('bottomBarText').style.color = "#21A909"; // green
			break;
		case "gray":
			$('bottomBarText').style.color = "#717171";// gray
			break;
		default:
		    $('bottomBarText').style.color = "#1488D3"; // default blue ?
			break;
	}
	
	if(opt_param != "" && opt_param != undefined)
		$('bottomBarText').innerHTML = event_msg.text + opt_param;
	else
		$('bottomBarText').innerHTML = event_msg.text;
	//$('bottomBarText').style.img = img;  //todo Ask PM for images
}
//
// http://srv-dev/jira/browse/STL-564
//
function disableCtrlKeyCombination(e)
{
	//list all CTRL + key combinations you want to disable
	//var forbiddenKeys = new Array('a', 'n', 'c', 'x', 'v', 'j');
	var forbiddenKeys = new Array('a');
	var key;
	var isCtrl;

	if(window.event)
	{
		key = window.event.keyCode;     //IE
		if(window.event.ctrlKey)
			isCtrl = true;
		else
			isCtrl = false;
	}
	else
	{
		key = e.which;     //firefox
		if(e.ctrlKey)
			isCtrl = true;
		else
			isCtrl = false;
	}

	//if ctrl is pressed check if other key is in forbidenKeys array
	if(isCtrl)
	{
		for(i=0; i < forbiddenKeys.length; i++)
		{
			//case-insensitive comparation
			if(forbiddenKeys[i].toLowerCase() == String.fromCharCode(key).toLowerCase())
			{
				//alert('Key combination CTRL + '  + String.fromCharCode(key) + ' has been disabled.');
				return false;
			}
		}
	}
	return true;
}


