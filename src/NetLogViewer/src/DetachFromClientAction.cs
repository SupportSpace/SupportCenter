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
    /// DetachFromClient action
    /// </summary>
    public class DetachFromClientAction : IAction
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
        public DetachFromClientAction(LogClient client)
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
                return (LogClientState)_client.InnerObj.state == LogClientState.Attached;
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
                    _client.InnerObj.Detach();
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(), "Failed to detach from client");
            }
        }

        #endregion public methods
    }
}
