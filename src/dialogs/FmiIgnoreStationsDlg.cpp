// FmiIgnoreStationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiIgnoreStationsDlg.h"
#include "afxdialogex.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiIgnoreStationsData.h"
#include "FmiInputValueDlg.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiSettings.h"
#include "NFmiFastQueryInfo.h"
#include "CloneBitmap.h"
#include "SmartMetToolboxDep_resource.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewFunctions.h"
#include "ApplicationInterface.h"
#include "FmiWin32Helpers.h"
// CFmiIgnoreStationsDlg dialog

IMPLEMENT_DYNAMIC(CFmiIgnoreStationsDlg, CDialogEx)

CFmiIgnoreStationsDlg::CFmiIgnoreStationsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
:CDialogEx(CFmiIgnoreStationsDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
, fUseWithSymbolDraw(FALSE)
, fUseWithContourDraw(FALSE)
{

}

CFmiIgnoreStationsDlg::~CFmiIgnoreStationsDlg()
{
}

void CFmiIgnoreStationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STATIONS, itsIgnoreStationList);
	DDX_Check(pDX, IDC_CHECK_USE_WITH_SYMBOLS, fUseWithSymbolDraw);
	DDX_Check(pDX, IDC_CHECK_USE_WITH_CONTOURS, fUseWithContourDraw);
}


BEGIN_MESSAGE_MAP(CFmiIgnoreStationsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CFmiIgnoreStationsDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CFmiIgnoreStationsDlg::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CFmiIgnoreStationsDlg::OnBnClickedButtonUpdate)
	ON_WM_CLOSE()
	ON_LBN_SELCHANGE(IDC_LIST_STATIONS, &CFmiIgnoreStationsDlg::OnLbnSelchangeListStations)
END_MESSAGE_MAP()


// CFmiIgnoreStationsDlg message handlers


BOOL CFmiIgnoreStationsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    std::string titleStr = ::GetDictionaryString("Ignore stations settings");
    SetWindowText(CA2T(titleStr.c_str()));
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	auto &ignoreStationsData = itsSmartMetDocumentInterface->IgnoreStationsData();
	fUseWithContourDraw = ignoreStationsData.UseListWithContourDraw();
	fUseWithSymbolDraw = ignoreStationsData.UseListWithSymbolDraw();
	UpdateStationList();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CFmiIgnoreStationsDlg::OnBnClickedButtonAdd()
{
	std::string helpStr = "Use one integer value or if range, use two values with '-' separator between them.\n I.e. \"12345\" or \"12345 - 13456\"";

    CFmiInputValueDlg dlg(CString(CA2T(helpStr.c_str())), this);
	if(dlg.DoModal() == IDOK)
	{
        std::string valueStr = CT2A(dlg.itsValueU_);
		NFmiIgnoreStation station;
		if(station.GetIdValues(valueStr) == false)
		{
			std::string errStr = "Unable to get station id or range from given input.";
			errStr += helpStr;
			std::string titleStr = "Error with given station id input";
            ::MessageBox(this->GetSafeHwnd(), CA2T(errStr.c_str()), CA2T(titleStr.c_str()), MB_OK | MB_ICONERROR);
			return ;
		}
		std::string stationName = "?";
		if(station.IsRange())
			stationName = "...";
		else
		{
			// 1. etsi asema id:n perusteellä asemaa havainto datoista
			auto infos = itsSmartMetDocumentInterface->InfoOrganizer()->GetInfos(NFmiInfoData::kObservations);
			for(size_t i = 0; i < infos.size(); i++)
			{
				if(infos[i]->IsGrid() == false)
				{
					if(infos[i]->Location(station.itsId))
					{
						stationName = static_cast<char*>(infos[i]->Location()->GetName());
						break;
					}
				}
			}
		}
		// 2. Laita asema tietoineen ignorelistaan
		station.fEnabled = true;
		station.itsName = stationName;
		itsSmartMetDocumentInterface->IgnoreStationsData().Add(station);
		// 3. päivitä itsIgnoreStationList-kontrolli
		UpdateStationList();
	}
}

