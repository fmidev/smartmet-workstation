#pragma once

#include "SmartMetDialogs_resource.h"
#include "GridCtrl.h"
#include "NFmiMetTime.h"
#include "NFmiDataMatrix.h"
#include "NFmiViewPosRegistryInfo.h"

class SmartMetDocumentInterface;

struct SeaIcingWarningsHeaderParInfo
{
	enum ColumnFunction
	{
		kMessage=1,
		kMessageNumber,
		kSpray,
		kReportTime,
		kRowNumber,
		kShipId,
		kIntensity1,
		kIntensity2,
		kVisibility,
		kWindSpeed,
		kTempWater,
		kTempAir,
		kWaves,
		kIceSpeed,
		kLon,
		kLat
	};

	SeaIcingWarningsHeaderParInfo(void)
	:itsHeader()
	,itsColumnFunction(kMessage)
	,itsColumnWidth(0)
	{}

	SeaIcingWarningsHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
	:itsHeader(theHeader)
	,itsColumnFunction(theColumnFunction)
	,itsColumnWidth(theColumnWidth)
	{}

	std::string itsHeader;
	ColumnFunction itsColumnFunction;
	int itsColumnWidth;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiSeaIcingWarningsGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiSeaIcingWarningsGridCtrl)

public:
    NFmiSeaIcingWarningsGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	,itsSmartMetDocumentInterface(nullptr)
	,itsLastSortedCell()
	,fLastSortedAscending(FALSE)
	,fLastSortedExist(false)
	,itsLastDataTime()
	,itsUsedHeaders(nullptr)
	,itsLastMessageCount(0)
	{}

	void SetDocument(SmartMetDocumentInterface *smartMetDocumentInterface){itsSmartMetDocumentInterface = smartMetDocumentInterface;}
	void SetUsedHeaders(checkedVector<SeaIcingWarningsHeaderParInfo> *theUsedHeaders){itsUsedHeaders = theUsedHeaders;}

	void LastDataTime(const NFmiMetTime &theTime)
	{ itsLastDataTime = theTime;}
	void LastMessageCount(int newValue)
	{ itsLastMessageCount = newValue;}

	static int CALLBACK pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort);

private:
    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista/tuhoa
	// v‰lill‰ pit‰‰ tehd‰ viimeksi tehty sorttaus uudestaan, t‰ss‰ on tiedot
	// viimeksi tehdyst‰ sortista
	CCellID itsLastSortedCell; // mille solulle se tehtiin
	BOOL fLastSortedAscending; // mik‰ oli sorttauksen suunta
	bool fLastSortedExist; // onko edes tehty mit‰‰n sorttausta

	NFmiMetTime itsLastDataTime; // k‰ytet‰‰n printtauksessa otsikossa
	checkedVector<SeaIcingWarningsHeaderParInfo> *itsUsedHeaders; // ei omista/tuhoa
	int itsLastMessageCount; // kuinka monta sanomaa/asemaa on taulukossa
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


// CFmiSeaIcingWarningsDlg dialog

class CFmiSeaIcingWarningsDlg : public CDialog
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiSeaIcingWarningsDlg)

public:
	CFmiSeaIcingWarningsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiSeaIcingWarningsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WARNING_CENTER_DLG };

	void Update(void);
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	virtual void OnCancel();
	void DoWhenClosing(void);
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

public:
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnBnClickedButtonPreviousTime();
	afx_msg void OnBnClickedButtonNextTime();
	afx_msg void OnBnClickedButtonWarningCenterOptions();
	afx_msg void OnBnClickedButtonWarningCenterRefreshMessages();
	afx_msg void OnBnClickedCheckShowAllMessages();
private:
	CRect CalcOtherArea(void);
	CRect CalcClientArea(void);
	void InitHeaders(void);
	void FillGridWithWarningMessages(int rowCount, bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, int theStationCount);
	int CountShownMessages(void);

	NFmiSeaIcingWarningsGridCtrl itsGridCtrl;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	bool fGridCtrlInitialized;

	// T‰h‰n laitetaan valittu aika ja p‰iv‰m‰‰r‰
    CString itsTimeStrU_;
	CFont itsTimeAndStationTextFont;
	int itsMapViewDescTopIndex;

	checkedVector<SeaIcingWarningsHeaderParInfo> itsHeaders;
};
