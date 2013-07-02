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
    /// LoadMessages action
    /// </summary>
    public class LoadMessagesAction : IAction
    {

        #region private members
        ProgressBar _progressBar;
        #endregion private members

        #region public methods

        /// <summary>
        /// Initializes object instance
        /// </summary>
        public LoadMessagesAction(ProgressBar progressBar)
        {
            _progressBar = progressBar;
        }

        /// <summary>
        /// Retutrns true if action could be performed
        /// </summary>
        /// <returns>true if action could be performed</returns>
        public bool Active
        {
            get
            {
                return true;
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
                    OpenFileDialog openDialog = new OpenFileDialog();
                    openDialog.CheckFileExists = true;
                    openDialog.Filter = "Log files (*.xml,*.log)|*.xml;*.log|All files|*";
                    openDialog.DefaultExt = "xml";
                    openDialog.Multiselect = false;
                    if (openDialog.ShowDialog() != DialogResult.OK)
                        return;
                    string fileName = openDialog.FileName;
                    int slashPos = System.Math.Max(fileName.LastIndexOf('\\'), fileName.LastIndexOf('/'));
                    string shortName = null;
                    if (slashPos >= 0)
                        shortName = fileName.Remove(0, slashPos + 1);
                    else
                        shortName = fileName;
                    LogClient client = new LogClient(new CNetLogClientClass());
                    client.InnerObj.Name = shortName;
                    NetLogClientView clientView = new NetLogClientView(client);
                    TabPage tabPage = TabObjectsCollection.Instance.FindObject(client);
                    if (tabPage != null)
                    {
                        new CloseClientTabAction(client).Execute();
                    }
                    // Adding new tab page
                    tabPage = TabObjectsCollection.Instance.AddObject(client);
                    clientView.Parent = tabPage;
                    clientView.Dock = DockStyle.Fill;
                    if (fileName.ToLower().EndsWith(".xml"))
                        clientView.messagesDataSet.ReadXml(fileName);
                    else
                    {
                        bool visible = false;
                        if (null != _progressBar)
                        {
                            visible = _progressBar.Visible;
                            _progressBar.Visible = true;
                            _progressBar.Value = 0;
                        }
                        try
                        {
                            new Log2DataSet(fileName, clientView.messagesDataSet, _progressBar).Parse();
                        }
                        finally
                        {
                            if (null != _progressBar)
                            {
                                _progressBar.Visible = visible;
                            }
                        }
                    }
                    TabObjectsCollection.Instance.SelectObject(client);
                }
            }
            catch (COMException exception)
            {
                MessageBox.Show(exception.ToString(), "Failed Load log messages");
            }
            catch (Exception exception)
            {
                MessageBox.Show(String.Format("File cannot be opened due to format incompatibility\n{0}\n{1}",exception.Message,exception.StackTrace), "Wrong file format", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch
            {
                MessageBox.Show("File cannot be opened due to format incompatibility", "Wrong file format", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        #endregion public methods
    }
}
