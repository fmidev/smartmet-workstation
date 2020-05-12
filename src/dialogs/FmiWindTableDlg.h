#pragma once

// Tuulitaulukko dialogi
// 3.3.2009 Marko

#include "SmartMetDialogs_resource.h"
#include "NFmiMetTime.h"
#include "NFmiProducerSystem.h"
#include "NFmiDataMatrix.h"
#include "GridCtrl.h"
#include "boost/shared_ptr.hpp"
#include "NFmiViewPosRegistryInfo.h"

#include "afxwin.h"

class SmartMetDocumentInterface;
class NFmiGrid;
class NFmiFastQueryInfo;

struct WindTableHeaderParInfo
{
	enum ColumnFunction
	{
		kNoFunction=0,
		kMessage=1,
		kMessageNumber,
		kTime,
		kAreaName,
		kRowNumber,
		kWindSpeed,
		kWindDirection,
		kMaxWind,
		kLon,
		kLat
	};

	WindTableHeaderParInfo(void)
	:itsHeader()
	,itsColumnFunction(kMessage)
	,itsColumnWidth(0)
	{}

	WindTableHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
	:itsHeader(theHeader)
	,itsColumnFunction(theColumnFunction)
	,itsColumnWidth(theColumnWidth)
	{}

	std::string itsHeader;
	ColumnFunction itsColumnFunction;
	int itsColumnWidth;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiWindTableGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiWindTableGridCtrl)

public:
    NFmiWindTableGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	,itsSmartMetDocumentInterface(nullptr)
	,itsLastSortedCell()
	,fLastSortedAscending(FALSE)
	,fLastSortedExist(false)
	,itsLastDataTime()
	,itsUsedHeaders(nullptr)
	,itsLastMessageCount(0)
	{}

	void SetDocument(SmartMetDocumentInterface *smartMetDocumentInterface){itsSmartMetDocumentInterface = smartMetDocumentInterface;};
	void SetUsedHeaders(std::vector<WindTableHeaderParInfo> *theUsedHeaders){itsUsedHeaders = theUsedHeaders;};

	void LastDataTime(const NFmiMetTime &theTime)
	{ itsLastDataTime = theTime;}
	void LastMessageCount(int newValue)
	{ itsLastMessageCount = newValue;}

	static int CALLBACK pfnCellTextCaseInSensitiveCompare(LPARAM lParam1,
																LPARAM lParam2,
																LPARAM lParamSort);

private:
	bool IsFixedRow(const CCellID &theCell);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista/tuhoa
	// v‰lill‰ pit‰‰ tehd‰ viimeksi tehty sorttaus uudestaan, t‰ss‰ on tiedot
	// viimeksi tehdyst‰ sortista
	CCellID itsLastSortedCell; // mille solulle se tehtiin
	BOOL fLastSortedAscending; // mik‰ oli sorttauksen suunta
	bool fLastSortedExist; // onko edes tehty mit‰‰n sorttausta

	NFmiMetTime itsLastDataTime; // k‰ytet‰‰n printtauksessa otsikossa
	std::vector<WindTableHeaderParInfo> *itsUsedHeaders; // ei omista/tuhoa
	int itsLastMessageCount; // kuinka monta sanomaa/asemaa on taulukossa
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

// CFmiWindTableDlg dialog

class CFmiWindTableDlg : public CDialog
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

	DECLARE_DYNAMIC(CFmiWindTableDlg)

public:
	CFmiWindTableDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiWindTableDlg();
	void Update(void);
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}

// Dialog Data
	enum { IDD = IDD_DIALOG_WIND_TABLE };

protected:
	void DoWhenClosing(void);
	CRect CalcClientArea(void);
	CRect CalcOtherArea(void);
	void InitHeaders(void);
	void FillGridWithValues(bool &fFirstTime, int theFixedRowCount, int theFixedColumnCount);
	void UpdateProducerList(void);
	boost::shared_ptr<NFmiFastQueryInfo> GetSelectedInfo(void);
	NFmiMetTime GetWantedStartTime(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	NFmiWindTableGridCtrl itsGridCtrl;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	bool fGridCtrlInitialized;
	int itsMapViewDescTopIndex;
	std::vector<WindTableHeaderParInfo> itsHeaders;
	std::vector<NFmiProducerHelperInfo> itsProducerList;
	NFmiMetTime itsLastStartTime; // jos Update-metodissa t‰m‰ ei ole vaihtunut, ei p‰ivitet‰ n‰yttˆ‰ (t‰m‰ on optimointia)
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedButtonPrint();
private:
	CComboBox itsProducerSelector;
public:
	afx_msg void OnCbnSelchangeComboWindProducerSelector();
private:
	BOOL fUseMaptime;
public:
	afx_msg void OnBnClickedCheckWindTableMapTime();
private:
	CComboBox itsAreaFilterSelector;
public:
	afx_msg void OnCbnSelchangeComboWindAreaFilterSelector();
};
