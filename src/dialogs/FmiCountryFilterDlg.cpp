// FmiCountryFilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiCountryFilterDlg.h"
#include "FmiSynopDataGridViewDlg.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"

static const int gRowSize = 15;
static const int gColumnSize = 15;

IMPLEMENT_DYNCREATE(NFmiGridCtrl2, CGridCtrl)

BEGIN_MESSAGE_MAP(NFmiGridCtrl2, CGridCtrl)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void NFmiGridCtrl2::OnMouseMove(UINT nFlags, CPoint point)
{
	CGridCtrl::OnMouseMove(nFlags, point);

	// tämä laittaa lyhennyksen sijasta koko maan nimen näkyviin tooltippiin
	CCellID idCurrentCell = GetCellFromPt(point);
	if(idCurrentCell.row >= this->GetFixedRowCount() && idCurrentCell.row < this->GetRowCount())
	{
        if (m_MouseMode == MOUSE_NOTHING)
        {
            CGridCellBase* pCell = NULL;
            CCellID idCurrentCell;
            if (!GetVirtualMode() || m_bTitleTips)
            {
                // Let the cell know that a big fat cursor is currently hovering
                // over it.
                idCurrentCell = GetCellFromPt(point);
                pCell = GetCell(idCurrentCell.row, idCurrentCell.col);
//                if (pCell)
  //                  pCell->OnMouseOver();
            }
            if (m_bTitleTips)
            {
               CRect TextRect, CellRect;
               if (pCell)
                {
					std::string tipText = itsWmoIdFilterManager->GetCountryFilter(idCurrentCell.row, idCurrentCell.col).itsLongName;
                    if (!m_bRMouseButtonDown
						&& !tipText.empty()
                        && !pCell->IsEditing()
                        && GetCellRect( idCurrentCell.row, idCurrentCell.col, &TextRect)
                        && pCell->GetTipTextRect( &TextRect)
                        && GetCellRect(idCurrentCell.row, idCurrentCell.col, CellRect) )
                    {
                        m_TitleTip.Show(TextRect, CA2T(tipText.c_str()), 0, CellRect, pCell->GetFont(), GetTitleTipTextClr(), GetTitleTipBackClr());
					}
				}
			}
		}
	}
}


// CFmiCountryFilterDlg dialog

IMPLEMENT_DYNAMIC(CFmiCountryFilterDlg, CDialog)
CFmiCountryFilterDlg::CFmiCountryFilterDlg(WmoIdFilterManager *theWmoIdFilterManager, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiCountryFilterDlg::IDD, pParent)
	, fShowAll(FALSE)
	,itsWmoIdFilterManager(theWmoIdFilterManager)
{
}

CFmiCountryFilterDlg::~CFmiCountryFilterDlg()
{
}

void CFmiCountryFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_GridControl(pDX, IDC_CUSTOM_GRID_CTRL, itsGridCtrl);
	DDX_Check(pDX, IDC_CHECK_SHOW_ALL, fShowAll);
}


BEGIN_MESSAGE_MAP(CFmiCountryFilterDlg, CDialog)
END_MESSAGE_MAP()


// CFmiCountryFilterDlg message handlers

BOOL CFmiCountryFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	itsGridCtrl.SetWmoIdFilterManager(itsWmoIdFilterManager);
	itsGridCtrl.SetRowCount(gRowSize);
	itsGridCtrl.SetColumnCount(gColumnSize);
	itsGridCtrl.SetGridLines(GVL_BOTH);
	itsGridCtrl.ExpandColumnsToFit(TRUE);
	itsGridCtrl.ExpandRowsToFit(TRUE);
	itsGridCtrl.SetRowResize(TRUE);
	itsGridCtrl.SetColumnResize(TRUE);
//	itsGridCtrl.SetFixedColumnSelection(FALSE);
//	itsGridCtrl.SetFixedRowSelection(FALSE);

	int countrySize = itsWmoIdFilterManager->GetSize();
	int currentIndex = 0;
	for(int j=0; j<gRowSize; j++)
	{
		for(int i=0; i<gColumnSize; i++)
		{
			if(currentIndex < countrySize)
			{
				CountryFilter &cFilter = itsWmoIdFilterManager->GetCountryFilter(currentIndex);
				cFilter.itsRow = j;
				cFilter.itsColumn = i;
                itsGridCtrl.SetItemText(j, i, CA2T(cFilter.itsShortName.c_str()));
				itsGridCtrl.SetItemState(j, i, itsGridCtrl.GetItemState(j, i) | GVIS_READONLY);
				if(cFilter.fSelected)
	                itsGridCtrl.SetItemState(j, i, itsGridCtrl.GetItemState(j, i) | GVIS_SELECTED);
			}
			else
			{
				itsGridCtrl.SetItemState(j, i, itsGridCtrl.GetItemState(j, i) | GVIS_READONLY);
				itsGridCtrl.SetItemBkColour(j, i, RGB(239, 235, 222));
			}
			currentIndex++;
		}
	}

	this->fShowAll = itsWmoIdFilterManager->AllSelected();
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiCountryFilterDlg::OnOK()
{
	UpdateData(TRUE);
	itsWmoIdFilterManager->AllSelected(fShowAll == TRUE);
	int countrySize = itsWmoIdFilterManager->GetSize();
	int currentIndex = 0;
	for(int j=0; j<gRowSize; j++)
	{
		for(int i=0; i<gColumnSize; i++)
		{
			if(currentIndex < countrySize)
			{
				CountryFilter &cFilter = itsWmoIdFilterManager->GetCountryFilter(j, i);
				cFilter.fSelected = itsGridCtrl.IsCellSelected(CCellID(j, i)) == TRUE;
			}
			else
				break;
			currentIndex++;
		}
	}

	CDialog::OnOK();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiCountryFilterDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("CFmiCountryFilterDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_ALL, "IDC_CHECK_SHOW_ALL_COUNTRIES");
}
