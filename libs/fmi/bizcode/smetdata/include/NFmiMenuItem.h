//**********************************************************
// C++ Class Name : NFmiMenuItem
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMenuItem.h
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
// Changed 1999.08.25/Marko	Lis‰sin uuden konstruktorin ja Level()-Metodin
//							, ja data osan -levelin.
// Changed 1999.09.22/Marko	Lis‰sin dataosaksi smartinfon datatyypin.
//
//**********************************************************

#pragma once

#include "NFmiString.h"
#include "NFmiParameterName.h"
#include "FmiNMeteditLibraryDefinitions.h"
#include "NFmiDataIdent.h"
#include "NFmiInfoData.h"
#include "NFmiMetEditorTypes.h"

#include <memory>

class NFmiMenuItemList;
class NFmiLevel;

class NFmiMenuItem
{

 public:

	NFmiMenuItem(void);
	NFmiMenuItem(const std::string &theText, const FmiParameterName& theParam);
	NFmiMenuItem(int theMapViewDescTopIndex, const std::string &theText, const NFmiDataIdent& theDataIdent,
				const FmiMenuCommandType &theMenuCommandType, 
				const NFmiMetEditorTypes::View &theViewType, const NFmiLevel* theLevel,
				NFmiInfoData::Type theDataType,
				int theIndexInViewRow = -1,
				bool viewMacroDrawParam = false);
	NFmiMenuItem(int theMapViewDescTopIndex, const std::string &theText, const FmiParameterName& theParam,
				const FmiMenuCommandType &theMenuCommandType,
				const NFmiMetEditorTypes::View &theViewType, const NFmiLevel* theLevel,
				NFmiInfoData::Type theDataType,
				int theIndexInViewRow = -1,
				bool viewMacroDrawParam = false);
	virtual ~NFmiMenuItem();
    NFmiMenuItem(NFmiMenuItem &theMenuItem) = delete;
    NFmiMenuItem &operator=(const NFmiMenuItem &theMenuItem) = delete;

	const FmiMenuCommandType &CommandType(void) const;
	const std::string &MenuText(void) const;
	const FmiParameterName &Parameter(void) const;
	const NFmiDataIdent& DataIdent(void)const {return itsDataIdent;};
    const NFmiLevel* Level(void) const;
	const NFmiMetEditorTypes::View &ViewType(void) const;
	const long& CommandId(void) const;
	NFmiMenuItemList* SubMenu(void) const;
	NFmiInfoData::Type DataType(void)const{return itsDataType;}; // 1999.09.22/Marko
	void DataType(NFmiInfoData::Type theType){itsDataType = theType;}; // 1999.09.22/Marko
	size_t NumberOfSubMenuItems(void);

	void CommandId(long theCommandId);

	bool AddSubMenu (NFmiMenuItemList* theSubMenu);

	int IndexInViewRow(void) const;
	void IndexInViewRow(int newValue);
	bool ViewMacroDrawParam(void) const {return fViewMacroDrawParam;}
	void ViewMacroDrawParam(bool newState) {fViewMacroDrawParam = newState;}
	const std::string& MacroParamInitName(void) const {return itsMacroParamInitName;}
	void MacroParamInitName(const std::string &theName) {itsMacroParamInitName = theName;}
	int MapViewDescTopIndex(void) const {return itsMapViewDescTopIndex;}
	void MapViewDescTopIndex(int newValue) {itsMapViewDescTopIndex = newValue;}
	double ExtraParam(void) const {return itsExtraParam;}
	void ExtraParam(double newValue) {itsExtraParam = newValue;}
 private:
   NFmiDataIdent itsDataIdent;
   FmiMenuCommandType itsCommandType;

   std::string itsMenuText;
   FmiParameterName itsParameter;
   NFmiMetEditorTypes::View itsViewType;
   std::unique_ptr<NFmiLevel> itsLevel; // t‰t‰ tarvitaan joissain tapauksissa kun pit‰‰ asettaa/valita leveli

//   Joskus varmaan tarvitsee 'merkata' k‰sky
//   ihan erillisell‰ tunnuksella.
   long itsCommandId; // WM_USER+? // windowsin komento id arvo

   std::unique_ptr<NFmiMenuItemList> itsSubMenu;		// Menu item saataa sis‰lt‰‰ ali-menun.
   NFmiInfoData::Type itsDataType; // 1999.09.22/Marko

   int itsMapViewDescTopIndex; // tein t‰st‰ tahallaan int enk‰ unsigned:in, ett‰ negatiivinen numero olisi puuttuva. Lis‰ksi laitoin
								// sen tahallaan konstruktorissa 1. parametriksi, ett‰ k‰‰nt‰j‰ valittaisi ja huomaisi miss‰ sit‰ ei ole k‰ytetty.
   int itsIndexInViewRow; // tarvitaan erottamiseen jos samalla n‰yttˆrivill‰ on useita samoja parametreja
						  // -1 tarkoittaa, ett‰ indeksill‰ ei ole v‰li‰ (tai 'alustamaton')
	bool fViewMacroDrawParam; // is this DrawParam from viewmacro, if it is, then some things are handled
							  // differently when modifying options, default value is false
	std::string itsMacroParamInitName; // kun macroparamia k‰ytet‰‰n popup-valikoista k‰sin, pit‰‰ t‰h‰n laittaa koko polku talteen
	double itsExtraParam; // tein t‰ll‰isen yleis parametrin, jos jossain k‰skyiss‰ tarvitaan sellaista (nyt tarvitsen sit‰ aikasarjan modelrun m‰‰r‰n asetukseen)
};

