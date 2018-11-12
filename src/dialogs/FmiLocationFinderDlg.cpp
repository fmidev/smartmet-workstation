// FmiLocationFinderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiLocationFinderDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiAutoComplete.h"
#include "NFmiQ2Client.h"
#include "boost/shared_ptr.hpp"
#include "afxmt.h"
#include "NFmiApplicationWinRegistry.h"


namespace
{
	class StopFunctor
	{
	public:
		StopFunctor(void):fStop(false){}

		bool Stop(void) const {return fStop;} // t‰t‰ virtuaali funktiota kutsutaan tyˆ luupin sis‰ll‰
		void Stop(bool newValue){fStop = newValue;}

	private:
		bool fStop;
	};

	class AutoCompleteThreadData
	{
	public:
		AutoCompleteThreadData(void)
		:itsQ2Client()
		,itsAutoComplete()
		,itsSearchStr()
		,itsStopFunctor()
		,fLogEvents(false)
		{
		}

		NFmiQ2Client itsQ2Client; // t‰t‰ k‰ytet‰‰n http-kutsujen tekoon
		NFmiAutoComplete itsAutoComplete; 
		std::string itsSearchStr;
		StopFunctor itsStopFunctor;
		bool fLogEvents;
	};

	class ResultDataHandler
	{
	public:
		ResultDataHandler(void)
			:itsResults()
			,itsDataSemaphore()
			,itsWaitTimeInMS(20)
		{}

		bool SetData(const std::vector<NFmiACLocationInfo> &theResultsIn)
		{
			CSingleLock singleLock(&itsDataSemaphore);
			if(singleLock.Lock(itsWaitTimeInMS))
			{
				itsResults = theResultsIn;
				return true;
			}
			else
				return false;
		}

		bool GetData(std::vector<NFmiACLocationInfo> &theResultsOut)
		{
			CSingleLock singleLock(&itsDataSemaphore);
			if(singleLock.Lock(itsWaitTimeInMS))
			{
				theResultsOut = itsResults;
				return true;
			}
			else
				return false;
		}

	private:
		std::vector<NFmiACLocationInfo> itsResults;
		CSemaphore itsDataSemaphore;
		int itsWaitTimeInMS;
	} gResultDataHandler;

	HANDLE gThreadEndingEvent;
}

UINT DoAutoCompleteThread(LPVOID pParam)
{
	if(pParam == 0)
		throw std::runtime_error("Error in DoAutoCompleteThread - Given AutoCompleteThreadData object was 0-pointer.");

	boost::shared_ptr<AutoCompleteThreadData> acThreadDataPtr(*(reinterpret_cast<boost::shared_ptr<AutoCompleteThreadData> *>(pParam)));
	bool status = true;
	std::vector<NFmiACLocationInfo> results;
	try
	{
		results = acThreadDataPtr->itsAutoComplete.DoAutoComplete(acThreadDataPtr->itsQ2Client, acThreadDataPtr->itsSearchStr, acThreadDataPtr->fLogEvents);
	}
	catch(...)
	{
		// virhe tilanteessa ei en‰‰ tehd‰ mit‰‰n
		status = false;
	}

	if(status && acThreadDataPtr->itsStopFunctor.Stop() == false)
		gResultDataHandler.SetData(results);

	SetEvent(gThreadEndingEvent);

	return 0;
}

// Paluta true, jos haku onnistui, ja theResultOut-parametri on k‰ytett‰viss‰.
// Palauta false, jos thready ei paluttanut tarpeeksi nopeasti, theResultOut-parametri ei ole k‰ytett‰viss‰.
static bool DoAutoComplete(SmartMetDocumentInterface *smartMetDocumentInterface, const std::string &theSearchStr, std::vector<NFmiACLocationInfo> &theResultOut)
{
	try
	{
		boost::shared_ptr<AutoCompleteThreadData> autoCompleteThreadDataPtr(new AutoCompleteThreadData());
		autoCompleteThreadDataPtr->itsAutoComplete = smartMetDocumentInterface->AutoComplete();
		int threadWaitTimeInMS = smartMetDocumentInterface->ApplicationWinRegistry().LocationFinderThreadTimeOutInMS(); // t‰m‰n verran odotetaan ett‰ haussa saa kestaa, jos menee pitemp‰‰n, j‰tet‰‰n thready oman onnensa nojaan ja jatketaan el‰m‰‰...
		autoCompleteThreadDataPtr->itsSearchStr = theSearchStr;
		autoCompleteThreadDataPtr->itsStopFunctor.Stop(false);
		autoCompleteThreadDataPtr->fLogEvents = false;

		gThreadEndingEvent = CreateEvent(NULL, FALSE, 0, _TEXT("Test"));
		CWinThread *doAutoCompleteThread = AfxBeginThread(DoAutoCompleteThread, &autoCompleteThreadDataPtr, THREAD_PRIORITY_NORMAL);
		DWORD status = WaitForSingleObject(gThreadEndingEvent, threadWaitTimeInMS);
		CloseHandle(gThreadEndingEvent);
		if(status == WAIT_OBJECT_0)
		{
			// homma ok, thready lopetti, ota data jostain ja palauta ok koodin kera
			return gResultDataHandler.GetData(theResultOut);
		}
		else
		{
			// 1. thready ei lopettanut m‰‰r‰tyss‰ ajassa, laita sen stop-funktoriin tieto ett‰ vastausta ei en‰‰ odoteta,
			autoCompleteThreadDataPtr->itsStopFunctor.Stop(true);
			// 2. palautetaan virhe koodi, jotta kyselij‰ ei ota vastaus-dataa k‰yttˆˆn
			return false;
		}
	}
	catch(...)
	{
	}

	return false;
}



