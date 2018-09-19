//**********************************************************
// C++ Class Name : NFmiLocationSelectionTool 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiLocationSelectionTool.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : selection tool -luokka 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Jun 10, 1999 
// 
// 
//  Description: 
//   Luokkaa k�ytet��n valitesemaan (maskaamaan) 
//   haluttuja paikkoja annetusta NFmiDrawParam-luokan NFmiSmartInfo-luokasta. 
//   Valintaa teht�ess� annetaan luokalle latitude-longitude 
//   piste, mista valinnat alkavat. Se miten valinta 
//   tehd��n, riippuu valitusta ty�kalusta. Jos 
//   ty�kalu on single-selection, valituksi tulee 
//   asema/hila, joka on l�hinn� annettua pistett�. 
//   Jos ty�kalu oli circle-selection, valituksi 
//   tulee asemat/hilat tietyn ympyr�n sis�ll�
//   Muuten luokka valitsee pisteet luokkansa perityill� maskaus ominaisuuksilla.
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiLocationSelectionTool.h"
#include "NFmiSmartInfo.h"
#include "NFmiArea.h"
#include "NFmiMetEditorTypes.h"
#include "EditedInfoMaskHandler.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiLocationSelectionTool::NFmiLocationSelectionTool (void)
:NFmiInfoAreaMask()
,itsSelectedTool(kFmiSingleLocationSelection)
,itsSearchRange(.1f)
,fUseValueFromLocation(false)
,fSearchOnlyNeibhors(false)
,fLimitSearchWithRange(false)
{
}
//--------------------------------------------------------
// SelectLocations 
//--------------------------------------------------------

//   Annetusta drawparamista suoritetaan haluttu 
//   paikkavalinta. latlon on paikka, mihin haku keskittyy.
//   area:a k�ytet��n et�isyys 
//   laskuihin ja et�isyys laskut ovat aina suhteellisia 
//   (ei esim. km tai muita yksik�it�). funktio 
//   on taas menetelm�, mill� yhdistet��n jo olemassa 
//   olevat valinnat ja uudet valinnat (clear 
//   first, add eli union, leikkaus eli section). 
//   theMask kertoo mihin maskiin valinnat tehd��n 
//   smartInfoon (esim. selection, activation, 
//   display, jotka ovat multilevelmaskissa).
//   Huom! jos halutaan tehd� single tai circle 
//   selection, voidaan drawparamiksi antaa 0-pointteri, 
//   koska sit� ei tarvita.
bool NFmiLocationSelectionTool::SelectLocations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
													  ,const NFmiPoint& theLatLon
													  ,const boost::shared_ptr<NFmiArea> &theArea
													  ,FmiSelectionCombineFunction theFunction
													  ,unsigned long theMask
													  ,const NFmiPoint& theViewGridSize)
{
	if(theInfo)
	{
		if(theFunction == kFmiSelectionCombineClearFirst)
			ClearLocationSelection(theInfo, theMask);

		if(theMask == NFmiMetEditorTypes::kFmiDisplayedMask)
			SingleSelection(theInfo, theLatLon, theArea, theFunction, theMask);
		else
		{
			switch(itsSelectedTool)
			{
			case kFmiSingleLocationSelection:
				SingleSelection(theInfo, theLatLon, theArea, theFunction, theMask);
				break;
			case kFmiCircleLocationSelection:
				CircleSelection(theInfo, theLatLon, theArea, theFunction, theMask, theViewGridSize);
				break;
			case kFmiParamMaskLocationSelection:
				ParamSelection(theInfo, theLatLon, theArea, theFunction, theMask);
				break;
			}
		}
	   return true;
	}
   return false;
}

//--------------------------------------------------------
// ClearLocationSelection 
//--------------------------------------------------------
void NFmiLocationSelectionTool::ClearLocationSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long theMask)
{
	if(theInfo)
		dynamic_cast<NFmiSmartInfo*>(theInfo.get())->MaskAllLocations(false, theMask);
}

