#pragma once

#include <memory>
#include <vector>

class NFmiMenuItemList;
class CPoint;
class CMenu;

class CFmiPopupMenu
{

public:
	CFmiPopupMenu(void);
	virtual ~CFmiPopupMenu(void);
	void Init(NFmiMenuItemList* menuItemList);
	void Run(const CPoint &point);
	CMenu* Popup(void);

private:
    // Here we build the real popup menu
    std::unique_ptr<CMenu> realPopupMenu;
    // All the sub menus must be stored here
    std::vector<std::unique_ptr<CMenu>> subMenuContainer;
};

