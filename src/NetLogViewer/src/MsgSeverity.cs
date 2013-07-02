using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.Drawing;
using System.Resources;

namespace NetLogViewer
{
    /// <summary>
    /// Log message severity
    /// </summary>
    public enum MsgSeverity
    {
        Message             = 0,
        Warning             = 1,
        Error               = 2,
        Exception           = 3,
        UnitTestSuite       = 4,
        UnitTestCase        = 5,
        FunctionalTestSuite = 6,
        FunctionalTestCase  = 7
    }

    /// <summary>
    /// Severity icons singleton
    /// </summary>
    public class MsgSeverityIcons
    {
        #region private members

        /// <summary>
        /// Unique class instance
        /// </summary>
        private static MsgSeverityIcons _instance = null;

        /// <summary>
        /// Severity icons table
        /// </summary>
        private Hashtable _iconsTable;

        #endregion //private members

        #region ctors
        /// <summary>
        /// private ctor
        /// </summary>
        private MsgSeverityIcons()
        {
            try
            {
                _iconsTable = new Hashtable();
                _iconsTable[MsgSeverity.Error] = new Icon(NetLogViewer.res.Resource1.Error, 16, 16);
                _iconsTable[MsgSeverity.Exception] = new Icon(NetLogViewer.res.Resource1.Exception, 16, 16);
                _iconsTable[MsgSeverity.Warning] = new Icon(NetLogViewer.res.Resource1.Warning, 16, 16);
                _iconsTable[MsgSeverity.Message] = new Icon(NetLogViewer.res.Resource1.Message, 16, 16);
                _iconsTable[MsgSeverity.UnitTestCase] = new Icon(NetLogViewer.res.Resource1.UnitTestCase, 16, 16);
                _iconsTable[MsgSeverity.UnitTestSuite] = new Icon(NetLogViewer.res.Resource1.UnitTestSuite, 16, 16);
                _iconsTable[MsgSeverity.FunctionalTestCase] = new Icon(NetLogViewer.res.Resource1.FunctionalTestCase, 16, 16);
                _iconsTable[MsgSeverity.FunctionalTestSuite] = new Icon(NetLogViewer.res.Resource1.FunctionalTestSuite, 16, 16);
            }
            catch (Exception exception)
            {
                System.Windows.Forms.MessageBox.Show(exception.ToString());
            }
        }
        #endregion //ctors


        #region public methods

        /// <summary>
        /// Returns unique object instance
        /// </summary>
        public static MsgSeverityIcons Instance
        {
            get
            {
                if (_instance == null)
                    _instance = new MsgSeverityIcons();
                return _instance;
            }
        }

        /// <summary>
        /// Returns icon for severity
        /// </summary>
        /// <param name="severity">severity</param>
        /// <returns>icon for severity</returns>
        public Icon GetSeverityIcon(MsgSeverity severity)
        {
            if (_iconsTable.Contains(severity))
            {

                return _iconsTable[severity] as Icon;
            } else
            {
                return System.Drawing.SystemIcons.Question;
            }
        }

        #endregion //public methods

    }
}
