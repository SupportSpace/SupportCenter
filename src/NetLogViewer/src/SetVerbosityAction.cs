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
    /// SetVerbosity action
    /// </summary>
    public class SetVerbosityAction : IAction
    {

        #region private members

        /// <summary>
        /// assigned to action netlog client
        /// </summary>
        private LogClient _client;

        /// <summary>
        /// New verbosity value
        /// </summary>
        private LogVerbosity _verbosity;

        #endregion private members

        #region public methods

        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="client">client to perform attach</param>
        /// <param name="verbosity">new verbosity value</param>
        public SetVerbosityAction(LogClient client, LogVerbosity verbosity)
        {
            if (client == null)
                throw new ArgumentNullException("client");
            _client = client;
            _verbosity = verbosity;
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
                {
                    _client.InnerObj.SetVerbosity((short)_verbosity);
                }
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(), "Failed to set verbosity");
            }
        }

        #endregion public methods
    }
}
