#pragma once

#include "SmartMetDialogs_resource.h"
#include "GridCtrl.h"
#include "NFmiMetTime.h"
#include "afxwin.h"
#include "NFmiViewPosRegistryInfo.h"
#include "HakeMessage/HakeMsg.h"
#include "HakeMessage/KahaMsg.h"

class SmartMetDocumentInterface;

struct WarningCenterHeaderParInfo
{
	enum ColumnFunction
	{
		kMessage=1,
		kMessageNumber,
		kAdditionalInfo,
		kStartTime,
		kReportTime,
		kRowNumber,
		kCenterId,
		kStationName,
		kMessageType,
		kMessageLevel,
		kCountyName,
		kCityName,
		kStreetName,
		kLon,
		kLat,
		kReason
	};

	WarningCenterHeaderParInfo(void)
	:itsHeader()
	,itsColumnFunction(kMessage)
	,itsColumnWidth(0)
	{}

	WarningCenterHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
	:itsHeader(theHeader)
	,itsColumnFunction(theColumnFunction)
	,itsColumnWidth(theColumnWidth)
	{}

	std::string itsHeader;
	ColumnFunction itsColumnFunction;
	int itsColumnWidth;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiWarningCenterGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiWarningCenterGridCtrl)

public:
    NFmiWarningCenterGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	,itsSmartMetDocumentInterface(nullptr)
	,itsLastSortedCell()
	,fLastSortedAscending(FALSE)
	,fLastSortedExist(false)
	,itsLastDataTime()
	,itsUsedHeaders(nullptr)
	,itsLastMessageCount(0)
	,fEnableFixedColumnSelection(true)
	,fUpdateParent(false)
	{}

	void SetDocument(SmartMetDocumentInterface *smartMetDocumentInterface){itsSmartMetDocumentInterface = smartMetDocumentInterface;};
	void SetUsedHeaders(std::vector<WarningCenterHeaderParInfo> *theUsedHeaders){itsUsedHeaders = theUsedHeaders;};

	void LastDataTime(const NFmiMetTime &theTime)
	{ itsLastDataTime = theTime;}
	void LastMessageCount(int newValue)
	{ itsLastMessageCount = newValue;}

	static int CALLBACK pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort);
	void DoLastSort(void);
	bool UpdateParent(void) {return fUpdateParent;}
	void UpdateParent(bool newValue) {fUpdateParent = newValue;}
protected:
    void OnFixedRowClick(CCellID& cell);
private:
    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista/tuhoa
	// v‰lill‰ pit‰‰ tehd‰ viimeksi tehty sorttaus uudestaan, t‰ss‰ on tiedot
	// viimeksi tehdyst‰ sortista
	CCellID itsLastSortedCell; // mille solulle se tehtiin
	BOOL fLastSortedAscending; // mik‰ oli sottauksen suunta
	bool fLastSortedExist; // onko edes tehty mit‰‰n sorttausta

	NFmiMetTime itsLastDataTime; // k‰ytet‰‰n printtauksessa otsikossa
	std::vector<WarningCenterHeaderParInfo> *itsUsedHeaders; // ei omista/tuhoa
	int itsLastMessageCount; // kuinka monta sanomaa/asemaa on taulukossa
	bool fEnableFixedColumnSelection; // t‰m‰ on pikku kikka mill‰ estet‰‰n valittujen ruutujen sekoittuminen kun tehd‰‰n DoLastSort
	bool fUpdateParent;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


// CFmiWarningCenterDlg dialog

class CFmiWarningCenterDlg : public CDialog
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiWarningCenterDlg)

public:
	CFmiWarningCenterDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiWarningCenterDlg();
	void Update(void);
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    void LoadViewMacroSettingsFromDocument();

// Dialog Data
	enum { IDD = IDD_DIALOG_WARNING_CENTER_DLG };
private:
	CRect CalcOtherArea(void);
	CRect CalcClientArea(void);
	void InitHeaders(void);
	void FillGridWithWarningMessages(bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount, const NFmiMetTime &theTime, int totalMessageCount);
	int CountShownMessages(void);
    void GetShownMessages();
    void FillGridWithWarningMessages(const std::vector<HakeMessage::HakeMsg> &messages, bool isHakeMessage, bool &fFirstTime, const NFmiMetTime &theTime, int &currentRowCounter);
    void ForceMainMapViewUpdate(const std::string &reasonForUpdate);

	NFmiWarningCenterGridCtrl itsGridCtrl;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	bool fGridCtrlInitialized;

	// T‰h‰n laitetaan valittu aika ja p‰iv‰m‰‰r‰
    CString itsTimeStrU_;
	CFont itsTimeAndStationTextFont;
	int itsMapViewDescTopIndex;

	std::vector<WarningCenterHeaderParInfo> itsHeaders;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	virtual void OnCancel();
	void DoWhenClosing(void);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnBnClickedButtonPreviousTime();
	afx_msg void OnBnClickedButtonNextTime();
	afx_msg void OnBnClickedButtonWarningCenterOptions();
    afx_msg void OnBnClickedCheckShowAllMessages();
    afx_msg void OnBnClickedButtonWarningCenterRefreshMessages();
    afx_msg void OnBnClickedCheckShowHakeMessages();
    afx_msg void OnBnClickedCheckShowKahaMessages();
    afx_msg void OnEnChangeEditMinimumTimeStepInMinutes();
private:
	BOOL fShowAllMessages;
    BOOL fShowHakeMessages;
    BOOL fShowKaHaMessages;
    std::vector<HakeMessage::HakeMsg> itsShownHakeMessages;
    std::vector<HakeMessage::KahaMsg> itsShownKaHaMessages;
    int itsMinimumTimeRangeForWarningsOnMapViewsInMinutes;
};
