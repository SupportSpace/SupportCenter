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
    /// SetDelayMode action
    /// </summary>
    public class SetDelayModeAction : IAction
    {

        #region private members

        /// <summary>
        /// assigned to action netlog client
        /// </summary>
        private LogClient _client;

        /// <summary>
        /// New delay mode value
        /// </summary>
        private bool _delayMode;

        #endregion private members

        #region public methods

        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="client">client to perform attach</param>
        /// <param name="verbosity">new delay mode</param>
        public SetDelayModeAction(LogClient client, bool delayMode)
        {
            if (client == null)
                throw new ArgumentNullException("client");
            _client = client;
            _delayMode = delayMode;
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
                    _client.InnerObj.DelayedMode = _delayMode;
                }
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(), "Failed to set delay mode");
            }
        }

        #endregion public methods
    }
}
