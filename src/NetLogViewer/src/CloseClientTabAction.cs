using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using NetLogViewerLib;
using System.Windows.Forms;
using System.Runtime.InteropServices;


namespace NetLogViewer
{
    /// <summary>
    /// CloseClientTab action
    /// </summary>
    public class CloseClientTabAction : IAction
    {

        #region private members

        /// <summary>
        /// assigned to action netlog client
        /// </summary>
        private LogClient _client;

        #endregion private members

        #region public methods

        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="client">client to perform attach</param>
        public CloseClientTabAction(LogClient client)
        {
            if (client == null)
                throw new ArgumentNullException("client");
            _client = client;
        }

        /// <summary>
        /// Retutrns true if action could be performed
        /// </summary>
        /// <returns>true if action could be performed</returns>
        public bool Active
        {
            get
            {
                return TabObjectsCollection.Instance.FindObject(_client) != null;
            }
        }

        /// <summary>
        /// IAction Execute implementation
        /// </summary>
        public void Execute()
        {
            try
            {
                if (Active)
                {
                    TabPage tabPage = TabObjectsCollection.Instance.FindObject(_client);
                    /// Detaching from client if attached
                    IAction detachAction = new DetachFromClientAction(_client);
                    if (detachAction.Active)
                        detachAction.Execute();
                    /// Removing tab page
                    if (tabPage.Parent is TabControl)
                        (tabPage.Parent as TabControl).TabPages.Remove(tabPage);
                    TabObjectsCollection.Instance.DeleteObject(_client);
                }
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(), "Failed to close tab");
            }
        }

        #endregion public methods
    }
}
