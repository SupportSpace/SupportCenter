using System;
using System.Collections.Generic;
using System.Text;
using NetLogViewerLib;
using System.Xml.Serialization;
using System.Xml;
using System.Xml.Schema;

namespace NetLogViewer
{

    /// <summary>
    /// NetLog client state enumeration
    /// </summary>
    public enum LogClientState
    {
        Detached = 0,
        Attaching = 1,
        Attached = 2,
    }

    /// <summary>
    /// Log client verbosity enumeration
    /// </summary>
    public enum LogVerbosity
    {
        NoTrace = 0,
        TraceDebug = 1,
        TraseStates = 2,
        TraceCalls = 3
    }

    /// <summary>
    /// Simple wrapper for log client object
    /// </summary>
    public class LogClient
    {
        #region private members
        
        private CNetLogClient _logClient;

        #endregion //private members

        #region ctors
        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="logClient">log client object</param>
        public LogClient(CNetLogClient logClient)
        {
            if (logClient == null)
                throw new ArgumentNullException("logClient");
            _logClient = logClient;
        }
        #endregion //ctors

        #region public declarations
        /// <summary>
        /// Retutns internal object instance
        /// </summary>
        public CNetLogClient InnerObj
        {
            get
            {
                return _logClient;
            }
        }

        /// <summary>
        /// Returns name of internal client object
        /// </summary>
        /// <returns>name of internal client object</returns>
        public override string ToString()
        {
            return _logClient.Name;
        }

        /// <summary>
        /// Equasl override. Assuming that cient name should be unique
        /// </summary>
        /// <param name="obj">object for comparision</param>
        /// <returns>true if object names are equal</returns>
        public override bool Equals(Object obj)
        {
            if (!(obj is LogClient))
                return false;
            return (obj as LogClient).InnerObj.Name == _logClient.Name;
        }

        /// <summary>
        /// Returns object hash code
        /// </summary>
        /// <returns>object hash code</returns>
        public override int GetHashCode()
        {
            return _logClient.Name.GetHashCode();
        }
        #endregion //public declarations
    }

    /// <summary>
    /// Simple wrapper for log client object
    /// </summary>
    public class LogMessage : IXmlSerializable
    {
        #region private members

        private CLogMessage _LogMessage;

        #endregion //private members

        #region ctors
        /// <summary>
        /// Initializes object instance
        /// </summary>
        /// <param name="LogMessage">log client object</param>
        public LogMessage(CLogMessage LogMessage)
        {
            if (LogMessage == null)
                throw new ArgumentNullException("LogMessage");
            _LogMessage = LogMessage;
        }

        /// <summary>
        /// Parameterless ctor
        /// </summary>
        public LogMessage()
        {
            _LogMessage = new CLogMessageClass();
        }
        #endregion //ctors

        #region public declarations
        /// <summary>
        /// Retutns internal object instance
        /// </summary>
        public CLogMessage InnerObj
        {
            get
            {
                return _LogMessage;
            }
        }

        /// <summary>
        /// Returns name of internal message object
        /// </summary>
        /// <returns>name of internal client object</returns>
        public override string ToString()
        {
            return _LogMessage.Message;
        }

        /// <summary>
        /// Serializes object to xml
        /// </summary>
        /// <param name="writer"></param>
        public virtual void WriteXml( XmlWriter writer )
        {
            string str;
            InnerObj.Serialize2String(out str);
            writer.WriteString(string.Format("{0}{1}",InnerObj.Severity.ToString(),str));
        }

        /// <summary>
        /// DeSerializes object from xml
        /// </summary>
        /// <param name="writer"></param>
        public virtual void ReadXml( XmlReader reader )
        {
            string str = reader.ReadString();
            InnerObj.Severity = (short)int.Parse(str.Substring(0, 1));
            str = str.Remove(0, 1);
            InnerObj.DeserializeFromString(str);
        }

        /// <summary>
        /// Returns null
        /// </summary>
        /// <returns></returns>
        public virtual XmlSchema GetSchema ()
        {
            return null;
        }

        #endregion //public declarations
    }
}
