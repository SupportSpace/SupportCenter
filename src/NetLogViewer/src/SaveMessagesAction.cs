using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using NetLogViewerLib;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Data;


namespace NetLogViewer
{
    /// <summary>
    /// SaveMessages action
    /// </summary>
    public class SaveMessagesAction : IAction
    {

        #region private members

        /// <summary>
        /// assigned to action netlog client
        /// </summary>
        private LogClient _client;

        /// <summary>
        /// DataSet, containing log messages
        /// </summary>
        private DataSet _dataSet;

        #endregion private members

        #region public methods

        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="client">client to perform attach</param>
        /// <param name="DataSet">DataSet, containing messages</param>
        public SaveMessagesAction(LogClient client, DataSet dataSet)
        {
            if (client == null)
                throw new ArgumentNullException("client");
            if (dataSet == null)
                throw new ArgumentNullException("dataSet");
            if (dataSet.Tables.Count <= 0)
                throw new Exception("Dataset should contan at least one table");
            _client = client;
            _dataSet = dataSet;
        }

        /// <summary>
        /// Retutrns true if action could be performed
        /// </summary>
        /// <returns>true if action could be performed</returns>
        public bool Active
        {
            get
            {
                return _dataSet.Tables[0].Rows.Count > 0;
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
                    SaveFileDialog saveDialog = new SaveFileDialog();
                    saveDialog.Filter = "XML files (*.xml)|*.xml|All files|*";
                    saveDialog.DefaultExt = "xml";
                    string fileName = string.Format("{0}.xml", _client.ToString());
                    fileName = fileName.Replace(":", " ");
                    saveDialog.FileName = fileName;
                    if (saveDialog.ShowDialog() != DialogResult.OK)
                        return;
                    _dataSet.WriteXml(saveDialog.FileName);
                }
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(), "Failed save log messages");
            }
        }

        #endregion public methods
    }
}
