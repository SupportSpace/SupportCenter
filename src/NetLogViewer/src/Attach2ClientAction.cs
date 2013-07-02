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
    /// Attach2Client action
    /// </summary>
    public class Attach2ClientAction : IAction
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
        public Attach2ClientAction(LogClient client)
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
                return (LogClientState)_client.InnerObj.state != LogClientState.Attached;
            }
        }

        /// <summary>
        /// IAction Execute implementation
        /// </summary>
        public void Execute()
        {
            try
            {
                TabPage tabPage = TabObjectsCollection.Instance.FindObject(_client);
                if (tabPage != null)
                {
                    // Already have tab page
                    if (Active)
                        _client.InnerObj.Attach();
                }
                else
                {
                    NetLogClientView clientView = new NetLogClientView(_client);
                    // Adding new tab page
                    tabPage = TabObjectsCollection.Instance.AddObject(_client);
                    clientView.Parent = tabPage;
                    clientView.Dock = DockStyle.Fill;
                    // Attaching to client
                    _client.InnerObj.Attach();
                }
                TabObjectsCollection.Instance.SelectObject(_client);
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(),"Failed to attach to client");
            }
        }

        #endregion public methods
    }
}