// CFmiLocationFinderDlg dialog

IMPLEMENT_DYNAMIC(CFmiLocationFinderDlg, CDialog)

CFmiLocationFinderDlg::CFmiLocationFinderDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiLocationFinderDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,fReplaceStoredWordOnClose(false)
    , itsfReplaceStoredWordOnCloseStrU_()
	,itsfReplaceStoredWordOnCloseSelection(0)
{

}

CFmiLocationFinderDlg::~CFmiLocationFinderDlg()
{
}

BOOL CFmiLocationFinderDlg::Create(void) // modaalittomaa varten
{
	return CDialog::Create(CFmiLocationFinderDlg::IDD);
}

void CFmiLocationFinderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LOCATION_FINDER, itsLocationFinderComboBox);
}


BEGIN_MESSAGE_MAP(CFmiLocationFinderDlg, CDialog)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_LOCATION_FINDER, &CFmiLocationFinderDlg::OnCbnSelchangeComboLocationFinder)
	ON_CBN_EDITCHANGE(IDC_COMBO_LOCATION_FINDER, &CFmiLocationFinderDlg::OnCbnEditchangeComboLocationFinder)
	ON_CBN_CLOSEUP(IDC_COMBO_LOCATION_FINDER, &CFmiLocationFinderDlg::OnCbnCloseupComboLocationFinder)
	ON_CBN_SELENDCANCEL(IDC_COMBO_LOCATION_FINDER, &CFmiLocationFinderDlg::OnCbnSelendcancelComboLocationFinder)
END_MESSAGE_MAP()


void CFmiLocationFinderDlg::DoWhenClosing(void)
{
	itsSmartMetDocumentInterface->AutoComplete().AutoCompleteDialogOn(false);
    itsSmartMetDocumentInterface->InvalidateMapView(false);
}

// CFmiLocationFinderDlg message handlers

void CFmiLocationFinderDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiLocationFinderDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiLocationFinderDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

BOOL CFmiLocationFinderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// siirret‰‰n aluksi dialogi p‰‰karttan‰ytˆn alareunaan
    SetPlaceToParentsBottomRightCorner(AfxGetMainWnd());
	SetErrorStr(std::string("")); // nollataan mahd. virhe teksti


	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiLocationFinderDlg::SetPlaceToParentsBottomRightCorner(CWnd *parentView)
{
    CRect mainWinPlace;
    parentView->GetWindowRect(mainWinPlace);
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    // gets current window position
    BOOL bRet = GetWindowPlacement(&wndpl);
    CRect oldRect(wndpl.rcNormalPosition);
    MoveWindow(mainWinPlace.right - oldRect.Width() - 10,
        mainWinPlace.bottom - oldRect.Height() - 5,
        oldRect.Width(), oldRect.Height()); // HUOM! dialogin kokoa ei saa muuttaa!!
}

void CFmiLocationFinderDlg::OnCbnSelchangeComboLocationFinder()
{
	UpdateData(TRUE);
}

// Haluan tyhjent‰‰ CComboBox:in listan, mutta esim. ResetContent-metodi tyhjent‰‰ myˆs edit-boxin
// ja t‰llˆin menet‰ kursorin paikan. Eli tein oman funktion, joka vain tyhjent‰‰ comboboxin listan
// mutta ei koske mitenk‰‰n sen edit-boxiin.
static void ClearComboList(CComboBox &theComboBox)
{
	// Delete every item from the combo box.
	for (int i = theComboBox.GetCount() - 1; i >= 0; i--)
	{
	   theComboBox.DeleteString(i);
	}
}

