
// CaseStudyExeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaseStudyExe.h"
#include "CaseStudyExeDlg.h"
#include "CloneBitmap.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiStringTools.h"
#include "NFmiFileSystem.h"
#include "execute-command-in-separate-process.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int GetUsedCoreCount()
{
#ifdef max
#undef max
#endif

    auto existingCoreCount = std::thread::hardware_concurrency();
    auto usedCoreCount = boost::math::iround(existingCoreCount * 0.3333);
    usedCoreCount = std::max(1, usedCoreCount);
    return usedCoreCount;
}

static std::string GetNumberOfCpuThreadsFor7zipOption()
{
    auto usedCoreCount = ::GetUsedCoreCount();
    if(usedCoreCount > 1)
    {
        std::string optionString = " -mmt";
        optionString += std::to_string(usedCoreCount);
        return optionString;
    }

    return "";
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CCaseStudyExeDlg dialog




CCaseStudyExeDlg::CCaseStudyExeDlg(const std::string &theSimpleCommandLineStr, CWnd* pParent)
	: CDialog(CCaseStudyExeDlg::IDD, pParent)
	,itsSimpleCommandLineStr(theSimpleCommandLineStr)
	,itsMetaFileName()
	,itsZipExe()
    ,itsCaseStudySystem()
{
	std::vector<std::string> commandParts = NFmiStringTools::Split(itsSimpleCommandLineStr, "?");
	if(commandParts.size() >= 1)
		itsMetaFileName = commandParts[0];
	if(commandParts.size() >= 2)
		itsZipExe = commandParts[1];

	m_hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_3, ColorPOD(160, 160, 164));
}

void CCaseStudyExeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCaseStudyExeDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CCaseStudyExeDlg message handlers

BOOL CCaseStudyExeDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    DoResizerHooking();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
        CString strAboutMenuU_;
        bNameValid = strAboutMenuU_.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
        if(!strAboutMenuU_.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenuU_);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CWnd *win = GetCopyDialogPositionWindow();
	if(win)
	{ // siirret��n t�m� static-ikkuna piilossa dialogin alapuolelle, t�h�n n�kym�tt�m��n ikkunaan laitetaan SHFileOperation -dialogi, jotta se olisi CaseStud-dialogin alapuolella
		CRect rect;
		win->GetWindowRect(&rect);
		rect.top += 50;
		win->MoveWindow(rect, FALSE);
	}

	itsStartOperationTimer = static_cast<UINT>(SetTimer(kFmiStartOperationTimer, 50, NULL)); // k�ynnistet��n timer, ett� saadaan pikkuisen viiveen j�lkeen k�ynnistetty� Case Studa datan keruu operaatio

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCaseStudyExeDlg::DoResizerHooking(void)
{
    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDOK, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_THIS_IS_WHERE_FILE_COPY_DIALOG_APPEARS, ANCHOR_BOTTOM | ANCHOR_LEFT);
    ASSERT(bOk == TRUE);
    bOk = m_resizer.SetAnchor(IDC_STATIC_MAIN_MESSAGE_STR, ANCHOR_HORIZONTALLY | ANCHOR_VERTICALLY);
    ASSERT(bOk == TRUE);
}

CWnd* CCaseStudyExeDlg::GetCopyDialogPositionWindow(void)
{
	CWnd *win = GetDlgItem(IDC_STATIC_THIS_IS_WHERE_FILE_COPY_DIALOG_APPEARS);
	return win;
}

void CCaseStudyExeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCaseStudyExeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCaseStudyExeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCaseStudyExeDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case kFmiStartOperationTimer:
		{
			KillTimer(itsStartOperationTimer);
			DoCaseDataOperation();
			return ;
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CCaseStudyExeDlg::DoCaseDataOperation(void)
{
    bool operationCanceled = false;
    std::string errorStr;
    std::string pathStr = NFmiFileSystem::PathFromPattern(itsMetaFileName);
    try
    {
        // 1. Kopioidaan data-tiedostot haluttuun hakemisto rakenteeseen
        itsCaseStudySystem.MakeCaseStudyData(itsMetaFileName, this, GetCopyDialogPositionWindow());

        // 2. Zippaa datapaketti
        if(!itsZipExe.empty())
        {
            // 2. Zippaa datapaketti
            //	esim: "D:\smartmet\MetEditor_5_8\utils\7z a -xr!*.zip -y -r -tzip -mmt3 -mx5 Case1.zip Case1*"
            WORD showWindow = IsIconic() ? SW_MINIMIZE : SW_SHOW;
            std::string commandStr;
            commandStr += itsZipExe;
            commandStr += " a";  // arkistoidaan dataa
            commandStr += " -xr!*.zip"; // excluudataan *.zip tiedostot (jos tekee uuden tallennuksen samaan paikkaa samalla nimell�, menee edellisell� kerralla luotu zip-tiedosto muuten mukaan)
            commandStr += " -xr!*.7z"; // excluudataan *.7z tiedostot (jos tekee uuden tallennuksen samaan paikkaa samalla nimell�, menee edellisell� kerralla luotu zip-tiedosto muuten mukaan)
            commandStr += " -y";  // ohita kaikki kysymykset yes-vastauksella
            commandStr += " -r";  // tee rekursiivinen tiedosto tallennus
            // Oletus tallennus formaatti on LZMA2 joka on nopea, joten k�ytet��n sit� ja unohdetaan hidas zip jupina
//            commandStr += " -tzip";  // tee tallennus zip-formaatissa
            commandStr += ::GetNumberOfCpuThreadsFor7zipOption();  // multi-core pakkaus, n. 1/3 coreista otetaan k�ytt��n esim. -mmt3
            commandStr += " -mx3";  // tallennuksen pakkaus taso
                                    // 0 = ei pakkausta
                                    // 1 = low
                                    // 3 = fast
                                    // 5 = normaali
                                    // 7 = max
                                    // 9 = ultra
            commandStr += " \"";  // laitetaan lainausmerkit metadatatiedoston polun ymp�rille, jos siin� sattuisi olemaan spaceja
            commandStr += pathStr + itsCaseStudySystem.Name();
            commandStr += ".7z";
            commandStr += "\" \""; // laitetaan lainausmerkit metadatatiedoston polun ymp�rille, jos siin� sattuisi olemaan spaceja
            commandStr += pathStr + itsCaseStudySystem.Name();
            commandStr += "*\""; // laitetaan lainausmerkit metadatatiedoston polun ymp�rille, jos siin� sattuisi olemaan spaceja
            if(CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr, true, true, showWindow, true, BELOW_NORMAL_PRIORITY_CLASS))
            {
                DoSuccessReport();
                return;
            }
            else
            {
                std::string errorStr = "Could not execute the system call";
                errorStr += ":\n";
                errorStr += commandStr;

                std::string titleStr(::GetDictionaryString("Compression operation failed"));
                std::string messageStr = errorStr;
                messageStr += "\n\n";
                messageStr += "You can now close this dialog";
                BringDialogUpFront(titleStr, messageStr);
                return;
            }
        }
        else
        {
            DoSuccessReport();
            return ;
        }
    }
    catch(CaseStudyOperationCanceledException& /* e */)
    {
        operationCanceled = true;
    }
    catch(std::exception& e)
    {
        errorStr = "Error in CCaseStudyExeDlg::DoCaseDataOperation:\n";
        errorStr += e.what();
    }
    catch(...)
    {
        errorStr = "Unknown error in CCaseStudyExeDlg::DoCaseDataOperation";
    }
    std::string titleStr(::GetDictionaryString("Error while storing CaseStudy-data"));

    std::string messageStr;
    if(operationCanceled)
    {
        titleStr = ::GetDictionaryString("CaseStudy-data operation was canceled");
        messageStr = ::GetDictionaryString("The CaseStudy-data operation was canceled by the user.");
        messageStr += "\n";
        messageStr += ::GetDictionaryString("You have to delete generated files by yourself from the directory");
        messageStr += ":\n";
        messageStr += pathStr;
        messageStr += "\n";
        messageStr += ::GetDictionaryString("You can now close this dialog");
    }
    else
    {
        titleStr = ::GetDictionaryString("Error while storing CaseStudy-data");
        messageStr = (::GetDictionaryString("Error while doing CaseStudy-data"));
        messageStr += "\n";
        messageStr += errorStr;
        messageStr += "\n";
        messageStr += ::GetDictionaryString("You can now close this dialog");
    }

    BringDialogUpFront(titleStr, messageStr);
}

void CCaseStudyExeDlg::DoSuccessReport()
{
    std::string titleStr(::GetDictionaryString("CaseStudy-data complete"));
    std::string messageStr(::GetDictionaryString("CaseStudy-data is now complete"));
    messageStr += "\n\n";
    messageStr += "You can now close this dialog";
    BringDialogUpFront(titleStr, messageStr);
}

void CCaseStudyExeDlg::BringDialogUpFront(const std::string &theTitleStr, const std::string &theMessageStr)
{
    SetWindowText(CA2T(theTitleStr.c_str()));
	CWnd *win = GetDlgItem(IDC_STATIC_MAIN_MESSAGE_STR);
	if(win)
        win->SetWindowText(CA2T(theMessageStr.c_str()));

    ShowWindow(SW_SHOWNORMAL);
}
