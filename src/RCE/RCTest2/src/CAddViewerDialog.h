#pragma once
#include "afxwin.h"
#include <RCEngine/RCEngine.h>
#include <boost/shared_ptr.hpp>
#include <map>

class COutStreamGZipped;
class CDirectNetworkStream;

// CAddViewerDialog dialog

class CAddViewerDialog : public CDialog
{
private:
	/// Current stream object
	boost::shared_ptr<CDirectNetworkStream> m_stream;
	boost::shared_ptr<CAbstractStream> m_astream;
	boost::shared_ptr<COutStreamGZipped> m_shadowStream;

	static const tstring m_fullScreenStr;
	static const tstring m_scaleStr;
	static const tstring m_scrollStr;

	char m_dispModeStr[MAX_PATH];

	DECLARE_DYNAMIC(CAddViewerDialog)

	/// encodings map
	std::map<int,int> m_encodings;
public:
    
	CAddViewerDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddViewerDialog();

	/// Returns new connected stream or NULL if failed
	/// @return new connected stream or NULL if failed
	boost::shared_ptr<CAbstractStream> GetNewConnectedStream();


// Dialog Data
	enum { IDD = IDD_ADD_VIEWER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CEdit m_hostEditBox;
	CEdit m_remotePortEditBox;
	CEdit m_localPortEditBox;
	CButton m_connectButton;
	CButton m_setShadowStreamButton;
	CComboBox m_displayModeCombo;
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonShadowStream();
	afx_msg void OnBnClickedButtonCreateFromShadowStream();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeDisplayModeCombo();
	CStatic m_encodingGroupBox;
	CButton m_autoDetectSettingsCheck;
	afx_msg void OnBnClickedAutodetectCheck();
	BOOL m_selEncoding;
	BOOL m_selColors;
	CButton m_useCopyRectCheck;
	CStatic m_colorsGroupBox;
	CButton m_useCacheCheck;
	CButton m_zipTightCompressionCheck;
	CButton m_jpegQualityCheck;
	CEdit m_zipCompressionEdit;
	CEdit m_jpegQualityEdit;
	CButton m_viewOnlyCheckBox;

	bool m_useCustomOptions;
	SViewerOptions m_customOptions;
	bool m_viewOnly;
	bool m_visualPointer;
	BOOL m_captureLayered;

	virtual void OnOK();
	void ReadOptions();
public:
	void OnConnectError(void*, EConnectErrorReason reason);
	void OnConnected(void*);
	void OnDisconnected(void*);
	
	/// Set viewer options
	/// @param viewer viewer to set options
	void SetOptions(CRCViewer* viewer);
	bool DisplayModeSelected();

	EDisplayMode GetDisplayMode();
	CButton m_visualPointerCheckBox;
	CEdit m_sourcePeerEdit;
	CEdit m_destPeerEdit;
	CEdit m_sessionIdEdit;
	afx_msg void OnBnClickedButtonConnect2();
};
