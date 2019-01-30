//**********************************************************
// C++ Class Name : NFmiMenuItemList 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMenuItemList.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 2 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Markon ehdotus 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Jan 28, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// Changed 1999.08.24/Marko	Lisäsin parit konstruktorit, jotka lisäävät 
//							parametrimenuun myös eri levelit.
// 
//**********************************************************

#pragma once

#include "NFmiPtrList.h"
#include "FmiNMeteditLibraryDefinitions.h"
#include "NFmiMenuItem.h"

#include <list>
#include <memory>
#include <vector>

class NFmiParamBag;
class NFmiDrawParamList;
class NFmiLevelBag;
class NFmiDataIdent;

class NFmiMenuItemList 
{
 public:
    using MenuItem = std::unique_ptr<NFmiMenuItem>;
    using MenuItemList = std::list<MenuItem>;

	NFmiMenuItemList(void);
	NFmiMenuItemList(NFmiParamBag* theParamBag);
	NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag, const FmiMenuCommandType &theMenuCommandType,
					const NFmiMetEditorTypes::View &theViewType, NFmiInfoData::Type theDataType, const NFmiDataIdent *thePossibleStreamLineParam = nullptr, const std::vector<std::unique_ptr<NFmiDataIdent>> *possibleMetaParams = nullptr);
	NFmiMenuItemList(int theMapViewDescTopIndex, const NFmiDataIdent& theDataIdent, 
					const FmiMenuCommandType& theMenuCommandType, NFmiLevelBag* theLevels, NFmiInfoData::Type theDataType);

	NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag, const FmiMenuCommandType &theMenuCommandType,
						const NFmiMetEditorTypes::View &theViewType, NFmiLevelBag* theLevels, NFmiInfoData::Type theDataType, FmiParameterName notLevelParam = kFmiLastParameter, const NFmiDataIdent *thePossibleStreamLineParam = nullptr, const std::vector<std::unique_ptr<NFmiDataIdent>> *possibleMetaParams = nullptr);
	virtual ~NFmiMenuItemList(void);
	
	// Asettaa listan menuItemeille ID:t:
	bool InitializeCommandIDs(unsigned long theFirstCommandID);

	bool Add(MenuItem &&theMenuItem);
	void Clear();
	bool Find(const long &theId);
	NFmiMenuItem* RecursivelyFoundMenuItem(void);
    MenuItemList::iterator begin();
    MenuItemList::iterator end();
	int NumberOfSubMenus(int theNumberOfSubmenus = 0);
	size_t NumberOfMenuItems(void);
	void CalcMinAndMaxId(void);
	unsigned int MinId(void);
	unsigned int MaxId(void);



 private:
     void SortParamsInAlphabeticalOrder();

     MenuItemList itsMenuItemList;
	 NFmiMenuItem* itsRecursivelyFoundMenuItem = nullptr;
	 unsigned int itsMinId = 421234567; // Joku iso luku vain
	 unsigned int itsMaxId = 0;

};

