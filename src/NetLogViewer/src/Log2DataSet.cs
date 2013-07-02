using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using GoldParser;
using System.IO;
using NetLogViewerLib;
using System.Windows.Forms;
using System.Globalization;
using System.Threading;

namespace NetLogViewer
{
    /// <summary>
    /// Class for parsing filelogs to dataset
    /// </summary>
    public class Log2DataSet
    {
        private string _fileName;
        private DataSet _dataSet;
        private Parser _parser;
        private ProgressBar _progressBar;

        /// <summary>
        /// Removes from log string unnecessary dots and {CR}{LF}
        /// </summary>
        /// <param name="logString">log string to clean</param>
        /// <returns>cleaned string</returns>
        private static string CleanupLogString(string logString)
        {
            string cleanedString = logString;
            while(cleanedString.StartsWith("."))
                cleanedString = cleanedString.Remove(0, 1);
            cleanedString = cleanedString.Replace(";{CR}{LF}","");
            cleanedString = cleanedString.Replace(";{LF}", "");
            cleanedString = cleanedString.Replace("{CR}", "\r");
            cleanedString = cleanedString.Replace("{LF}", "\n");
            return cleanedString;
        }

        /// <summary>
        /// Inserts log message to dataset
        /// </summary>
        /// <param name="logMessage">log message</param>
        private void Insert2DataSet(CLogMessage Message)
        {
            if (null == Message)
                return;

            DataRow row = _dataSet.Tables[0].NewRow();
            row.ItemArray = new object[] { Message.Severity, Message.AddedDate, Message.Message, new LogMessage(Message) };
            _dataSet.Tables[0].Rows.Add(row);

        }

        /// <summary>
        /// constructs an object instance
        /// </summary>
        /// <param name="fileName">Log file to parse</param>
        /// <param name="dataSet">dataset to save results</param>
        public Log2DataSet(string fileName, DataSet dataSet, ProgressBar progressBar)
        {
            if (null == fileName)
                throw new ArgumentNullException("fileName");
            _fileName = fileName;

            if (null == dataSet)
                throw new ArgumentNullException("dataSet");
            _dataSet = dataSet;

            _progressBar = progressBar;
        }

        /// <summary>
        /// Perform parsing
        /// </summary>
        public void Parse()
        {
            FileInfo logFileInfo = new FileInfo(_fileName);
            logFileInfo.Attributes = FileAttributes.Normal;

            /// constructing grammar object
            MemoryStream stream = new MemoryStream(NetLogViewer.res.Resource1.AidLibLogs);
            Grammar grammar = new Grammar(new BinaryReader(stream));

            /// creating parser
            _parser = new Parser(new StreamReader(logFileInfo.OpenRead()), grammar);
            CLogMessage logMessage = new CLogMessage();
            string token = "", valueToken = "";
            if (null != _progressBar)
            {
                _progressBar.Minimum = 0;
                _progressBar.Maximum = (int)logFileInfo.Length;
            }
            for(int i=0;;++i)
            {
                if (null != _progressBar && i % 100 == 0)
                {
                    _progressBar.Value = _parser.CharPosition;
                    Application.DoEvents();
                }
                ParseMessage parseMsg = _parser.Parse();
                switch (parseMsg)
                {
                    case ParseMessage.Reduction:
                        if ("<message>" == _parser.ReductionRule.Name)
                        {
                            logMessage.Severity = 0; //_MESSAGE
                            Insert2DataSet(logMessage);
                            logMessage = new CLogMessage();
                        } 
                        else if ("<warning>" == _parser.ReductionRule.Name)
                        {
                            logMessage.Severity = 1; //_WARNING
                            Insert2DataSet(logMessage);
                            logMessage = new CLogMessage();
                        }
                        else if ("<error>" == _parser.ReductionRule.Name)
                        {
                            logMessage.Severity = 2; //_ERROR
                            Insert2DataSet(logMessage);
                            logMessage = new CLogMessage();
                        }
                        else if ("<exception>" == _parser.ReductionRule.Name)
                        {
                            logMessage.Severity = 3; //_EXCEPTION
                            Insert2DataSet(logMessage);
                            logMessage = new CLogMessage();
                        }
                        else if ("<timeValue>" == _parser.ReductionRule.Name)
                        {
                            DateTimeFormatInfo dateTimeFormatInfo = Thread.CurrentThread.CurrentCulture.DateTimeFormat.Clone() as DateTimeFormatInfo;
                            //System.Windows.Forms.MessageBox.Show(dateTimeFormatInfo.MonthDayPattern);
                            dateTimeFormatInfo.MonthDayPattern = "dd.MM";
                            logMessage.AddedDate = DateTime.Parse(valueToken, dateTimeFormatInfo);
                        }
                        else if ("<processValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.PID = valueToken;
                        }
                        else if ("<threadValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.TID = valueToken;
                        }
                        else if ("<sourceFileValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.FileName = valueToken;
                        }
                        else if ("<lineNumberValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.FileLine = short.Parse(valueToken);
                        }
                        else if ("<compileDateValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.CompileDate = valueToken;
                        }
                        else if ("<callStackValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.CallStack = valueToken;
                        }
                        else if ("<messageValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.Message = valueToken;
                        }
                        else if ("<systemErrorValue>" == _parser.ReductionRule.Name)
                        {
                            logMessage.SysError = valueToken;
                        }
                        break;
                    case ParseMessage.Accept:
                        if (null != _progressBar)
                            _progressBar.Value = _parser.CharPosition;
                        return;
                    case ParseMessage.InternalError:
                    case ParseMessage.LexicalError:
                    case ParseMessage.NotLoadedError:
                    case ParseMessage.SyntaxError:
                        throw new Exception(String.Format("{0} occured while parsing file", parseMsg.ToString()));
                    case ParseMessage.TokenRead:
                        valueToken = token;
                        token = CleanupLogString(_parser.TokenString);
                        break;
                }
            }
        }
    }
}
