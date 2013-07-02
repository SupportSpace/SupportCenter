namespace NetLogViewer
{
    partial class MainForm
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.tabClientsList = new System.Windows.Forms.TabPage();
            this.loadMessagesBtn = new System.Windows.Forms.Button();
            this.refreshBtn = new System.Windows.Forms.Button();
            this.attachClientBtn = new System.Windows.Forms.Button();
            this.clientsListBox = new System.Windows.Forms.ListBox();
            this.mainTabControl = new System.Windows.Forms.TabControl();
            this.mainStatusStrip = new System.Windows.Forms.StatusStrip();
            this.operationProgressBar = new System.Windows.Forms.ProgressBar();
            this.tabClientsList.SuspendLayout();
            this.mainTabControl.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabClientsList
            // 
            this.tabClientsList.Controls.Add(this.operationProgressBar);
            this.tabClientsList.Controls.Add(this.mainStatusStrip);
            this.tabClientsList.Controls.Add(this.loadMessagesBtn);
            this.tabClientsList.Controls.Add(this.refreshBtn);
            this.tabClientsList.Controls.Add(this.attachClientBtn);
            this.tabClientsList.Controls.Add(this.clientsListBox);
            this.tabClientsList.Location = new System.Drawing.Point(4, 22);
            this.tabClientsList.Name = "tabClientsList";
            this.tabClientsList.Padding = new System.Windows.Forms.Padding(3);
            this.tabClientsList.Size = new System.Drawing.Size(764, 402);
            this.tabClientsList.TabIndex = 0;
            this.tabClientsList.Text = "Alive clients";
            this.tabClientsList.UseVisualStyleBackColor = true;
            // 
            // loadMessagesBtn
            // 
            this.loadMessagesBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.loadMessagesBtn.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.loadMessagesBtn.Location = new System.Drawing.Point(683, 61);
            this.loadMessagesBtn.Name = "loadMessagesBtn";
            this.loadMessagesBtn.Size = new System.Drawing.Size(75, 23);
            this.loadMessagesBtn.TabIndex = 3;
            this.loadMessagesBtn.Text = "Load";
            this.loadMessagesBtn.UseVisualStyleBackColor = true;
            this.loadMessagesBtn.Click += new System.EventHandler(this.loadMessagesBtn_Click);
            // 
            // refreshBtn
            // 
            this.refreshBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.refreshBtn.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.refreshBtn.Location = new System.Drawing.Point(683, 32);
            this.refreshBtn.Name = "refreshBtn";
            this.refreshBtn.Size = new System.Drawing.Size(75, 23);
            this.refreshBtn.TabIndex = 2;
            this.refreshBtn.Text = "Refresh";
            this.refreshBtn.UseVisualStyleBackColor = true;
            this.refreshBtn.Click += new System.EventHandler(this.refreshBtn_Click);
            // 
            // attachClientBtn
            // 
            this.attachClientBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.attachClientBtn.Enabled = false;
            this.attachClientBtn.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.attachClientBtn.Location = new System.Drawing.Point(683, 3);
            this.attachClientBtn.Name = "attachClientBtn";
            this.attachClientBtn.Size = new System.Drawing.Size(75, 23);
            this.attachClientBtn.TabIndex = 1;
            this.attachClientBtn.Text = "Attach";
            this.attachClientBtn.UseVisualStyleBackColor = true;
            this.attachClientBtn.Click += new System.EventHandler(this.attachClientBtn_Click);
            // 
            // clientsListBox
            // 
            this.clientsListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.clientsListBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.clientsListBox.FormattingEnabled = true;
            this.clientsListBox.Location = new System.Drawing.Point(0, 1);
            this.clientsListBox.Name = "clientsListBox";
            this.clientsListBox.Size = new System.Drawing.Size(680, 379);
            this.clientsListBox.TabIndex = 0;
            this.clientsListBox.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.clientsListBox_MouseDoubleClick);
            this.clientsListBox.SelectedIndexChanged += new System.EventHandler(this.clientsListBox_SelectedIndexChanged);
            // 
            // mainTabControl
            // 
            this.mainTabControl.Controls.Add(this.tabClientsList);
            this.mainTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainTabControl.Location = new System.Drawing.Point(0, 0);
            this.mainTabControl.Name = "mainTabControl";
            this.mainTabControl.SelectedIndex = 0;
            this.mainTabControl.Size = new System.Drawing.Size(772, 428);
            this.mainTabControl.TabIndex = 2;
            this.mainTabControl.SelectedIndexChanged += new System.EventHandler(this.mainTabControl_SelectedIndexChanged);
            // 
            // mainStatusStrip
            // 
            this.mainStatusStrip.GripMargin = new System.Windows.Forms.Padding(0);
            this.mainStatusStrip.Location = new System.Drawing.Point(3, 377);
            this.mainStatusStrip.Name = "mainStatusStrip";
            this.mainStatusStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
            this.mainStatusStrip.Size = new System.Drawing.Size(758, 22);
            this.mainStatusStrip.TabIndex = 5;
            // 
            // operationProgressBar
            // 
            this.operationProgressBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.operationProgressBar.Location = new System.Drawing.Point(3, 381);
            this.operationProgressBar.Name = "operationProgressBar";
            this.operationProgressBar.Size = new System.Drawing.Size(738, 15);
            this.operationProgressBar.TabIndex = 6;
            this.operationProgressBar.Visible = false;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(772, 428);
            this.Controls.Add(this.mainTabControl);
            this.Name = "MainForm";
            this.Text = "NetLog Viewer";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.tabClientsList.ResumeLayout(false);
            this.tabClientsList.PerformLayout();
            this.mainTabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabPage tabClientsList;
        private System.Windows.Forms.Button refreshBtn;
        private System.Windows.Forms.Button attachClientBtn;
        private System.Windows.Forms.ListBox clientsListBox;
        private System.Windows.Forms.TabControl mainTabControl;
        private System.Windows.Forms.Button loadMessagesBtn;
        private System.Windows.Forms.ProgressBar operationProgressBar;
        private System.Windows.Forms.StatusStrip mainStatusStrip;

    }
}

