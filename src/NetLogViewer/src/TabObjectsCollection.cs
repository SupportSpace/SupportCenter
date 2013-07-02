using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Collections;

namespace NetLogViewer
{
    /// <summary>
    /// Singleton object - collection of tab pages / objects
    /// </summary>
    class TabObjectsCollection
    {
        #region private members

        /// <summary>
        /// Unique object instance
        /// </summary>
        private static TabObjectsCollection _instance = null;

        /// <summary>
        /// Host TabControl object
        /// </summary>
        private TabControl _tabControl;

        /// <summary>
        /// tab - objects collection
        /// </summary>
        private Hashtable _objectsCollection;

        /// <summary>
        /// Private constructor
        /// </summary>
        private TabObjectsCollection()
        {
            _objectsCollection = new Hashtable();
        }

        #endregion //private members

        #region public properties
        /// <summary>
        /// Returns object unique instance
        /// </summary>
        public static TabObjectsCollection Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new TabObjectsCollection();
                }
                return _instance;
            }
        }

        /// <summary>
        /// Setup host TabControl Object
        /// </summary>
        public TabControl TabControl
        {
            set
            {
                if (value == null || ! (value is TabControl))
                    throw new ArgumentException("value");
                _tabControl = value as TabControl;
            }
        }
        #endregion //public properties

        #region public methods
        /// <summary>
        /// Adds object to tab objects collection
        /// </summary>
        /// <param name="obj">object to add</param>
        /// <returns>newly created tab</returns>
        public TabPage AddObject(Object obj)
        {
            if (obj == null)
                throw new ArgumentNullException("obj");
            if (_objectsCollection.Contains(obj))
                throw new Exception(string.Format("object {0} already exists in collection",obj.ToString()));
            TabPage tabPage = new TabPage(obj.ToString());
            _tabControl.TabPages.Add(tabPage);
            _objectsCollection.Add(obj, tabPage);
            return tabPage;
        }

        /// <summary>
        /// Deletes object from tab objects collection
        /// </summary>
        /// <param name="obj">object to delete</param>
        public void DeleteObject(Object obj)
        {
            if (obj == null)
                throw new ArgumentNullException("obj");
            if (!_objectsCollection.Contains(obj))
                throw new Exception(string.Format("object {0} not found in collection", obj.ToString()));
            _tabControl.TabPages.Remove(_objectsCollection[obj] as TabPage);
            _objectsCollection.Remove(obj);
        }

        /// <summary>
        /// Selects tab mapped to object
        /// </summary>
        /// <param name="obj"></param>
        public void SelectObject(Object obj)
        {
            if (obj == null)
                throw new ArgumentNullException("obj");
            if (!_objectsCollection.Contains(obj))
                throw new Exception(string.Format("object {0} not found in collection", obj.ToString()));
            _tabControl.SelectedTab = _objectsCollection[obj] as TabPage;
        }

        /// <summary>
        /// Finds tab, mapped to object
        /// </summary>
        /// <param name="obj"></param>
        /// <returns>found tab</returns>
        public TabPage FindObject(Object obj)
        {
            if (obj == null)
                throw new ArgumentNullException("obj");
            if (!_objectsCollection.Contains(obj))
                return null;
            return _objectsCollection[obj] as TabPage;
        }
        #endregion //public methods
    }
}
