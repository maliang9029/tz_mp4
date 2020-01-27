
// mfc_demoDlg.h : 头文件
//

#pragma once


// Cmfc_demoDlg 对话框
class Cmfc_demoDlg : public CDialog
{
// 构造
public:
	Cmfc_demoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MFC_DEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
    unsigned int m_lPlayId ;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnNextFrame();
    afx_msg void OnBnClickedBtnPreFrame();
    afx_msg void OnBnClickedBtnPause();
    afx_msg void OnBnClickedBtnResume();
    afx_msg void OnBnClickedBtnPlay();
    afx_msg void OnBnClickedBtnSpeed();
    afx_msg void OnBnClickedBtnSnap();
	afx_msg void OnBnClickedBtnSeek();
};
