#include "CFmiPopupMenu.h"
#include "NFmiMenuItemList.h"

#include "stdafx.h"

CFmiPopupMenu::CFmiPopupMenu()
:realPopupMenu()
,subMenuContainer()
{
} 

CFmiPopupMenu::~CFmiPopupMenu() = default;

static void initPopupMenu(std::unique_ptr<CMenu> &popupMenu, NFmiMenuItemList *menuItemList, std::vector<std::unique_ptr<CMenu>> &subMenuContainer)
{
    for(auto &menuItem : *menuItemList)
    {
        auto *subMenuItemList = menuItem->SubMenu();
        if(subMenuItemList)
        {
            auto subMenu = std::make_unique<CMenu>();
            subMenu->CreatePopupMenu();
            popupMenu->AppendMenu(MF_BYPOSITION | MF_STRING | MF_POPUP, UINT_PTR(subMenu->GetSafeHmenu()), CA2T(menuItem->Text()));
            ::initPopupMenu(subMenu, subMenuItemList, subMenuContainer);
            subMenuContainer.emplace_back(std::move(subMenu));
        }
        else
        {
            popupMenu->AppendMenu(MF_BYPOSITION | MF_STRING, menuItem->CommandId(), CA2T(menuItem->Text()));
        }
    }
}

void CFmiPopupMenu::Init(NFmiMenuItemList* menuItemList)
{
	realPopupMenu = std::make_unique<CMenu>();
    realPopupMenu->CreatePopupMenu();
    ::initPopupMenu(realPopupMenu, menuItemList, subMenuContainer);
}

void CFmiPopupMenu::Run(const CPoint &point)
{
    if(realPopupMenu)
	    realPopupMenu->TrackPopupMenu(TPM_CENTERALIGN | TPM_RIGHTBUTTON, point.x, point.y, 
									AfxGetMainWnd());	
	return;
}

CMenu* CFmiPopupMenu::Popup(void)
{
    if(realPopupMenu)
        return realPopupMenu.get();
    return nullptr;
}

