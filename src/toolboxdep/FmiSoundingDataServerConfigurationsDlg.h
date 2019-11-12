#pragma once

#include "SmartMetToolboxDep_resource.h"
#include "GridCtrl.h"
#include <vector>
#include "NFmiViewPosRegistryInfo.h"
#include "WndResizer.h"

class SmartMetDocumentInterface;
class ModelDataServerConfiguration;
class CGridCellCheck;

struct SoundingConfHeaderParInfo
{
	enum ColumnFunction
	{
		kRowNumber = 0,
		kModelName,
        kModerProducerId,
		kDataNameOnServer
	};

    SoundingConfHeaderParInfo(void)
	:itsHeader()
	,itsColumnFunction(kModelName)
	,itsColumnWidth(0)
	{}

    SoundingConfHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
	:itsHeader(theHeader)
	,itsColumnFunction(theColumnFunction)
	,itsColumnWidth(theColumnWidth)
	{}

	std::string itsHeader;
	ColumnFunction itsColumnFunction;
	int itsColumnWidth;
};

// pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiSoundingConfGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiSoundingConfGridCtrl)

public:
    NFmiSoundingConfGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
	:CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
	{}

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

// CFmiSoundingDataServerConfigurationsDlg dialog

class CFmiSoundingDataServerConfigurationsDlg : public CDialogEx
{
private: // n‰ill‰ talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

    DECLARE_DYNAMIC(CFmiSoundingDataServerConfigurationsDlg)

public:
    CFmiSoundingDataServerConfigurationsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiSoundingDataServerConfigurationsDlg();
	void SetDefaultValues(void);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}
    void Update();

// Dialog Data
	enum { IDD = IDD_DIALOG_SOUNDING_DATA_SERVER_CONFIGURATIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void DoOnOk(void);
	void InitGridControlValues(void);
    void UpdateGridControlValues();
    void UpdateRows(int fixedRowCount, int fixedColumnCount, bool updateOnly);
	void InitHeaders(void);
	void SetGridRow(int row, const ModelDataServerConfiguration &theSoundingConf, int theFixedColumnCount, bool updateOnly);
    void GetProducerIdFromGridCtrlCell(ModelDataServerConfiguration &modelConfiguration, int row, int column);
    void GetDataNameOnServerFromGridCtrlCell(ModelDataServerConfiguration &modelConfiguration, int row, int column);
    void GetModelConfigurationFromGridCtrlCell(ModelDataServerConfiguration &modelConfiguration, int row, int column);
    void GetModelConfigurationsFromGridCtrlRow(ModelDataServerConfiguration &modelConfiguration, int row);
    void GetSettingsFromDialog();
    void DoResizerHooking();
    void InitDialogTexts();
    void FitLastColumnOnVisibleArea();
    void InitSelectedServerUrlSelector();

	NFmiSoundingConfGridCtrl itsGridCtrl;
	std::vector<SoundingConfHeaderParInfo> itsHeaders;
	bool fGridControlInitialized = false;
	bool fDialogInitialized = false; // t‰m‰ on OnSize -s‰‰tˆj‰ varten, 1. kerralla ei saa s‰‰t‰‰ ikkunoita, koska niit‰ ei ole viel‰ olemassa.

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
    CWndResizer m_resizer;
    CComboBox itsServerUrlSelector;
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnCancel();
	virtual void OnOK();
public:
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
//	afx_msg void OnPaint();
    afx_msg void OnBnClickedButtonApply();
    afx_msg void OnCbnSelchangeComboSelectedSoundingDataServer();
};
