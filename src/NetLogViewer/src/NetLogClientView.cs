using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace NetLogViewer
{

    public partial class NetLogClientView : UserControl
    {
        #region private members

        /// <summary>
        /// assigned log client object
        /// </summary>
        private LogClient _logClient;

        /// <summary>
        /// Adds name / value pair to MessagesListView
        /// </summary>
        /// <param name="name">message field name</param>
        /// <param name="value">value</param>
        private void AddPair2MessageList(string name, object value)
        {
            if (name == null ||
                value == null ||
                (value is string && (value as string) == string.Empty) ||
                name == string.Empty)
                return;
            ListViewItem listViewItem = null;
            string valueStr = value.ToString();
            for (int pos = valueStr.IndexOf('\n');
                pos != -1;
                pos = valueStr.IndexOf('\n'))
            {
                listViewItem = new ListViewItem(new string[] { name, valueStr.Substring(0, pos) });
                messageListView.Items.Add(listViewItem);
                valueStr = valueStr.Remove(0, pos + 1);
                name = "";
            }
            listViewItem = new ListViewItem(new string[] { name, valueStr });
            messageListView.Items.Add(listViewItem);
        }

        /// <summary>
        /// Refreshes tool buttons states
        /// </summary>
        void RefreshToolButtonsState()
        {
            attachBtn.Enabled = new Attach2ClientAction(_logClient).Active;
            detachBtn.Enabled = new DetachFromClientAction(_logClient).Active;
            closeTabBtn.Enabled = new CloseClientTabAction(_logClient).Active;
            setDelayModeBtn.Enabled = new SetDelayModeAction(_logClient, false).Active;
            verbosityDoropDown.Enabled = new SetVerbosityAction(_logClient, LogVerbosity.TraseStates).Active;
            //saveMessagesBtn.Enabled = new SaveMessagesAction(_logClient, messagesDataSet).Active;
        }

        #endregion //private members

        #region ctors

        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="logClient">assigned logClient object</param>
        public NetLogClientView(LogClient logClient)
            : base()
        {
            if (logClient == null)
                throw new ArgumentNullException("logClient");
            _logClient = logClient;
            InitializeComponent();
            statusStrip.Items.Add("Unknown client state");
            logClient.InnerObj.OnStateChanged += new NetLogViewerLib._INetLogClientEvents_OnStateChangedEventHandler(InnerObj_OnStateChanged);
            logClient.InnerObj.OnLogMessage += new NetLogViewerLib._INetLogClientEvents_OnLogMessageEventHandler(InnerObj_OnLogMessage);
        }

        #endregion

        #region event handlers

        /// <summary>
        /// Delegate type for sync call on log message received event
        /// </summary>
        /// <param name="client"></param>
        public delegate void DelegateLogMessageReceived(NetLogViewerLib.CLogMessage Message);

        /// <summary>
        /// Log message received event handler
        /// </summary>
        /// <param name="Message"></param>
        void InnerObj_OnLogMessage(NetLogViewerLib.CLogMessage Message)
        {
            this.BeginInvoke(new DelegateLogMessageReceived(this.InnerObj_OnLogMessage_Sync), new object[] { Message });
        }

        /// <summary>
        /// Log message received event handler (called from control thread only)
        /// </summary>
        /// <param name="Message"></param>
        void InnerObj_OnLogMessage_Sync(NetLogViewerLib.CLogMessage Message)
        {
            DataRow row = messagesDataSet.Tables[0].NewRow();
            row.ItemArray = new object[] { Message.Severity, Message.ReceivedDate, Message.Message, new LogMessage(Message) };
            messagesDataSet.Tables[0].Rows.Add(row);
        }

        /// <summary>
        /// DataGrid selected row changed event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void messagesGridView_SelectionChanged(object sender, EventArgs e)
        {
            if (messagesGridView.SelectedRows.Count <= 0)
                return;
            NetLogViewerLib.CLogMessage Message = (messagesGridView.SelectedRows[0].Cells[3].Value as LogMessage).InnerObj;
            messageListView.Items.Clear();
            AddPair2MessageList("Severity", (MsgSeverity)Message.Severity);
            if (Message.AddedDate != DateTime.FromOADate(0))
                AddPair2MessageList("Added", Message.AddedDate);
            AddPair2MessageList("Message", Message.Message);
            AddPair2MessageList("PID", Message.PID);
            AddPair2MessageList("TID", Message.TID);
            AddPair2MessageList("SysError", Message.SysError);
            AddPair2MessageList("TestCase", Message.TestCase);
            AddPair2MessageList("TestSuite", Message.TestSuite);
            AddPair2MessageList("CompileDate", Message.CompileDate);
            AddPair2MessageList("FileName", Message.FileName);
            if (Message.FileLine != 0)
                AddPair2MessageList("FileLine", Message.FileLine);
            AddPair2MessageList("CallStack", Message.CallStack);
        }

        /// <summary>
        /// Delegate type for sync call on log client state changed
        /// </summary>
        /// <param name="client"></param>    
        public delegate void DelegateStateChanged(short state);

        /// <summary>
        /// State changed event handler
        /// </summary>
        /// <param name="state"></param>
        void InnerObj_OnStateChanged(short state)
        {
            //this.Invoke(new DelegateStateChanged(this.InnerObj_OnStateChangedSync), new object[] { state });
            //System.Windows.Forms.MessageBox.Show(((LogClientState)state).ToString());
            //StatusStrip
            InnerObj_OnStateChangedSync(state);
        }

        /// <summary>
        /// State changed event handler from GUI thread
        /// </summary>
        /// <param name="state"></param>
        void InnerObj_OnStateChangedSync(short state)
        {
            statusStrip.Items[0].Text = ((LogClientState)state).ToString();
            RefreshToolButtonsState();
        }

        /// <summary>
        /// Attach 2 client tool button click event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void attachBtn_Click(object sender, EventArgs e)
        {
            new Attach2ClientAction(_logClient).Execute();
        }

        /// <summary>
        /// Detach from client tool button click event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void detachBtn_Click(object sender, EventArgs e)
        {
            new DetachFromClientAction(_logClient).Execute();
        }

        /// <summary>
        /// Close log client tab action
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void closeTabBtn_Click(object sender, EventArgs e)
        {
            new CloseClientTabAction(_logClient).Execute();
        }

        /// <summary>
        /// Clears all messages in list button
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void clearMessagesBtn_Click(object sender, EventArgs e)
        {
            messagesDataSet.Tables[0].Rows.Clear();
        }

        /// <summary>
        /// Sort function for saverity column
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void messagesGridView_SortCompare(object sender, DataGridViewSortCompareEventArgs e)
        {
            if (e.Column == severityDataGridViewTextBoxColumn)
            {
                e.SortResult = e.CellValue1.GetHashCode().CompareTo(e.CellValue2.GetHashCode());
            }
        }

        private void verbosityCallTraceMenuItem_Click(object sender, EventArgs e)
        {
            new SetVerbosityAction(_logClient, LogVerbosity.TraceCalls).Execute();
        }

        private void verbosityTraceStateMenuItem_Click(object sender, EventArgs e)
        {
            new SetVerbosityAction(_logClient, LogVerbosity.TraseStates).Execute();
        }

        private void verbosityTraceDebugMenuItem_Click(object sender, EventArgs e)
        {
            new SetVerbosityAction(_logClient, LogVerbosity.TraceDebug).Execute();
        }

        private void verbosityNoTraceMenuItem_Click(object sender, EventArgs e)
        {
            new SetVerbosityAction(_logClient, LogVerbosity.NoTrace).Execute();
        }

        /// <summary>
        /// Click on setDelaybtn event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void setDelayModeBtn_Click(object sender, EventArgs e)
        {
            setDelayModeBtn.Checked = !setDelayModeBtn.Checked;
            new SetDelayModeAction(_logClient, setDelayModeBtn.Checked ).Execute();
        }

        /// <summary>
        /// Save log messages event handler
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void saveMessagesBtn_Click(object sender, EventArgs e)
        {
            new SaveMessagesAction(_logClient, messagesDataSet).Execute();
        }

        /// <summary>
        /// Cell owner drawing (for severity icons)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void messagesGridView_CellPainting(object sender, DataGridViewCellPaintingEventArgs e)
        {
            if (severityDataGridViewTextBoxColumn.Index == e.ColumnIndex
                &&
                e.RowIndex >= 0)
            {
                try
                {
                    Icon icon = MsgSeverityIcons.Instance.GetSeverityIcon((MsgSeverity)int.Parse(e.Value.ToString()));
                    //Brush gridBrush = new SolidBrush();
                    if ((e.State & DataGridViewElementStates.Selected) == DataGridViewElementStates.Selected)
                    {
                        e.Graphics.FillRectangle(System.Drawing.SystemBrushes.MenuHighlight, e.CellBounds);
                    }
                    else
                    {
                        e.Graphics.FillRectangle(System.Drawing.SystemBrushes.Window, e.CellBounds);
                    }
                    e.Graphics.DrawRectangle(System.Drawing.SystemPens.ControlDark, e.CellBounds.X - 1, e.CellBounds.Y -1, e.CellBounds.Width, e.CellBounds.Height);
                    e.Graphics.DrawIcon(icon,
                                        e.CellBounds.X + (e.CellBounds.Width - icon.Width) / 2,
                                        e.CellBounds.Y + (e.CellBounds.Height - icon.Height) / 2);
                    e.Handled = true;
                }
                catch
                {
                }
            }
        }

        #endregion //event handlers
    }
}