static CString MakeStationString(const NFmiIgnoreStation &theStation)
{
    CString strU_(_TEXT(" ")); // laitetaan yksi space alkuun, koska muuten 1. numero/kirjain voi peittyä hieman tietyissä tapauksissa
    strU_ += CA2T(NFmiStringTools::Convert<unsigned long>(theStation.itsId).c_str());
	if(theStation.IsRange())
	{
		strU_ += _TEXT(" - ");
        strU_ += CA2T(NFmiStringTools::Convert<unsigned long>(theStation.itsId2).c_str());
	}
    strU_ += _TEXT(" (");
    strU_ += CA2T(theStation.itsName.c_str());
    strU_ += _TEXT(")");

    return strU_;
}

void CFmiIgnoreStationsDlg::UpdateStationList(void)
{
	itsIgnoreStationList.ResetContent();
	int counter = 0;
	std::list<NFmiIgnoreStation> &stationList = itsSmartMetDocumentInterface->IgnoreStationsData().StationList();
	for(std::list<NFmiIgnoreStation>::iterator it = stationList.begin(); it != stationList.end(); ++it)
	{
		itsIgnoreStationList.AddString(::MakeStationString(*it));
		itsIgnoreStationList.SetCheck(counter, (*it).fEnabled);
		counter++;
	}

	UpdateData(FALSE);
}

void CFmiIgnoreStationsDlg::OnBnClickedButtonRemove()
{
	int index = itsIgnoreStationList.GetCurSel();
	if(index != LB_ERR)
	{
		std::list<NFmiIgnoreStation> &stationList = itsSmartMetDocumentInterface->IgnoreStationsData().StationList();
		std::list<NFmiIgnoreStation>::iterator it = stationList.begin();
		std::advance(it, index);
		if(it != stationList.end())
		{
			stationList.erase(it);
			UpdateStationList();
		}
	}
}

void CFmiIgnoreStationsDlg::StoreSettingsToDoc(void)
{
	UpdateData(TRUE);
    auto &ignoreStationsData = itsSmartMetDocumentInterface->IgnoreStationsData();
    ignoreStationsData.UseListWithContourDraw(fUseWithContourDraw == TRUE);
	ignoreStationsData.UseListWithSymbolDraw(fUseWithSymbolDraw == TRUE);
	int counter = 0;
	std::list<NFmiIgnoreStation> &stationList = ignoreStationsData.StationList();
	for(std::list<NFmiIgnoreStation>::iterator it = stationList.begin(); it != stationList.end(); ++it)
	{
		(*it).fEnabled = itsIgnoreStationList.GetCheck(counter) == TRUE;
		counter++;
	}
}

void CFmiIgnoreStationsDlg::OnBnClickedButtonUpdate()
{
	StoreSettingsToDoc();
	itsSmartMetDocumentInterface->IgnoreStationsData().StoreToSettings();
	NFmiSettings::Save();
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
	itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("IgnoreStationsDlg: Update pressed", true, true, CtrlViewUtils::kDoAllMapViewDescTopIndex);
}


void CFmiIgnoreStationsDlg::OnOK()
{
	DoWhenClosing();
	CDialogEx::OnOK();
}


void CFmiIgnoreStationsDlg::OnCancel()
{
	DoWhenClosing();
	CDialogEx::OnCancel();
}


void CFmiIgnoreStationsDlg::OnClose()
{
	DoWhenClosing();
	CDialogEx::OnClose();
}

void CFmiIgnoreStationsDlg::DoWhenClosing(void)
{
	StoreSettingsToDoc();
	itsSmartMetDocumentInterface->IgnoreStationsData().IgnoreStationsDialogOn(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
}

void CFmiIgnoreStationsDlg::OnLbnSelchangeListStations()
{
	StoreSettingsToDoc();
}
