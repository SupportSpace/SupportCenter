namespace NetLogViewer
{
    /// <summary>
    /// Actions interface
    /// </summary>
    interface IAction
    {
        /// <summary>
        /// Retutrns true if action could be performed
        /// </summary>
        /// <returns>true if action could be performed</returns>
        bool Active
        {
            get;
        }

        /// <summary>
        /// Runs action
        /// </summary>
        void Execute();
    }
}
