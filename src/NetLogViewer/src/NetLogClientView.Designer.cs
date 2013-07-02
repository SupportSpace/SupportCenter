using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;


namespace NetLogViewer
{
    partial class NetLogClientView
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle4 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(NetLogClientView));
            this.mainSplitContainer = new System.Windows.Forms.SplitContainer();
            this.messagesGridView = new System.Windows.Forms.DataGridView();
            this.severityDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.addedDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.messageDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.messageObjectDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.messagesDataSet = new System.Data.DataSet();
            this.Messages = new System.Data.DataTable();
            this.SeverityColumn = new System.Data.DataColumn();
            this.AddedColumn = new System.Data.DataColumn();
            this.messageColumn = new System.Data.DataColumn();
            this.messageObjectColumn = new System.Data.DataColumn();
            this.clientToolStrip = new System.Windows.Forms.ToolStrip();
            this.attachBtn = new System.Windows.Forms.ToolStripButton();
            this.setDelayModeBtn = new System.Windows.Forms.ToolStripButton();
            this.detachBtn = new System.Windows.Forms.ToolStripButton();
            this.verbosityDoropDown = new System.Windows.Forms.ToolStripDropDownButton();
            this.verbosityNoTraceMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.verbosityTraceDebugMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.verbosityTraceStateMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.verbosityCallTraceMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.clearMessagesBtn = new System.Windows.Forms.ToolStripButton();
            this.closeTabBtn = new System.Windows.Forms.ToolStripButton();
            this.saveMessagesBtn = new System.Windows.Forms.ToolStripButton();
            this.messageListView = new System.Windows.Forms.ListView();
            this.columnFieldName = new System.Windows.Forms.ColumnHeader();
            this.columnValue = new System.Windows.Forms.ColumnHeader();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewImageColumn1 = new System.Windows.Forms.DataGridViewImageColumn();
            this.dataGridViewTextBoxColumn5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewImageColumn2 = new System.Windows.Forms.DataGridViewImageColumn();
            this.dataGridViewTextBoxColumn6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewImageColumn3 = new System.Windows.Forms.DataGridViewImageColumn();
            this.dataGridViewTextBoxColumn7 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewImageColumn4 = new System.Windows.Forms.DataGridViewImageColumn();
            this.dataGridViewTextBoxColumn8 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewImageColumn5 = new System.Windows.Forms.DataGridViewImageColumn();
            this.dataGridViewTextBoxColumn9 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn10 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn11 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.mainSplitContainer.Panel1.SuspendLayout();
            this.mainSplitContainer.Panel2.SuspendLayout();
            this.mainSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.messagesGridView)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.messagesDataSet)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.Messages)).BeginInit();
            this.clientToolStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainSplitContainer
            // 
            this.mainSplitContainer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mainSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.mainSplitContainer.Name = "mainSplitContainer";
            this.mainSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // mainSplitContainer.Panel1
            // 
            this.mainSplitContainer.Panel1.Controls.Add(this.messagesGridView);
            this.mainSplitContainer.Panel1.Controls.Add(this.clientToolStrip);
            // 
            // mainSplitContainer.Panel2
            // 
            this.mainSplitContainer.Panel2.Controls.Add(this.messageListView);
            this.mainSplitContainer.Size = new System.Drawing.Size(583, 298);
            this.mainSplitContainer.SplitterDistance = 177;
            this.mainSplitContainer.TabIndex = 0;
            // 
            // messagesGridView
            // 
            this.messagesGridView.AllowUserToAddRows = false;
            this.messagesGridView.AllowUserToDeleteRows = false;
            this.messagesGridView.AutoGenerateColumns = false;
            this.messagesGridView.ColumnHeadersBorderStyle = System.Windows.Forms.DataGridViewHeaderBorderStyle.Single;
            this.messagesGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.messagesGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.severityDataGridViewTextBoxColumn,
            this.addedDataGridViewTextBoxColumn,
            this.messageDataGridViewTextBoxColumn,
            this.messageObjectDataGridViewTextBoxColumn});
            this.messagesGridView.DataMember = "messagesTable";
            this.messagesGridView.DataSource = this.messagesDataSet;
            this.messagesGridView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.messagesGridView.Location = new System.Drawing.Point(0, 25);
            this.messagesGridView.MultiSelect = false;
            this.messagesGridView.Name = "messagesGridView";
            this.messagesGridView.ReadOnly = true;
            this.messagesGridView.RowHeadersBorderStyle = System.Windows.Forms.DataGridViewHeaderBorderStyle.Single;
            dataGridViewCellStyle4.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.messagesGridView.RowsDefaultCellStyle = dataGridViewCellStyle4;
            this.messagesGridView.RowTemplate.Height = 18;
            this.messagesGridView.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.messagesGridView.ShowEditingIcon = false;
            this.messagesGridView.Size = new System.Drawing.Size(583, 152);
            this.messagesGridView.TabIndex = 2;
            this.messagesGridView.SortCompare += new System.Windows.Forms.DataGridViewSortCompareEventHandler(this.messagesGridView_SortCompare);
            this.messagesGridView.CellPainting += new System.Windows.Forms.DataGridViewCellPaintingEventHandler(this.messagesGridView_CellPainting);
            this.messagesGridView.SelectionChanged += new System.EventHandler(this.messagesGridView_SelectionChanged);
            // 
            // severityDataGridViewTextBoxColumn
            // 
            this.severityDataGridViewTextBoxColumn.DataPropertyName = "Severity";
            this.severityDataGridViewTextBoxColumn.HeaderText = "Severity";
            this.severityDataGridViewTextBoxColumn.Name = "severityDataGridViewTextBoxColumn";
            this.severityDataGridViewTextBoxColumn.ReadOnly = true;
            this.severityDataGridViewTextBoxColumn.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.severityDataGridViewTextBoxColumn.Width = 50;
            // 
            // addedDataGridViewTextBoxColumn
            // 
            this.addedDataGridViewTextBoxColumn.DataPropertyName = "Added";
            dataGridViewCellStyle3.Format = "G";
            dataGridViewCellStyle3.NullValue = null;
            this.addedDataGridViewTextBoxColumn.DefaultCellStyle = dataGridViewCellStyle3;
            this.addedDataGridViewTextBoxColumn.HeaderText = "Added";
            this.addedDataGridViewTextBoxColumn.Name = "addedDataGridViewTextBoxColumn";
            this.addedDataGridViewTextBoxColumn.ReadOnly = true;
            this.addedDataGridViewTextBoxColumn.Width = 150;
            // 
            // messageDataGridViewTextBoxColumn
            // 
            this.messageDataGridViewTextBoxColumn.DataPropertyName = "Message";
            this.messageDataGridViewTextBoxColumn.HeaderText = "Message";
            this.messageDataGridViewTextBoxColumn.Name = "messageDataGridViewTextBoxColumn";
            this.messageDataGridViewTextBoxColumn.ReadOnly = true;
            this.messageDataGridViewTextBoxColumn.Width = 500;
            // 
            // messageObjectDataGridViewTextBoxColumn
            // 
            this.messageObjectDataGridViewTextBoxColumn.DataPropertyName = "messageObject";
            this.messageObjectDataGridViewTextBoxColumn.HeaderText = "messageObject";
            this.messageObjectDataGridViewTextBoxColumn.Name = "messageObjectDataGridViewTextBoxColumn";
            this.messageObjectDataGridViewTextBoxColumn.ReadOnly = true;
            this.messageObjectDataGridViewTextBoxColumn.Visible = false;
            // 
            // messagesDataSet
            // 
            this.messagesDataSet.DataSetName = "NewDataSet";
            this.messagesDataSet.Tables.AddRange(new System.Data.DataTable[] {
            this.Messages});
            // 
            // Messages
            // 
            this.Messages.Columns.AddRange(new System.Data.DataColumn[] {
            this.SeverityColumn,
            this.AddedColumn,
            this.messageColumn,
            this.messageObjectColumn});
            this.Messages.TableName = "messagesTable";
            // 
            // SeverityColumn
            // 
            this.SeverityColumn.ColumnName = "Severity";
            this.SeverityColumn.DataType = typeof(object);
            // 
            // AddedColumn
            // 
            this.AddedColumn.ColumnName = "Added";
            this.AddedColumn.DataType = typeof(System.DateTime);
            this.AddedColumn.DateTimeMode = System.Data.DataSetDateTime.Local;
            // 
            // messageColumn
            // 
            this.messageColumn.ColumnName = "Message";
            // 
            // messageObjectColumn
            // 
            this.messageObjectColumn.Caption = "messageObject";
            this.messageObjectColumn.ColumnName = "messageObject";
            this.messageObjectColumn.DataType = typeof(object);
            // 
            // clientToolStrip
            // 
            this.clientToolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.attachBtn,
            this.setDelayModeBtn,
            this.detachBtn,
            this.verbosityDoropDown,
            this.toolStripSeparator1,
            this.saveMessagesBtn,
            this.toolStripSeparator2,
            this.clearMessagesBtn,
            this.closeTabBtn});
            this.clientToolStrip.Location = new System.Drawing.Point(0, 0);
            this.clientToolStrip.Name = "clientToolStrip";
            this.clientToolStrip.Size = new System.Drawing.Size(583, 25);
            this.clientToolStrip.TabIndex = 1;
            this.clientToolStrip.Text = "toolStrip1";
            // 
            // attachBtn
            // 
            this.attachBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.attachBtn.Image = ((System.Drawing.Image)(resources.GetObject("attachBtn.Image")));
            this.attachBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.attachBtn.Name = "attachBtn";
            this.attachBtn.Size = new System.Drawing.Size(23, 22);
            this.attachBtn.Text = "Attach to client";
            this.attachBtn.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
            this.attachBtn.ToolTipText = "Attach to client";
            this.attachBtn.Click += new System.EventHandler(this.attachBtn_Click);
            // 
            // setDelayModeBtn
            // 
            this.setDelayModeBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.setDelayModeBtn.Image = ((System.Drawing.Image)(resources.GetObject("setDelayModeBtn.Image")));
            this.setDelayModeBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.setDelayModeBtn.Name = "setDelayModeBtn";
            this.setDelayModeBtn.Size = new System.Drawing.Size(23, 22);
            this.setDelayModeBtn.Text = "Set delay mode";
            this.setDelayModeBtn.Click += new System.EventHandler(this.setDelayModeBtn_Click);
            // 
            // detachBtn
            // 
            this.detachBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.detachBtn.Image = ((System.Drawing.Image)(resources.GetObject("detachBtn.Image")));
            this.detachBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.detachBtn.Name = "detachBtn";
            this.detachBtn.Size = new System.Drawing.Size(23, 22);
            this.detachBtn.Text = "Detach from client";
            this.detachBtn.TextImageRelation = System.Windows.Forms.TextImageRelation.TextBeforeImage;
            this.detachBtn.ToolTipText = "detach from client";
            this.detachBtn.Click += new System.EventHandler(this.detachBtn_Click);
            // 
            // verbosityDoropDown
            // 
            this.verbosityDoropDown.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.verbosityDoropDown.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.verbosityNoTraceMenuItem,
            this.verbosityTraceDebugMenuItem,
            this.verbosityTraceStateMenuItem,
            this.verbosityCallTraceMenuItem});
            this.verbosityDoropDown.Image = ((System.Drawing.Image)(resources.GetObject("verbosityDoropDown.Image")));
            this.verbosityDoropDown.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.verbosityDoropDown.Name = "verbosityDoropDown";
            this.verbosityDoropDown.Size = new System.Drawing.Size(29, 22);
            this.verbosityDoropDown.Text = "Verbosity";
            // 
            // verbosityNoTraceMenuItem
            // 
            this.verbosityNoTraceMenuItem.Name = "verbosityNoTraceMenuItem";
            this.verbosityNoTraceMenuItem.Size = new System.Drawing.Size(134, 22);
            this.verbosityNoTraceMenuItem.Text = "No trace";
            this.verbosityNoTraceMenuItem.Click += new System.EventHandler(this.verbosityNoTraceMenuItem_Click);
            // 
            // verbosityTraceDebugMenuItem
            // 
            this.verbosityTraceDebugMenuItem.Name = "verbosityTraceDebugMenuItem";
            this.verbosityTraceDebugMenuItem.Size = new System.Drawing.Size(134, 22);
            this.verbosityTraceDebugMenuItem.Text = "Trace debug";
            this.verbosityTraceDebugMenuItem.Click += new System.EventHandler(this.verbosityTraceDebugMenuItem_Click);
            // 
            // verbosityTraceStateMenuItem
            // 
            this.verbosityTraceStateMenuItem.Name = "verbosityTraceStateMenuItem";
            this.verbosityTraceStateMenuItem.Size = new System.Drawing.Size(134, 22);
            this.verbosityTraceStateMenuItem.Text = "Trace states";
            this.verbosityTraceStateMenuItem.Click += new System.EventHandler(this.verbosityTraceStateMenuItem_Click);
            // 
            // verbosityCallTraceMenuItem
            // 
            this.verbosityCallTraceMenuItem.Name = "verbosityCallTraceMenuItem";
            this.verbosityCallTraceMenuItem.Size = new System.Drawing.Size(134, 22);
            this.verbosityCallTraceMenuItem.Text = "Call trace";
            this.verbosityCallTraceMenuItem.Click += new System.EventHandler(this.verbosityCallTraceMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // clearMessagesBtn
            // 
            this.clearMessagesBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.clearMessagesBtn.Image = ((System.Drawing.Image)(resources.GetObject("clearMessagesBtn.Image")));
            this.clearMessagesBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.clearMessagesBtn.Name = "clearMessagesBtn";
            this.clearMessagesBtn.Size = new System.Drawing.Size(23, 22);
            this.clearMessagesBtn.Text = "Clear messages";
            this.clearMessagesBtn.ToolTipText = "Clears all messages";
            this.clearMessagesBtn.Click += new System.EventHandler(this.clearMessagesBtn_Click);
            // 
            // closeTabBtn
            // 
            this.closeTabBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.closeTabBtn.Image = ((System.Drawing.Image)(resources.GetObject("closeTabBtn.Image")));
            this.closeTabBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.closeTabBtn.Name = "closeTabBtn";
            this.closeTabBtn.Size = new System.Drawing.Size(23, 22);
            this.closeTabBtn.Text = "close tab";
            this.closeTabBtn.TextImageRelation = System.Windows.Forms.TextImageRelation.TextBeforeImage;
            this.closeTabBtn.Click += new System.EventHandler(this.closeTabBtn_Click);
            // 
            // saveMessagesBtn
            // 
            this.saveMessagesBtn.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.saveMessagesBtn.Image = ((System.Drawing.Image)(resources.GetObject("saveMessagesBtn.Image")));
            this.saveMessagesBtn.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.saveMessagesBtn.Name = "saveMessagesBtn";
            this.saveMessagesBtn.Size = new System.Drawing.Size(23, 22);
            this.saveMessagesBtn.Text = "Save log messages";
            this.saveMessagesBtn.Click += new System.EventHandler(this.saveMessagesBtn_Click);
            // 
            // messageListView
            // 
            this.messageListView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.messageListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnFieldName,
            this.columnValue});
            this.messageListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.messageListView.Location = new System.Drawing.Point(0, 0);
            this.messageListView.Name = "messageListView";
            this.messageListView.Size = new System.Drawing.Size(583, 117);
            this.messageListView.TabIndex = 0;
            this.messageListView.UseCompatibleStateImageBehavior = false;
            this.messageListView.View = System.Windows.Forms.View.Details;
            // 
            // columnFieldName
            // 
            this.columnFieldName.Text = "Field";
            this.columnFieldName.Width = 127;
            // 
            // columnValue
            // 
            this.columnValue.Text = "Value";
            this.columnValue.Width = 361;
            // 
            // statusStrip
            // 
            this.statusStrip.Location = new System.Drawing.Point(0, 301);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.ManagerRenderMode;
            this.statusStrip.Size = new System.Drawing.Size(583, 22);
            this.statusStrip.TabIndex = 1;
            this.statusStrip.Text = "statusStrip1";
            // 
            // dataGridViewTextBoxColumn1
            // 
            this.dataGridViewTextBoxColumn1.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn1.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            this.dataGridViewTextBoxColumn1.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewTextBoxColumn1.Visible = false;
            // 
            // dataGridViewTextBoxColumn2
            // 
            this.dataGridViewTextBoxColumn2.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn2.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
            this.dataGridViewTextBoxColumn2.ReadOnly = true;
            this.dataGridViewTextBoxColumn2.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewTextBoxColumn2.Visible = false;
            // 
            // dataGridViewTextBoxColumn3
            // 
            this.dataGridViewTextBoxColumn3.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn3.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            this.dataGridViewTextBoxColumn3.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewTextBoxColumn3.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.dataGridViewTextBoxColumn3.Visible = false;
            // 
            // dataGridViewTextBoxColumn4
            // 
            this.dataGridViewTextBoxColumn4.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn4.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            this.dataGridViewTextBoxColumn4.ReadOnly = true;
            this.dataGridViewTextBoxColumn4.Visible = false;
            // 
            // dataGridViewImageColumn1
            // 
            this.dataGridViewImageColumn1.DataPropertyName = "Severity";
            this.dataGridViewImageColumn1.HeaderText = "Severity";
            this.dataGridViewImageColumn1.Name = "dataGridViewImageColumn1";
            this.dataGridViewImageColumn1.ReadOnly = true;
            this.dataGridViewImageColumn1.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewImageColumn1.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            // 
            // dataGridViewTextBoxColumn5
            // 
            this.dataGridViewTextBoxColumn5.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn5.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            this.dataGridViewTextBoxColumn5.ReadOnly = true;
            this.dataGridViewTextBoxColumn5.Visible = false;
            // 
            // dataGridViewImageColumn2
            // 
            this.dataGridViewImageColumn2.DataPropertyName = "Severity";
            this.dataGridViewImageColumn2.HeaderText = "Severity";
            this.dataGridViewImageColumn2.Name = "dataGridViewImageColumn2";
            this.dataGridViewImageColumn2.ReadOnly = true;
            this.dataGridViewImageColumn2.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewImageColumn2.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            this.dataGridViewImageColumn2.Width = 50;
            // 
            // dataGridViewTextBoxColumn6
            // 
            this.dataGridViewTextBoxColumn6.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn6.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn6.Name = "dataGridViewTextBoxColumn6";
            this.dataGridViewTextBoxColumn6.ReadOnly = true;
            this.dataGridViewTextBoxColumn6.Visible = false;
            // 
            // dataGridViewImageColumn3
            // 
            this.dataGridViewImageColumn3.DataPropertyName = "Severity";
            this.dataGridViewImageColumn3.HeaderText = "Severity";
            this.dataGridViewImageColumn3.Name = "dataGridViewImageColumn3";
            this.dataGridViewImageColumn3.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewImageColumn3.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Programmatic;
            this.dataGridViewImageColumn3.Width = 50;
            // 
            // dataGridViewTextBoxColumn7
            // 
            this.dataGridViewTextBoxColumn7.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn7.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn7.Name = "dataGridViewTextBoxColumn7";
            this.dataGridViewTextBoxColumn7.Visible = false;
            // 
            // dataGridViewImageColumn4
            // 
            this.dataGridViewImageColumn4.DataPropertyName = "Severity";
            this.dataGridViewImageColumn4.HeaderText = "Severity";
            this.dataGridViewImageColumn4.Name = "dataGridViewImageColumn4";
            this.dataGridViewImageColumn4.ReadOnly = true;
            this.dataGridViewImageColumn4.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewImageColumn4.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Programmatic;
            this.dataGridViewImageColumn4.Width = 50;
            // 
            // dataGridViewTextBoxColumn8
            // 
            this.dataGridViewTextBoxColumn8.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn8.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn8.Name = "dataGridViewTextBoxColumn8";
            this.dataGridViewTextBoxColumn8.ReadOnly = true;
            this.dataGridViewTextBoxColumn8.Visible = false;
            // 
            // dataGridViewImageColumn5
            // 
            this.dataGridViewImageColumn5.DataPropertyName = "Severity";
            this.dataGridViewImageColumn5.HeaderText = "Severity";
            this.dataGridViewImageColumn5.Name = "dataGridViewImageColumn5";
            this.dataGridViewImageColumn5.ReadOnly = true;
            this.dataGridViewImageColumn5.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewImageColumn5.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            this.dataGridViewImageColumn5.Width = 50;
            // 
            // dataGridViewTextBoxColumn9
            // 
            this.dataGridViewTextBoxColumn9.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn9.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn9.Name = "dataGridViewTextBoxColumn9";
            this.dataGridViewTextBoxColumn9.ReadOnly = true;
            this.dataGridViewTextBoxColumn9.Visible = false;
            // 
            // dataGridViewTextBoxColumn10
            // 
            this.dataGridViewTextBoxColumn10.DataPropertyName = "Severity";
            this.dataGridViewTextBoxColumn10.HeaderText = "Severity";
            this.dataGridViewTextBoxColumn10.Name = "dataGridViewTextBoxColumn10";
            this.dataGridViewTextBoxColumn10.ReadOnly = true;
            this.dataGridViewTextBoxColumn10.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewTextBoxColumn10.Width = 50;
            // 
            // dataGridViewTextBoxColumn11
            // 
            this.dataGridViewTextBoxColumn11.DataPropertyName = "messageObject";
            this.dataGridViewTextBoxColumn11.HeaderText = "messageObject";
            this.dataGridViewTextBoxColumn11.Name = "dataGridViewTextBoxColumn11";
            this.dataGridViewTextBoxColumn11.ReadOnly = true;
            this.dataGridViewTextBoxColumn11.Visible = false;
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
            // 
            // NetLogClientView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.mainSplitContainer);
            this.Name = "NetLogClientView";
            this.Size = new System.Drawing.Size(583, 323);
            this.mainSplitContainer.Panel1.ResumeLayout(false);
            this.mainSplitContainer.Panel1.PerformLayout();
            this.mainSplitContainer.Panel2.ResumeLayout(false);
            this.mainSplitContainer.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.messagesGridView)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.messagesDataSet)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.Messages)).EndInit();
            this.clientToolStrip.ResumeLayout(false);
            this.clientToolStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.SplitContainer mainSplitContainer;
        private System.Windows.Forms.ToolStrip clientToolStrip;
        private System.Windows.Forms.ToolStripButton attachBtn;
        private StatusStrip statusStrip;
        private ListView messageListView;
        public System.Data.DataSet messagesDataSet;
        private System.Data.DataTable Messages;
        private System.Data.DataColumn SeverityColumn;
        private System.Data.DataColumn AddedColumn;
        private System.Data.DataColumn messageColumn;
        private System.Data.DataColumn messageObjectColumn;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private ColumnHeader columnFieldName;
        private ColumnHeader columnValue;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private DataGridView messagesGridView;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private DataGridViewImageColumn dataGridViewImageColumn1;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private DataGridViewImageColumn dataGridViewImageColumn2;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn6;
        private ToolStripButton detachBtn;
        private ToolStripButton closeTabBtn;
        private ToolStripButton clearMessagesBtn;
        private DataGridViewImageColumn dataGridViewImageColumn3;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn7;
        private DataGridViewImageColumn dataGridViewImageColumn4;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn8;
        private ToolStripDropDownButton verbosityDoropDown;
        private ToolStripMenuItem verbosityNoTraceMenuItem;
        private ToolStripMenuItem verbosityTraceDebugMenuItem;
        private ToolStripMenuItem verbosityTraceStateMenuItem;
        private ToolStripMenuItem verbosityCallTraceMenuItem;
        private ToolStripButton setDelayModeBtn;
        private ToolStripSeparator toolStripSeparator1;
        private DataGridViewImageColumn dataGridViewImageColumn5;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn9;
        private ToolStripButton saveMessagesBtn;
        private DataGridViewTextBoxColumn severityDataGridViewTextBoxColumn;
        private DataGridViewTextBoxColumn addedDataGridViewTextBoxColumn;
        private DataGridViewTextBoxColumn messageDataGridViewTextBoxColumn;
        private DataGridViewTextBoxColumn messageObjectDataGridViewTextBoxColumn;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn10;
        private DataGridViewTextBoxColumn dataGridViewTextBoxColumn11;
        private ToolStripSeparator toolStripSeparator2;

    }
}
