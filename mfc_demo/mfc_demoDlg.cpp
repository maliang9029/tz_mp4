
// mfc_demoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mfc_demo.h"
#include "mfc_demoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Cmfc_demoDlg 对话框




Cmfc_demoDlg::Cmfc_demoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cmfc_demoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cmfc_demoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cmfc_demoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_NEXT_FRAME, &Cmfc_demoDlg::OnBnClickedBtnNextFrame)
    ON_BN_CLICKED(IDC_BTN_PRE_FRAME, &Cmfc_demoDlg::OnBnClickedBtnPreFrame)
    ON_BN_CLICKED(IDC_BTN_PAUSE, &Cmfc_demoDlg::OnBnClickedBtnPause)
    ON_BN_CLICKED(IDC_BTN_RESUME, &Cmfc_demoDlg::OnBnClickedBtnResume)
    ON_BN_CLICKED(IDC_BTN_PLAY, &Cmfc_demoDlg::OnBnClickedBtnPlay)
    ON_BN_CLICKED(IDC_BTN_SPEED, &Cmfc_demoDlg::OnBnClickedBtnSpeed)
    ON_BN_CLICKED(IDC_BTN_SNAP, &Cmfc_demoDlg::OnBnClickedBtnSnap)
	ON_BN_CLICKED(IDC_BTN_SEEK, &Cmfc_demoDlg::OnBnClickedBtnSeek)
END_MESSAGE_MAP()


// Cmfc_demoDlg 消息处理程序

BOOL Cmfc_demoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_lPlayId = 0;
   
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Cmfc_demoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cmfc_demoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cmfc_demoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void Cmfc_demoDlg::OnBnClickedBtnNextFrame()
{
    // TODO: 在此添加控件通知处理程序代码
    //play_pause(m_lPlayId);

    play_step(m_lPlayId);
}

void Cmfc_demoDlg::OnBnClickedBtnPreFrame()
{
    // TODO: 在此添加控件通知处理程序代码
    play_step_prev(m_lPlayId);

}

void Cmfc_demoDlg::OnBnClickedBtnPause()
{
    // TODO: 在此添加控件通知处理程序代码
    play_pause(m_lPlayId);
}

void Cmfc_demoDlg::OnBnClickedBtnResume()
{
    // TODO: 在此添加控件通知处理程序代码
    play_resume(m_lPlayId);
}

void Cmfc_demoDlg::OnBnClickedBtnPlay()
{
    // TODO: 在此添加控件通知处理程序代码
     unsigned int hWnd = (unsigned int)GetDlgItem(IDC_STATIC)->m_hWnd;
    open_mp4(m_lPlayId,"",1920,1080,50);

    play_start(m_lPlayId,hWnd);
}

void Cmfc_demoDlg::OnBnClickedBtnSpeed()
{
    // TODO: 在此添加控件通知处理程序代码
    play_speed(m_lPlayId,2);
}

void Cmfc_demoDlg::OnBnClickedBtnSnap()
{
    // TODO: 在此添加控件通知处理程序代码
    play_snap(m_lPlayId,"D:\\snap");
}

void Cmfc_demoDlg::OnBnClickedBtnSeek()
{
	// TODO: 在此添加控件通知处理程序代码
	play_seek(m_lPlayId,0);
}