void CFmiLocationFinderDlg::OnCbnEditchangeComboLocationFinder()
{
	UpdateData(TRUE);

	// Edit-boxista pit‰‰ ottaa kaikki tiedot talteen ett‰ lopuksi ne voidaan palauttaa, koska muuten
	// paikannimen muokkaus ei toimi intuiivisesti. 
	// Mm. ShowDropDown (jolla lista pudotetaan n‰kyviin) tuhoaa kaiken halutun toiminnallisuuden ja piilottaa vie hiiren kursiorin
    CString editStrU_;
    itsLocationFinderComboBox.GetWindowText(editStrU_);
	DWORD editSel = itsLocationFinderComboBox.GetEditSel();
	itsfReplaceStoredWordOnCloseSelection = editSel;
	short startPos = static_cast<short>(editSel);
	short endPos = static_cast<short>(editSel >> 16);
	int selected = itsLocationFinderComboBox.GetCurSel();

	std::vector<NFmiACLocationInfo> locationInfos;
    std::string tmp = CT2A(editStrU_);
	if(::DoAutoComplete(itsSmartMetDocumentInterface, tmp, locationInfos))
	{
        itsSmartMetDocumentInterface->AutoComplete().AutoCompleteResults(locationInfos);
	//	itsLocationFinderComboBox.ResetContent(); // Ei voi k‰ytt‰‰ ResetContent:ia, koska tyhjent‰‰ sek‰ listan ett‰ edit-controllin ja tuhoaa mm. back space editoinnin
		::ClearComboList(itsLocationFinderComboBox);
		if(locationInfos.size())
		{
			// t‰ytet‰‰n drolista lˆydetyill‰ paikannimill‰
			for(size_t i=0; i<locationInfos.size(); i++)
			{
				std::string locatInfo = locationInfos[i].itsName;
				itsLocationFinderComboBox.AddString(CA2T(locatInfo.c_str()));
			}

			// laitetaan lista n‰kyviin
			itsLocationFinderComboBox.ShowDropDown(true); // t‰m‰ jostain syyst‰ piilottaa hiiren kursorin?!?!?
			SetCursor(LoadCursor(NULL, IDC_ARROW)); // joten t‰t‰ on pakko kutsua ShowDropDown-kutsun j‰lkeen, ett‰ kursori n‰kyisi
		}
		// palautetaan teksti ja kursorin paikka editboxiin 
        itsLocationFinderComboBox.SetWindowText(editStrU_);
		itsLocationFinderComboBox.SetEditSel(startPos, endPos);
		SetErrorStr(std::string("")); // nollataan mahd. virhe teksti
	}
	else
	{
		static int errorCounter = 0;
		// error teksti‰ kuntoon
		errorCounter++;
		std::string errorStr("DB search failed (");
		errorStr += NFmiStringTools::Convert(errorCounter);
		errorStr += ")";
		SetErrorStr(errorStr);
	}
    itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(0, true, true);
}

void CFmiLocationFinderDlg::SetErrorStr(std::string &theErrorStr)
{
	CWnd *errWin = GetDlgItem(IDC_STATIC_AUTO_COMPLETE_ERROR_STR);
	if(errWin)
		errWin->SetWindowText(CA2T(theErrorStr.c_str()));
}

void CFmiLocationFinderDlg::OnCbnCloseupComboLocationFinder()
{
	if(fReplaceStoredWordOnClose)
	{
		fReplaceStoredWordOnClose = false;
        itsLocationFinderComboBox.SetWindowText(itsfReplaceStoredWordOnCloseStrU_);
		short startPos = static_cast<short>(itsfReplaceStoredWordOnCloseSelection);
		short endPos = static_cast<short>(itsfReplaceStoredWordOnCloseSelection >> 16);
		itsLocationFinderComboBox.SetEditSel(startPos, endPos);
	}

	UpdateData(FALSE);
}

void CFmiLocationFinderDlg::OnCbnSelendcancelComboLocationFinder()
{
	UpdateData(TRUE);

    CString editStrU_;
    itsLocationFinderComboBox.GetWindowText(editStrU_);

	fReplaceStoredWordOnClose = true;
    itsfReplaceStoredWordOnCloseStrU_ = editStrU_;
}

void CFmiLocationFinderDlg::ActivateDialog(CWnd *parentView)
{
    SetPlaceToParentsBottomRightCorner(parentView);
    ShowWindow(SW_RESTORE);
    SetActiveWindow();
    itsLocationFinderComboBox.SetFocus();
}
