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

class NFmiParamBag;
class NFmiDrawParamList;
class NFmiLevelBag;
class NFmiDataIdent;

class NFmiMenuItemList 
{

 public:
	NFmiMenuItemList(void);
	NFmiMenuItemList(NFmiParamBag* theParamBag);
	NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag, const FmiMenuCommandType &theMenuCommandType,
					const NFmiMetEditorTypes::View &theViewType, NFmiInfoData::Type theDataType, const NFmiDataIdent *thePossibleStreamLineParam = nullptr, const NFmiDataIdent *thePossibleWindVectorParam = nullptr);
	NFmiMenuItemList(int theMapViewDescTopIndex, const NFmiDataIdent& theDataIdent, 
					const FmiMenuCommandType& theMenuCommandType, NFmiLevelBag* theLevels, NFmiInfoData::Type theDataType);

	NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag, const FmiMenuCommandType &theMenuCommandType,
						const NFmiMetEditorTypes::View &theViewType, NFmiLevelBag* theLevels, NFmiInfoData::Type theDataType, FmiParameterName notLevelParam = kFmiLastParameter, const NFmiDataIdent *thePossibleStreamLineParam = nullptr, const NFmiDataIdent *thePossibleWindVectorParam = nullptr);
	NFmiMenuItemList(NFmiDrawParamList* theDrawParamList);
	virtual ~NFmiMenuItemList(void);
	
	// Asettaa listan menuItemeille ID:t:
	bool InitializeCommandIDs(unsigned long theFirstCommandID);

	bool Add(NFmiMenuItem* theMenuItem);
	void Clear(bool fDeleteItem = false);
	bool Find(const long &theId);
	NFmiMenuItem *FoundMenuItem(void);
	void Print(int roundCheck);			// Tämä on vain testausta varten.
	void Reset(void);
	bool Next(void);
	NFmiMenuItem *Current(void);
	int NumberOfSubMenus(int theNumberOfSubmenus = 0);
	unsigned long NumberOfMenuItems(void);
	void CalcMinAndMaxId(void);
	unsigned int MinId(void);
	unsigned int MaxId(void);



 private:
	 NFmiPtrList<NFmiMenuItem> itsList;
	 NFmiPtrList<NFmiMenuItem>::Iterator itsIter;
	 NFmiMenuItem *itsFoundMenuItem;
	 unsigned int itsMinId;
	 unsigned int itsMaxId;

};