// T�t� pit�isi kutsua vain NFmiSmartInfo-luokan otuksilla, mutta t�m� funktio kuitenkin tarkistaan ett�
// olio on oikeasti mit� pit�� olla ja vasta sitten kutsuu haluttua metodia.
static void DoMaskLocation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool theState, unsigned long theMask)
{
	NFmiSmartInfo *info = dynamic_cast<NFmiSmartInfo*>(theInfo.get());
	if(info)
		info->MaskLocation(theState, theMask);
}

//--------------------------------------------------------
// SingleSelection 
//--------------------------------------------------------
void NFmiLocationSelectionTool::SingleSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
												,const NFmiPoint& theLatLon
												,const boost::shared_ptr<NFmiArea> &theArea
												,FmiSelectionCombineFunction theFunction
												,unsigned long theMask)
{
	if(theInfo && theArea && theInfo->NearestLocation(NFmiLocation(theLatLon.X(), theLatLon.Y())))
	{
		NFmiPoint cursorViewPosition(theArea->ToXY(theLatLon));
		NFmiPoint currentLocationViewPosition(theArea->ToXY(theInfo->LatLon()));
		if(!LimitSearchWithRange() || cursorViewPosition.Distance(currentLocationViewPosition) <= itsSearchRange/2.)
		{
			if(theFunction == kFmiSelectionCombineRemove)
				::DoMaskLocation(theInfo, false, theMask);
			else
				::DoMaskLocation(theInfo, true, theMask);
		}
	}
}
//--------------------------------------------------------
// CircleSelection 
//--------------------------------------------------------
void NFmiLocationSelectionTool::CircleSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
												,const NFmiPoint& theLatLon
												,const boost::shared_ptr<NFmiArea> &theArea
												,FmiSelectionCombineFunction theFunction
												,unsigned long theMask
												,const NFmiPoint& theViewGridSize)
{
	if(theInfo && theArea)
	{
        EditedInfoMaskHandler editedInfoMaskHandler(theInfo, NFmiMetEditorTypes::kFmiNoMask);
		NFmiPoint cursorViewPosition(theArea->ToXY(theLatLon));
		for(theInfo->ResetLocation(); theInfo->NextLocation(); )
		{
			NFmiPoint currentLocationViewPosition(theArea->ToXY(theInfo->LatLon()));
			if(cursorViewPosition.Distance(currentLocationViewPosition) <= itsSearchRange/(2. * theViewGridSize.X())) // 1999.10.04/Marko Ottaa huomioon n�ytt�ruudukon, ett� voi ottaa huomioon searchrangen koon ruudukossa
				if(theFunction == kFmiSelectionCombineRemove)
					::DoMaskLocation(theInfo, false, theMask);
				else
					::DoMaskLocation(theInfo, true, theMask);
		}
	}
}
//--------------------------------------------------------
// ParamSelection 
//--------------------------------------------------------
void NFmiLocationSelectionTool::ParamSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
											   ,const NFmiPoint& theLatLon
											   ,const boost::shared_ptr<NFmiArea> &theArea
											   ,FmiSelectionCombineFunction theFunction
											   ,unsigned long theMask)
{
	if(theInfo && theArea)
	{
        EditedInfoMaskHandler editedInfoMaskHandler(theInfo, NFmiMetEditorTypes::kFmiNoMask);
        if(UseValueFromLocation())
		{
			double value = Info()->InterpolatedValue(theLatLon);
			LowerLimit(value); // laitetaan klikatun kohdan arvo lowerlimitiksi
		}

		if(!LimitSearchWithRange())
		{
			for(theInfo->ResetLocation(); theInfo->NextLocation(); )
			{
				if(theArea->IsInside(theInfo->LatLon()))
					::DoMaskLocation(theInfo, true, theMask);
			}
		}
		else
		{
			NFmiPoint cursorViewPosition(theArea->ToXY(theLatLon));
			for(theInfo->ResetLocation(); theInfo->NextLocation(); )
			{
				NFmiPoint currentLocationViewPosition(theArea->ToXY(theInfo->LatLon()));
				if(cursorViewPosition.Distance(currentLocationViewPosition) <= itsSearchRange/2.)
				{
					if(theArea->IsInside(theInfo->LatLon()))
						::DoMaskLocation(theInfo, true, theMask);
				}
			}
		}
	}
}
