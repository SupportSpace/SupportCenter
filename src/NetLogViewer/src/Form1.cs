using System;
using System.Collections.Generic;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using NetLogViewerLib;
using System.Runtime.InteropServices;


namespace NetLogViewer
{
    public partial class MainForm : Form
    {
        #region private members

        /// <summary>
        /// NetLogViewer com object
        /// </summary>
        private CNetLogViewer _logViewer;

        /// <summary>
        /// Refreshes main form controls state
        /// </summary>
        private void RefreshControls()
        {
            if (clientsListBox.SelectedIndex >= 0)
            {
                attachClientBtn.Enabled = true; //;
                if (new Attach2ClientAction(clientsListBox.SelectedItem as LogClient).Active)
                    attachClientBtn.Text = "Attach";
                else
                    attachClientBtn.Text = "View";
            }
            else
                attachClientBtn.Enabled = false;
        }

        #endregion //private members

        #region ctors

        /// <summary>
        /// Initialiazes main form object instance
        /// </summary>
        public MainForm()
        {
            InitializeComponent();
        }

        #endregion //#region ctors

        #region event handlers
        /// <summary>
        /// Client timed out event handler
        /// </summary>
        /// <param name="client">timed out client</param>
        void _logViewer_OnClientTimedOut(CNetLogClient client)
        {
            try
            {
                if (client == null)
                    throw new ArgumentNullException("client");
                clientsListBox.Items.Remove(new LogClient(client));
            }
            catch (System.Exception exception)
            {
                System.Windows.Forms.MessageBox.Show(exception.ToString(), "Error gettint clients", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        /// <summary>
        /// Delegate type for sync call on found client
        /// </summary>
        /// <param name="client"></param>
        public delegate void DelegateClientFound(CNetLogClient client);

        /// <summary>
        ///  Client found event handler
        /// </summary>
        /// <param name="client">found client</param>
        void _logViewer_OnClientFound(CNetLogClient client)
        {
            try
            {
                if (client == null)
                    throw new ArgumentNullException("client");
                clientsListBox.Invoke(new DelegateClientFound(this.logViewer_OnClientFoundSync), new object[] { client });
            }
            catch (System.Exception exception)
            {
                System.Windows.Forms.MessageBox.Show(exception.ToString(), "Error gettint clients", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        /// <summary>
        /// Client found event handler from GUI thread
        /// </summary>
        /// <param name="client"></param>
        void logViewer_OnClientFoundSync(CNetLogClient client)
        {
            try
            {
                clientsListBox.Items.Add(new LogClient(client));
            }
            catch (System.Exception exception)
            {
                System.Windows.Forms.MessageBox.Show(exception.ToString(), "Error gettint clients", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        /// <summary>
        /// List box selection changed event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void clientsListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshControls();
        }

        /// <summary>
        /// refresh client list event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void refreshBtn_Click(object sender, EventArgs e)
        {
            try
            {
                clientsListBox.Items.Clear();
                System.Collections.IEnumerable clientList = _logViewer.GetClientsList();
                if (clientList == null)
                {
                    throw new System.Exception("GetClientsList returned null enumerable");
                }
                foreach (Object obj in clientList)
                {
                    CNetLogClient logClient = obj as CNetLogClient;
                    if (logClient != null)
                    {
                        clientsListBox.Items.Add(new LogClient(logClient));
                    }
                }
            }
            catch (System.Exception exception)
            {
                System.Windows.Forms.MessageBox.Show(exception.ToString(), "Error gettint clients", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        /// <summary>
        /// Attach 2 client button clicked event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void attachClientBtn_Click(object sender, EventArgs e)
        {
            if (clientsListBox.SelectedIndex >= 0)
                (new Attach2ClientAction(clientsListBox.SelectedItem as LogClient)).Execute();
        }

        /// <summary>
        /// Double click on client list event handler (attaching to client)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void clientsListBox_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (clientsListBox.SelectedIndex >=0)
                (new Attach2ClientAction(clientsListBox.SelectedItem as LogClient)).Execute();
        }

        /// <summary>
        /// Form loaded event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainForm_Load(object sender, EventArgs e)
        {
            _logViewer = new CNetLogViewerClass();
            _logViewer.OnClientFound += new _INetLogViewerEvents_OnClientFoundEventHandler(_logViewer_OnClientFound);
            _logViewer.OnClientTimedOut += new _INetLogViewerEvents_OnClientTimedOutEventHandler(_logViewer_OnClientTimedOut);
            _logViewer.Start();
            TabObjectsCollection.Instance.TabControl = mainTabControl;
        }

        /// <summary>
        /// Tab page changed event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void mainTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshControls();
        }

        /// <summary>
        /// Load messages button pressed
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void loadMessagesBtn_Click(object sender, EventArgs e)
        {
            new LoadMessagesAction(operationProgressBar).Execute();
        }

        #endregion //event handlers

        /*[DllImport("Kernel32.dll")]
        private static extern bool TerminateProcess(int handle, int exitCode);
        [DllImport("kernel32.dll")]
        static extern uint GetCurrentProcessId();
        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            // deadlock crutch
            TerminateProcess((int)GetCurrentProcessId(), 0);
        }*/
    }
}