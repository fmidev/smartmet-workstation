// ======================================================================
/*!
 * \file NFmiViewSettingMacro.cpp
 * \brief Implementation of class NFmiViewSettingMacro
 */
// ======================================================================

#include "NFmiViewSettingMacro.h"
#include "NFmiArea.h"
#include "NFmiDrawParamList.h"
#include "NFmiAreaMaskList.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParam.h"
#include "NFmiAreaFactory.h"
#include "NFmiDataStoringHelpers.h"
#include "CtrlViewFunctions.h"
#include "NFmiDrawParam.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace std;

double NFmiViewSettingMacro::itsLatestVersionNumber = 2.0;

bool NFmiLightWeightViewSettingMacro::ViewMacroOk() const 
{ 
    return fViewMacroOk; 
}

bool NFmiLightWeightViewSettingMacro::IsEmpty() const
{
    if(fIsViewMacroDirectory)
        return itsName.empty();
    else if(itsName.empty() || itsInitFilePath.empty())
        return true;
    else
        return false;
}

void SkipRestOfTheLine(std::istream &is)
{
    std::string line;
    std::getline(is, line);
}

bool SkipViewMacroFileCommentLine(std::istream &is)
{
    char ch = 0;
    do
    {
        is.get(ch);
        if(is.fail())
            return false;
    } while(std::isspace(ch)); // etsit‰‰n merkki kerrallaan kunnes vastaan tulee ei space (oletus, stream alkaa aina rivin alusta)
    if(ch == '/' || ch == '#')
    { // Jos oli kommentti merkki rivin alussa (spacejen j‰lkeen), luetaan loppu rivi pois
        ::SkipRestOfTheLine(is);
        return true;
    }
    else
    { // Jos oli jotain muuta alussa, laitetaan merkki takaisin streamiin
        is.unget();
        return false;
    }
}

bool GetNextNumberFromViewMacroFile(std::istream &is, double &number)
{
    for(;;)
    {
        if(!::SkipViewMacroFileCommentLine(is))
        {
            if(is.fail())
                return false;
            is >> number;
            return !is.fail();
        }
    }
}

// T‰m‰ luetaan raa'asta tiedostosta, ilman ett‰ poistetaan kommentteja ja ett‰ luetaan ensin koko tiedosto streamiin muistiin.
void NFmiLightWeightViewSettingMacro::Read(std::istream& is)
{
    fViewMacroOk = true;
    double versionNumber = -1;
    // Luetaan ensin versionumero pois alta
    if(::GetNextNumberFromViewMacroFile(is, versionNumber))
    {
        ::SkipRestOfTheLine(is);
        double nameStringLength = -1;
        // Luetaan sitten nimi-stringiin liittyv‰ numero
        if(::GetNextNumberFromViewMacroFile(is, nameStringLength))
        {
            ::SkipRestOfTheLine(is);
            double descriptionStringLength = -1;
            // Luetaan sitten description-stringiin liittyv‰ numero
            if(::GetNextNumberFromViewMacroFile(is, descriptionStringLength))
            {
                int stringSize = static_cast<int>(descriptionStringLength);
                if(stringSize > 0)
                {
                    if(stringSize > 4000)
                        stringSize = 4000; // rajoitetaan luetun descriptionin kooksi 4000 merkki‰
                    char ch;
                    is.get(ch); // luetaan space pois
                    itsDescription.resize(stringSize);
                    is.read(&itsDescription[0], stringSize);
                    return;
                }
            }
        }
    }

    if(is.fail())
        fViewMacroOk = false;
}


NFmiViewSettingMacro::Param::Param(void)
:itsDrawParam(new NFmiDrawParam())
,itsLevel()
,itsDataType(NFmiInfoData::kNoDataType)
,itsModelOrigTimeOffsetInHours(0)
,fHidden(false)
,fActive(false)
,fShowTimeDifference(false)
,fShowDifferenceToOriginalData(false)
{
	itsDrawParam->ViewMacroDrawParam(true);
}

NFmiViewSettingMacro::Param::Param(const boost::shared_ptr<NFmiDrawParam> &theDrawParam, const NFmiLevel &theLevel, NFmiInfoData::Type theDataType, int theModelOrigTimeOffsetInHours)
:itsDrawParam(new NFmiDrawParam(*theDrawParam))
,itsDataIdent(const_cast<NFmiDrawParam*>(theDrawParam.get())->Param())
,itsLevel(theLevel)
,itsDataType(theDataType)
,itsModelOrigTimeOffsetInHours(theModelOrigTimeOffsetInHours)
,fHidden(theDrawParam->IsParamHidden())
,fActive(theDrawParam->IsActive())
,fShowTimeDifference(theDrawParam->ShowDifference())
,fShowDifferenceToOriginalData(theDrawParam->ShowDifferenceToOriginalData())
{
	itsDrawParam->ViewMacroDrawParam(true);
}

NFmiViewSettingMacro::Param::Param(const NFmiDataIdent &theDataIdent, const NFmiLevel &theLevel, NFmiInfoData::Type theDataType, int theModelOrigTimeOffsetInHours)
:itsDrawParam(new NFmiDrawParam())
,itsDataIdent(theDataIdent)
,itsLevel(theLevel)
,itsDataType(theDataType)
,itsModelOrigTimeOffsetInHours(theModelOrigTimeOffsetInHours)
,fHidden(false)
,fActive(false)
,fShowTimeDifference(false)
,fShowDifferenceToOriginalData(false)
{
	itsDrawParam->ViewMacroDrawParam(true);
}

NFmiViewSettingMacro::Param::~Param(void)
{
}

void NFmiViewSettingMacro::Param::DrawParam(const boost::shared_ptr<NFmiDrawParam> &newValue)
{
	itsDrawParam = boost::shared_ptr<NFmiDrawParam>(new NFmiDrawParam(*newValue));
	itsDrawParam->ViewMacroDrawParam(true);
}

void NFmiViewSettingMacro::Param::SetMacroParamInitFileNames(const std::string &theRootPath)
{
	// siis jos kyseess‰ macroParam, pit‰‰ sille asettaa initFileName oikein, ett‰
	// macroParamit saadaan ladattua oikein alihakemistoistakin.
	if(NFmiDrawParam::IsMacroParamCase(DataType()))
	{
		std::string tmpStr(theRootPath);
		if(!itsDrawParam->MacroParamRelativePath().empty())
		{
			tmpStr += itsDrawParam->MacroParamRelativePath();
			tmpStr += kFmiDirectorySeparator;
		}
		tmpStr += itsDrawParam->ParameterAbbreviation();
		tmpStr += ".dpa";
		itsDrawParam->InitFileName(tmpStr);
	}
}

void NFmiViewSettingMacro::Param::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::Param::Write..." << endl;
	os << "// NFmiDrawParam" << endl;
	os << *itsDrawParam << endl;
	os << "// DataIdent" << endl;
	os << itsDataIdent << endl;
	os << "// NFmiLevel" << endl;
	os << itsLevel << endl;
	os << "// NFmiInfoData::Type" << endl;
	os << itsDataType << endl;
	os << "// ModelOrigTimeOffsetInHours" << endl;
	os << itsModelOrigTimeOffsetInHours << endl;
	os << "// fHidden fActive fShowTimeDifference fShowDifferenceToOriginalData" << endl;
	os << fHidden << " " << fActive << " " << fShowTimeDifference << " " << fShowDifferenceToOriginalData << endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::Param::Write ep‰onnistui");
}

void NFmiViewSettingMacro::Param::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Param::Read failed";
    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	if(is)
		is >> *itsDrawParam;
	itsDrawParam->ViewMacroDrawParam(true);
	if(is)
		is >> itsDataIdent;
	if(is)
		itsDrawParam->Param(itsDataIdent); // pit‰‰ asettaa myˆs parametri kohdalleen drawParamiin, muuten koiranpennut rupee juhlimaan
	if(is)
		is >> itsLevel;
	if(is)
		itsDrawParam->Level(itsLevel); // myˆs level pit‰‰ asettaa t‰ss‰ heti
	if(is)
	{
		int tmp = 0;
		is >> tmp;
		itsDataType = static_cast<NFmiInfoData::Type>(tmp);
	}
	if(is)
		is >> itsModelOrigTimeOffsetInHours;
	if(is)
		is >> fHidden >> fActive >> fShowTimeDifference >> fShowDifferenceToOriginalData;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}

NFmiViewSettingMacro::Mask::Mask(void)
:itsParamSettings()
,itsMaskSettings()
,fMaskEnabled(true)
{
}

NFmiViewSettingMacro::Mask::Mask(const Param &theParamSettings, const NFmiCalculationCondition &theMaskSettings, bool theMaskEnabled)
:itsParamSettings(theParamSettings)
,itsMaskSettings(theMaskSettings)
,fMaskEnabled(theMaskEnabled)
{
}

NFmiViewSettingMacro::Mask::~Mask(void)
{
}

void NFmiViewSettingMacro::Mask::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::Mask::Write..." << endl;
	os << "// NFmiViewSettingMacro::Param" << endl;
	os << itsParamSettings << endl;
	os << "// NFmiCalculationCondition" << endl;
	os << itsMaskSettings << endl;
	os << "// MaskEnabled" << endl;
	os << fMaskEnabled << endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::Mask::Write ep‰onnistui");
}

void NFmiViewSettingMacro::Mask::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Mask::Read failed";
    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsParamSettings;
	is >> itsMaskSettings;
	is >> fMaskEnabled;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}

NFmiViewSettingMacro::MaskSettings::MaskSettings(void)
:itsMasks()
,fShowMasksOnMapView(false)
,fUseMasksInTimeSerialViews(false)
,fUseMasksWithFilterTool(false)
,fUseMaskWithBrush(false)
{
}

NFmiViewSettingMacro::MaskSettings::MaskSettings(const checkedVector<Mask> &theMasks, bool theShowMasksOnMapView, bool theUseMasksInTimeSerialViews, bool theUseMasksWithFilterTool, bool theUseMaskWithBrush)
:itsMasks(theMasks)
,fShowMasksOnMapView(theShowMasksOnMapView)
,fUseMasksInTimeSerialViews(theUseMasksInTimeSerialViews)
,fUseMasksWithFilterTool(theUseMasksWithFilterTool)
,fUseMaskWithBrush(theUseMaskWithBrush)
{
}

NFmiViewSettingMacro::MaskSettings::~MaskSettings(void)
{
}

void NFmiViewSettingMacro::MaskSettings::SetAllMasks(NFmiAreaMaskList &theMasks)
{
	Clear();
	for(theMasks.Reset(); theMasks.Next(); )
	{
		boost::shared_ptr<NFmiAreaMask> areaMask = theMasks.Current();
		NFmiViewSettingMacro::Param param;
		if(areaMask->Level())
			param = NFmiViewSettingMacro::Param(*areaMask->DataIdent(), *areaMask->Level(), areaMask->GetDataType(), 0);
		else
			param = NFmiViewSettingMacro::Param(*areaMask->DataIdent(), NFmiLevel(), areaMask->GetDataType(), 0);
		NFmiViewSettingMacro::Mask mask(param, areaMask->Condition(), areaMask->IsEnabled());
		Add(mask);
	}
}

void NFmiViewSettingMacro::MaskSettings::Clear(void)
{
	itsMasks.clear();
}

void NFmiViewSettingMacro::MaskSettings::Add(const Mask &theMask)
{
	itsMasks.push_back(theMask);
}

void NFmiViewSettingMacro::MaskSettings::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::MaskSettings::Write..." << endl;
	os << "// checkedVector<Mask>" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsMasks, os, string("\n"));
	os << "// ShowMasksOnMapView UseMasksInTimeSerialViews fUseMasksWithFilterTool fUseMaskWithBrush" << endl;
	os << fShowMasksOnMapView << " " << fUseMasksInTimeSerialViews << " " << fUseMasksWithFilterTool << " " << fUseMaskWithBrush << endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::MaskSettings::Write ep‰onnistui");
}

void NFmiViewSettingMacro::MaskSettings::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::MaskSettings::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	NFmiDataStoringHelpers::ReadContainer(itsMasks, is);
	is >> fShowMasksOnMapView >> fUseMasksInTimeSerialViews >> fUseMasksWithFilterTool >> fUseMaskWithBrush;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}

NFmiViewSettingMacro::MapRow::MapRow(void)
:itsRowParams()
{
}

NFmiViewSettingMacro::MapRow::MapRow(const checkedVector<Param>& theParams)
:itsRowParams(theParams)
{
}

NFmiViewSettingMacro::MapRow::~MapRow(void)
{
}

void NFmiViewSettingMacro::MapRow::Clear(void)
{
	itsRowParams.clear();
}

void NFmiViewSettingMacro::MapRow::Add(const Param &theParam)
{
	itsRowParams.push_back(theParam);
}

struct SetMacroParamInitFileNamesFunctor
{
	SetMacroParamInitFileNamesFunctor(const std::string &theRootPath):itsRootPath(theRootPath){}

	template<class T>
	void operator()(T & theRow)
	{
		theRow.SetMacroParamInitFileNames(itsRootPath);
	}

	std::string itsRootPath;
};

void NFmiViewSettingMacro::MapRow::SetMacroParamInitFileNames(const std::string &theRootPath)
{
	std::for_each(itsRowParams.begin(), itsRowParams.end(), SetMacroParamInitFileNamesFunctor(theRootPath));
}

void NFmiViewSettingMacro::MapRow::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::MapRow::Write..." << endl;
	os << "// checkedVector<Param> itsRowParams" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsRowParams, os, string("\n"));

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::MapRow::Write ep‰onnistui");
}

void NFmiViewSettingMacro::MapRow::Read(std::istream& is)
{ // toivottavasti olet poistanut kommentit luettavasta streamista!!
	NFmiDataStoringHelpers::ReadContainer(itsRowParams, is);
	if(is.fail())
		throw runtime_error("NFmiViewSettingMacro::MapRow::Read ep‰onnistui");
}

NFmiViewSettingMacro::TimeViewRow::TimeViewRow(void)
:itsParam()
{
}

NFmiViewSettingMacro::TimeViewRow::TimeViewRow(const NFmiViewSettingMacro::Param& theParam)
:itsParam(theParam)
{
}

NFmiViewSettingMacro::TimeViewRow::~TimeViewRow(void)
{
}

void NFmiViewSettingMacro::TimeViewRow::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::TimeViewRow::Write..." << endl;
	os << "// Param" << endl;
	os << itsParam << endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TimeViewRow::Write ep‰onnistui");
}

void NFmiViewSettingMacro::TimeViewRow::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TimeViewRow::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsParam;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}

static NFmiViewSettingMacro::MapRow MakeMapRow(NFmiDrawParamList *theDrawParamList, NFmiMacroParamSystem& theMacroParamSystem)
{
	NFmiViewSettingMacro::MapRow mapRow;
	for(theDrawParamList->Reset(); theDrawParamList->Next(); )
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = theDrawParamList->Current();
		if(NFmiDrawParam::IsMacroParamCase(drawParam->DataType()))
		{ // t‰m‰ on ik‰v‰‰ koodia, mutta en keksinyt t‰h‰n h‰t‰‰n parempaa. Eli pit‰‰ saada viewMacrossa olevaan macroParamiin
			// suhteellinen polku talteen, joka talletetaan drawParamiin. Mutta t‰m‰ drawParam on kahdessa paikassa
			// hieman erilaisena ja oikean suht.polun saa vain MacroParamSystemist‰ lˆytyv‰st‰ macroParamin DrawParamista
			// eika dokumentista lˆytyv‰st‰ DrawParamListasta.
			if(theMacroParamSystem.FindTotal(drawParam->InitFileName()))
				drawParam->MacroParamRelativePath(theMacroParamSystem.CurrentMacroParam()->DrawParam()->MacroParamRelativePath());
		}
		NFmiViewSettingMacro::Param param(drawParam, drawParam->Level(), drawParam->DataType(), 0);
		mapRow.Add(param);
	}
	return mapRow;
}

NFmiViewSettingMacro::GeneralDoc::GeneralDoc(void)
:itsProjectionCurvatureInfo()
,itsCPLocationVector()
{
}

NFmiViewSettingMacro::GeneralDoc::~GeneralDoc(void)
{
}

void NFmiViewSettingMacro::GeneralDoc::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::GeneralDoc::Write..." << endl;
	os << "// ProjectionCurvatureInfo" << endl;
	os << itsProjectionCurvatureInfo << endl;
	os << "// CPLocationVector" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsCPLocationVector, os, string(" "));

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::GeneralDoc::Write ep‰onnistui");
}

void NFmiViewSettingMacro::GeneralDoc::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::GeneralDoc::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsProjectionCurvatureInfo;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsCPLocationVector, is);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}



NFmiViewSettingMacro::CrossSectionView::CrossSectionView(void)
:itsMapRowSettings()
,itsCrossSectionSystem(CtrlViewUtils::MaxViewGridYSize)
,itsAbsolutRect()
,itsViewStatus()
{
}

NFmiViewSettingMacro::CrossSectionView::~CrossSectionView(void)
{}

void NFmiViewSettingMacro::CrossSectionView::SetAllRowParams(NFmiPtrList<NFmiDrawParamList> *theDrawParamListVector, NFmiMacroParamSystem& theMacroParamSystem)
{
	Clear();
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = theDrawParamListVector->Start();
	for( ; iter.Next(); )
		Add(MakeMapRow(iter.CurrentPtr(), theMacroParamSystem));
}

void NFmiViewSettingMacro::CrossSectionView::SetMacroParamInitFileNames(const std::string &theRootPath)
{
	std::for_each(itsMapRowSettings.begin(), itsMapRowSettings.end(), SetMacroParamInitFileNamesFunctor(theRootPath));
}

void NFmiViewSettingMacro::CrossSectionView::Clear(void)
{
	itsMapRowSettings.clear();
}

void NFmiViewSettingMacro::CrossSectionView::Add(const MapRow &theMapRow)
{
	itsMapRowSettings.push_back(theMapRow);
}

static std::ostream& WriteMfcViewStatus(std::ostream& os, const MfcViewStatus &viewStatus)
{
    os << viewStatus.ShowCommand() << " " << viewStatus.ShowWindow();
    return os;
}

static void ReadMfcViewStatus(std::istream& in, MfcViewStatus &viewStatus)
{
    unsigned int showCommand = 0;
    in >> showCommand;
    viewStatus.ShowCommand(showCommand);
    bool showWindow = false;
    in >> showWindow;
    viewStatus.ShowWindow(showWindow);
}

void NFmiViewSettingMacro::CrossSectionView::Write(std::ostream& os) const
{

	os << "// NFmiViewSettingMacro::CrossSectionView::Write..." << endl;
	os << "// checkedVector<MapRow> itsMapRowSettings" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsMapRowSettings, os, string("\n"));
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status fShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;

	os << "// CrossSectionSystem" << endl;
	os << itsCrossSectionSystem;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::CrossSectionView::Write ep‰onnistui");

}

void NFmiViewSettingMacro::CrossSectionView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::CrossSectionView::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	NFmiDataStoringHelpers::ReadContainer(itsMapRowSettings, is);
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsCrossSectionSystem;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

}



NFmiViewSettingMacro::TimeView::TimeView(void)
:itsRows()
,itsAbsolutRect()
,itsViewStatus()
,fShowHelpData(false)
,fShowHelpData2(false)
,fShowHelpData3(false)
,fShowHelpData4(false)
,itsStartTimeOffset(0)
,itsEndTimeOffset(0)
{
}

NFmiViewSettingMacro::TimeView::~TimeView(void)
{
}

void NFmiViewSettingMacro::TimeView::SetAllParams(NFmiDrawParamList *theDrawParamList)
{
	Clear();
	for(theDrawParamList->Reset(); theDrawParamList->Next(); )
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = theDrawParamList->Current();
		NFmiViewSettingMacro::Param param;
		param = NFmiViewSettingMacro::Param(drawParam, drawParam->Level(), drawParam->DataType(), 0);
		NFmiViewSettingMacro::TimeViewRow row(param);
		itsRows.push_back(row);
	}
}

void NFmiViewSettingMacro::TimeView::Clear(void)
{
	itsRows.clear();
}

void NFmiViewSettingMacro::TimeView::Add(const TimeViewRow &theTimeViewRow)
{
	itsRows.push_back(theTimeViewRow);
}

void NFmiViewSettingMacro::TimeView::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::TimeView::Write..." << endl;
	os << "// checkedVector<TimeViewRow> itsRows" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsRows, os, string("\n"));
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status ShowWindow ShowHelpData" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus);
	os << " " << fShowHelpData << endl;
	os << "// itsStartTimeOffset itsEndTimeOffset" << endl;
	os << itsStartTimeOffset << " " << itsEndTimeOffset << endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.

    // Lis‰t‰‰n apudata2 ja 3 ja 4 asetukset n‰yttˆmakroon
    extraData.Add(static_cast<double>(fShowHelpData2));
    extraData.Add(static_cast<double>(fShowHelpData3));
    extraData.Add(static_cast<double>(fShowHelpData4));

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	std::string timeBagStr = NFmiDataStoringHelpers::GetTimeBagOffSetStr(usedViewMacroTime, itsTimeBag);
	extraData.Add(timeBagStr); // lis‰t‰‰n 1. extra-datana aikaikkunan timebagi offsettina currenttiin aikaan

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TimeView::Write ep‰onnistui");
}

void NFmiViewSettingMacro::TimeView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TimeView::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	NFmiDataStoringHelpers::ReadContainer(itsRows, is);
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
	is >> fShowHelpData;
	is >> itsStartTimeOffset >> itsEndTimeOffset;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

    // Apudata2 ja 3 asetukset n‰yttˆmakrosta
    fShowHelpData2 = false;
    if(extraData.itsDoubleValues.size() >= 1)
        fShowHelpData2 = extraData.itsDoubleValues[0] != 0;
    fShowHelpData3 = false;
    if(extraData.itsDoubleValues.size() >= 2)
        fShowHelpData3 = extraData.itsDoubleValues[1] != 0;
    fShowHelpData4 = false;
    if(extraData.itsDoubleValues.size() >= 3)
        fShowHelpData4 = extraData.itsDoubleValues[2] != 0;

	fTimeBagUpdated = false;
	if(extraData.itsStringValues.size() >= 1)
	{// luetaan 1. extra-datana aikaikkunan timebagi offsettina currenttiin aikaan
		if(is.fail())
			throw std::runtime_error(exceptionErrorMessage);
		NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
		itsTimeBag = NFmiDataStoringHelpers::GetTimeBagOffSetFromStr(usedViewMacroTime, extraData.itsStringValues[0]);
		fTimeBagUpdated = true;
	}

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}

NFmiViewSettingMacro::TempView::TempView(void)
:itsAbsolutRect()
,itsViewStatus()
,fShowHirlam(false)
,fShowEcmwf(false)
,fShowRealSounding(false)
,itsMTATempSystem()
{
}

NFmiViewSettingMacro::TempView::~TempView(void)
{
}

void NFmiViewSettingMacro::TempView::MTATempSystem(const NFmiMTATempSystem &theData)
{
	itsMTATempSystem.InitFromViewMacro(theData);
}

void NFmiViewSettingMacro::TempView::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::TempView::Write..." << endl;
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status ShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;
	os << "// ShowHirlam ShowEcmwf ShowRealSounding" << endl;
	os << fShowHirlam << " " << fShowEcmwf << " " << fShowRealSounding << endl;

	// ************************************
	// T‰st‰ eteenp‰in on versio 2. tavaraa
	// ************************************
	os << "// MTATempSystem" << endl;
	os << itsMTATempSystem << endl;
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TempView::Write ep‰onnistui");
}

void NFmiViewSettingMacro::TempView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TempView::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
    is >> fShowHirlam >> fShowEcmwf >> fShowRealSounding;

	if(itsCurrentVersionNumber > 1.0)
	{
		// luetaan uudet ver 2.0 ja myˆh. jutut
		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsMTATempSystem;

		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
		is >> extraData;
		// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
		// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.
	}
	else
	{
		// muuten tehd‰‰n sopivia alustuksia uusiin muuttujiin
		// menn‰‰n itsMTATempSystem-olion defaultti arvoilla
		itsMTATempSystem = NFmiMTATempSystem();
	}

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}


NFmiViewSettingMacro::TrajectoryView::TrajectoryView()
:itsAbsolutRect()
,itsViewStatus()
,itsTrajectorySystem(0, 0)
{
}

NFmiViewSettingMacro::TrajectoryView::~TrajectoryView(void)
{
}

void NFmiViewSettingMacro::TrajectoryView::TrajectorySystem(const NFmiTrajectorySystem &theData)
{
	itsTrajectorySystem.Init(theData);
}

void NFmiViewSettingMacro::TrajectoryView::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::TrajectoryView::Write..." << endl;
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status ShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;

	os << "// TrajectorySystem" << endl;
	os << itsTrajectorySystem << endl;

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TrajectoryView::Write ep‰onnistui");
}

void NFmiViewSettingMacro::TrajectoryView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TrajectoryView::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsTrajectorySystem;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}


// **********************************************************************
NFmiViewSettingMacro::WarningCenterView::WarningCenterView()
:itsAbsolutRect()
,itsViewStatus()
,itsWarningCenterSystem()
,fShowHakeMessages(true)
,fShowKaHaMessages(false)
,itsMinimumTimeRangeForWarningsOnMapViewsInMinutes(0)
{
}

NFmiViewSettingMacro::WarningCenterView::~WarningCenterView(void)
{
}

void NFmiViewSettingMacro::WarningCenterView::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::WarningCenterView::Write..." << endl;
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status + ShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;

	os << "// WarningCenterSystem" << endl;
	os << itsWarningCenterSystem << endl;

	os << "// Container<Header-Column-Widths-In-Pixels>" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsHeaderColumnWidthsInPixels, os, string(" "));

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.

    extraData.Add(static_cast<double>(fShowHakeMessages)); // fShowHakeMessages on 1. uusi 'double' arvo
    extraData.Add(static_cast<double>(fShowKaHaMessages)); // fShowKaHaMessages on 2. uusi 'double' arvo
    extraData.Add(static_cast<double>(itsMinimumTimeRangeForWarningsOnMapViewsInMinutes)); // itsMinimumTimeRangeForWarningsOnMapViewsInMinutes on 3. uusi 'double' arvo

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::WarningCenterView::Write ep‰onnistui");
}

void NFmiViewSettingMacro::WarningCenterView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::WarningCenterView::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsWarningCenterSystem;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsHeaderColumnWidthsInPixels, is);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

    fShowHakeMessages = true;
    if(extraData.itsDoubleValues.size() >= 1)
        fShowHakeMessages = (extraData.itsDoubleValues[0] == 0) ? false : true;
    fShowKaHaMessages = false;
    if(extraData.itsDoubleValues.size() >= 2)
        fShowKaHaMessages = (extraData.itsDoubleValues[1] == 0) ? false : true;
    itsMinimumTimeRangeForWarningsOnMapViewsInMinutes = 0;
    if(extraData.itsDoubleValues.size() >= 3)
        itsMinimumTimeRangeForWarningsOnMapViewsInMinutes = static_cast<int>(extraData.itsDoubleValues[2]);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}
// **********************************************************************

NFmiViewSettingMacro::SynopDataGridView::SynopDataGridView(void) 
:itsAbsolutRect()
,itsViewStatus()
,itsSelectedProducer()
,itsMinMaxRangeStartTime()
,fMinMaxModeOn(false)
,itsDayRangeValue(1)
,fAllCountriesSelected(true)
,itsSelectedCountryAbbrStr()
,itsHeaderColumnWidthsInPixels()
{
}

NFmiViewSettingMacro::SynopDataGridView::~SynopDataGridView(void) 
{
}

void NFmiViewSettingMacro::SynopDataGridView::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::SynopDataGridView::Write..." << endl;
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status + ShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;

	os << "// SelectedProducer" << endl;
	os << itsSelectedProducer << endl;

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	os << "// MinMaxRangeStartTime with utc hour + minute + day shift to current day" << std::endl;
	NFmiDataStoringHelpers::WriteTimeWithOffsets(usedViewMacroTime, itsMinMaxRangeStartTime, os);

	os << "// MinMaxModeOn + DayRangeValue + AllCountriesSelected" << endl;
	os << fMinMaxModeOn << " " << itsDayRangeValue << " " << fAllCountriesSelected << endl;

	NFmiString tmpStr(itsSelectedCountryAbbrStr);
	os << "// SelectedCountryAbbrStr" << endl;
	os << tmpStr;

	os << "// Container<Header-Column-Widths-In-Pixels>" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsHeaderColumnWidthsInPixels, os, string(" "));

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::SynopDataGridView::Write ep‰onnistui");
}

void NFmiViewSettingMacro::SynopDataGridView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::SynopDataGridView::Read failed";

    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsSelectedProducer;

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	NFmiDataStoringHelpers::ReadTimeWithOffsets(usedViewMacroTime, itsMinMaxRangeStartTime, is);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> fMinMaxModeOn >> itsDayRangeValue >> fAllCountriesSelected;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiString tmpStr;
	is >> tmpStr;
	itsSelectedCountryAbbrStr = static_cast<char*>(tmpStr);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsHeaderColumnWidthsInPixels, is);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}


NFmiViewSettingMacro::SynopPlotSettings::SynopPlotSettings(void)
:itsSynopPlotSettings()
{
}

NFmiViewSettingMacro::SynopPlotSettings::~SynopPlotSettings(void)
{
}

void NFmiViewSettingMacro::SynopPlotSettings::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::SynopPlotSettings::Write..." << endl;

	os << itsSynopPlotSettings << endl;

	// ei k‰ytet‰ extradata talletusta t‰ss‰, koska se on jo NFmiSynopPlotSettings-luokassa
//	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::SynopPlotSettings::Write ep‰onnistui");
}

void NFmiViewSettingMacro::SynopPlotSettings::Read(std::istream& is)
{
	is >> itsSynopPlotSettings;
	if(is.fail())
		throw runtime_error("NFmiViewSettingMacro::SynopPlotSettings::Read ep‰onnistui");
}

NFmiViewSettingMacro::ObsComparisonInfo::ObsComparisonInfo(void)
:itsObsComparisonInfo()
{
}

NFmiViewSettingMacro::ObsComparisonInfo::~ObsComparisonInfo(void)
{
}

void NFmiViewSettingMacro::ObsComparisonInfo::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::ObsComparisonInfo::Write..." << endl;

	os << itsObsComparisonInfo << endl;

	// ei k‰ytet‰ extradata talletusta t‰ss‰, koska se on jo NFmiSynopPlotSettings-luokassa
//	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::ObsComparisonInfo::Write ep‰onnistui");
}

void NFmiViewSettingMacro::ObsComparisonInfo::Read(std::istream& is)
{
	is >> itsObsComparisonInfo;
	if(is.fail())
		throw runtime_error("NFmiViewSettingMacro::ObsComparisonInfo::Read ep‰onnistui");
}


NFmiViewSettingMacro::NFmiViewSettingMacro(void)
:itsCurrentVersionNumber(itsLatestVersionNumber)
,itsName()
,itsDescription()
,itsInitFileName()
,itsGeneralDoc()
,itsTimeView()
,itsTempView()
,itsCrossSectionView()
,itsSynopPlotSettings()
,itsMaskSettings()
,itsWarningCenterView()
,itsSynopDataGridView()
,itsExtraMapViewDescTops()
,fIsPrinterPortrait(true)
,fViewMacroDirectory(false)
,fViewMacroWasCorrupted(false)
,fUseBrushTool(false)
,fUseAnalyzeTool(false)
,fUseControlPoinTool(false)
,fUseAnimationTool(false)
,itsAnimationStartPosition(0)
,itsAnimationEndPosition(0)
,itsAnimationDelayInMS(150)
,itsOriginalLoadVersionNumber(itsLatestVersionNumber)
,itsTrajectoryView()
{
}

NFmiViewSettingMacro::~NFmiViewSettingMacro(void)
{
}

void ChangePossibleComments(NFmiString &theText)
{
	static const NFmiString comment1("//");
	static const NFmiString comment2("/*");
	static const NFmiString comment3("*/");
	static const NFmiString replacement("&&");
	theText.ReplaceChars(comment1, replacement);
	theText.ReplaceChars(comment2, replacement);
	theText.ReplaceChars(comment3, replacement);
}

void NFmiViewSettingMacro::Write(std::ostream& os) const
{
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Write failed";

	if(fViewMacroDirectory)
		return ; // hakemisto virityst‰ ei ole tarkoitus tallettaa

	os << "// NFmiViewSettingMacro::Write..." << endl;
	os << "// version number" << endl;
	itsCurrentVersionNumber = itsLatestVersionNumber; // aina kirjoitetaan viimeisell‰ versio numerolla
	os << itsCurrentVersionNumber << endl;
	os << "// itsName" << endl;
	// talletan itsName ja itsDescription NFmiString:ein‰, ett‰ luku ja kirjoitus menev‰t ok vaikka olisi white spaceja
	NFmiString tmp1(itsName);
	ChangePossibleComments(tmp1); // pit‰‰ korvata mahd. kommentti-merkit, koska luettaessa kommentit poistetaan ja sitten oltaisiin pulassa
	os << tmp1;
	os << "// Description" << endl;
	NFmiString tmp2(itsDescription);
	ChangePossibleComments(tmp2); // pit‰‰ korvata mahd. kommentti-merkit, koska luettaessa kommentit poistetaan ja sitten oltaisiin pulassa
	os << tmp2;
	os << "// GeneralDoc-settings" << endl;
	os << itsGeneralDoc << endl;
	os << "// TimeView-settings (timeseries-view)" << endl;
	os << itsTimeView << endl;
	os << "// TempView-settings (sounding-view)" << endl;
	os << itsTempView << endl;
	os << "// MaskSettings" << endl;
	os << itsMaskSettings << endl;
	os << "// IsPrinterPortrait" << endl;
	os << fIsPrinterPortrait << endl;
	os << "// fUseBrushTool fUseAnalyzeTool fUseTextGenTool (removed feature legacy storage) fUseChangeSpreaderTool fUseControlPoinTool fUseAnimationTool" << endl;
    // ViewMacrojen eteen/taaksep‰in yhteensopivuuksien takia pit‰‰ tallettaa ja lukea yksi boolean muuttuja
    bool legacy_UseChangeSpreaderTool = false;
	os << fUseBrushTool << " " << fUseAnalyzeTool << " " << false << " " << legacy_UseChangeSpreaderTool << " " << fUseControlPoinTool << " " << fUseAnimationTool << endl;
	os << "// itsAnimationStartPosition itsAnimationEndPosition itsAnimationDelayInMS" << endl;
	os << itsAnimationStartPosition << " " << itsAnimationEndPosition << " " << itsAnimationDelayInMS << endl;

	if(os.fail())
		throw runtime_error(exceptionErrorMessage);

	// ************************************
	// T‰st‰ eteenp‰in on versio 2. tavaraa
	// ************************************
	os << "// Here starts View Macro version 2 stuff" << endl;
	os << itsTrajectoryView << endl;
	os << itsCrossSectionView << endl;
	os << itsSynopPlotSettings<< endl;
	os << itsObsComparisonInfo<< endl;
	os << itsWarningCenterView << endl;
	os << itsSynopDataGridView << endl;

	os << "// ExtraMapView(s) settings are here" << endl;
	size_t ssize = itsExtraMapViewDescTops.size();
	os << "// ExtraMapView size" << endl;
	os << ssize << endl;
	for(size_t i=0; i < ssize; i++)
	{
		os << itsExtraMapViewDescTops[i] << endl;
	}

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error(exceptionErrorMessage);
}

void NFmiViewSettingMacro::Read(std::istream& is)
{
	is >> itsCurrentVersionNumber;
	if(itsCurrentVersionNumber > itsLatestVersionNumber)
		throw std::runtime_error("NFmiViewSettingMacro::Read failed the version number war newer than program can handle.");
	itsOriginalLoadVersionNumber = itsCurrentVersionNumber; // laitetaan originaali versio talteen

    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Read failed";

	NFmiString tmp1;
	is >> tmp1;
	itsName = tmp1;
	NFmiString tmp2;
	is >> tmp2;
	itsDescription = tmp2;
	is >> itsGeneralDoc;
	is >> itsTimeView;

	itsTempView.CurrentVersionNumber(itsCurrentVersionNumber); // versio numeroa pit‰‰ jakaa eteenp‰in
	is >> itsTempView;
	is >> itsMaskSettings;
	is >> fIsPrinterPortrait;
    bool removedLegacyUseGsmToolflag = false;
    // ViewMacrojen eteen/taaksep‰in yhteensopivuuksien takia pit‰‰ tallettaa ja lukea yksi boolean muuttuja
    bool legacy_UseChangeSpreaderTool = false;
    is >> fUseBrushTool >> fUseAnalyzeTool >> removedLegacyUseGsmToolflag >> legacy_UseChangeSpreaderTool >> fUseControlPoinTool >> fUseAnimationTool;
	is >> itsAnimationStartPosition >> itsAnimationEndPosition >> itsAnimationDelayInMS;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	if(itsCurrentVersionNumber > 1.0)
	{
		// luetaan uudet ver 2.0 ja myˆh. jutut
		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsTrajectoryView;
		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsCrossSectionView;
		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsSynopPlotSettings;
		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsObsComparisonInfo;

		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsWarningCenterView;

		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsSynopDataGridView;

		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		size_t ssize = 0;
		is >> ssize;
		itsExtraMapViewDescTops.resize(ssize);
		for(size_t i=0; i < ssize; i++)
		{
			is >> itsExtraMapViewDescTops[i];
		}

		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
		is >> extraData;
		// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
		// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.
	}
	else
	{
		// muuten tehd‰‰n sopivia alustuksia uusiin muuttujiin

		// itsTrajectoryView ; // ‰l‰ koske trajektori-systeemiin kuitenkaan
		// itsCrossSectionView ; // ‰l‰ koske crosssection-systeemiin kuitenkaan
		// itsSynopPlotSettings ; // ‰l‰ koske synop-plot-asetuksiin kuitenkaan
		// itsWarningCenterView ; // ‰l‰ koske warning-center-asetuksiin kuitenkaan
		// itsSynopDataGridView ; // ‰l‰ koske synop-taulukko-asetuksiin kuitenkaan
	}

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	itsCurrentVersionNumber = itsLatestVersionNumber; // aina jatketaan viimeisell‰ versio numerolla
}

void NFmiViewSettingMacro::SetMacroParamInitFileNames(const std::string &theRootPath)
{
	itsCrossSectionView.SetMacroParamInitFileNames(theRootPath);
	for(size_t i=0; i<itsExtraMapViewDescTops.size(); i++)
		itsExtraMapViewDescTops[i].SetMacroParamInitFileNames(theRootPath);
}

// ****************************************
// *** MapViewDescTop osio alkaa t‰st‰ ****
// ****************************************

NFmiViewSettingMacro::MapViewDescTop::MapViewDescTop(void)
:itsMapRowSettings()
,itsMapViewDescTop()
,itsAbsolutRect()
,itsViewStatus()
,itsDipMapHelperList()
{
}

NFmiViewSettingMacro::MapViewDescTop::~MapViewDescTop(void)
{
}

void NFmiViewSettingMacro::MapViewDescTop::SetAllRowParams(NFmiPtrList<NFmiDrawParamList> *theDrawParamListVector, NFmiMacroParamSystem& theMacroParamSystem)
{
	Clear(itsMapRowSettings);
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = theDrawParamListVector->Start();
	for( ; iter.Next(); )
		Add(itsMapRowSettings, MakeMapRow(iter.CurrentPtr(), theMacroParamSystem));
}

void NFmiViewSettingMacro::MapViewDescTop::SetMacroParamInitFileNames(const std::string &theRootPath)
{
	std::for_each(itsMapRowSettings.begin(), itsMapRowSettings.end(), SetMacroParamInitFileNamesFunctor(theRootPath));
}

void NFmiViewSettingMacro::MapViewDescTop::Clear(checkedVector<MapRow> &theMapRowSettings)
{
	theMapRowSettings.clear();
}

void NFmiViewSettingMacro::MapViewDescTop::Add(checkedVector<MapRow> &theMapRowSettings, const MapRow &theMapRow)
{
	theMapRowSettings.push_back(theMapRow);
}

void NFmiViewSettingMacro::MapViewDescTop::SetMapViewDescTop(const NFmiMapViewDescTop &theData, NFmiMapViewWinRegistry &theMapViewWinRegistry)
{
	itsMapViewDescTop.InitForViewMacro(theData, theMapViewWinRegistry ,true);
}

// tutkii miss‰ n‰yttˆ riviss‰ on viimeiset parametrit. T‰ll‰ pyrit‰‰n s‰‰st‰m‰‰n
// talletuksissa, ett‰ jos vaikka 3. rivin j‰lkeen ei ole parametreja, ei tallleteta tyhji‰ rivej‰ sen j‰lkeen,
// koska tyhj‰tkin rivit viev‰t tilaa.
static int CalcStoredMapRowCount(const checkedVector<NFmiViewSettingMacro::MapRow> &theMapRowSettings)
{
	size_t totalSize = theMapRowSettings.size();
	size_t lastRowWithParams = 0;
	for(size_t i=0; i < totalSize; i++)
	{
		if(theMapRowSettings[i].RowParams().size() > 0)
			lastRowWithParams = i+1; // pit‰‰ lis‰t‰ 1, koska indeksit alkavat 0:sta
	}
	return static_cast<int>(lastRowWithParams);
}

void NFmiViewSettingMacro::MapViewDescTop::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::MapViewDescTop::Write..." << endl;
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status fShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;

	os << "// vector<MapRow> MapRowSettings" << endl;
    // Talletetaan rivej‰ vain niin pitk‰lle kuin sielt‰ lˆytyy jotain talletettavaa (nyt siis rivej‰ voi potentiaalisesti olla aina 50 kpl)
    int mapRowWriteSize = ::CalcStoredMapRowCount(itsMapRowSettings); 
    NFmiDataStoringHelpers::WriteContainer(itsMapRowSettings, os, string("\n"), mapRowWriteSize);

	os << "// vector<MapRow> ExtraMapRowSettings" << endl;
    checkedVector<MapRow> emptyLegacyMapRowSettings; //  Pakko tallettaa tyhj‰ vektori n‰yttˆmakroon taaksep‰in yhteensopivuuden takia
    NFmiDataStoringHelpers::WriteContainer(emptyLegacyMapRowSettings, os, string("\n"), 0);

	os << "// MapViewDescTop" << endl;
	os << itsMapViewDescTop << endl;

	os << "// DipMapHelperList" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsDipMapHelperList, os, "\n");

	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	// Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
	// edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::MapViewDescTop::Write ep‰onnistui");

}

// T‰m‰ on legacy funktio, jolla siirret‰‰n legacy-extraMapRow:iin talletetut rivit p‰‰vektoriin.
// Jos theLegacyExtraMapRowSettings vektorissa on tavaraa, siirret‰‰n ne theMapRowSettings:in per‰‰n.
// Jos theMapRowSettings:in koko on pienempi kuin preferredSizeAfterExtraRowsAreAppended, kasvata sen kokoa annettuun lukuun.
static void CombineMapRowSettings(checkedVector<NFmiViewSettingMacro::MapRow> &theMapRowSettings, checkedVector<NFmiViewSettingMacro::MapRow> &theLegacyExtraMapRowSettings, int preferredSizeAfterExtraRowsAreAppended)
{
    if(theLegacyExtraMapRowSettings.size())
    {
        if(theMapRowSettings.size() > preferredSizeAfterExtraRowsAreAppended)
            return; // T‰m‰ on joku virhetilanne, enk‰ tee mit‰‰n
        if(theMapRowSettings.size() < preferredSizeAfterExtraRowsAreAppended)
            theMapRowSettings.resize(preferredSizeAfterExtraRowsAreAppended);
        theMapRowSettings.insert(theMapRowSettings.end(), theLegacyExtraMapRowSettings.begin(), theLegacyExtraMapRowSettings.end());
    }
}

void NFmiViewSettingMacro::MapViewDescTop::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::MapViewDescTop::Read failed";
    // toivottavasti olet poistanut kommentit luettavasta streamista!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
    if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsMapRowSettings, is);
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

    checkedVector<MapRow> legacyExtraMapRowSettings; //  Pakko lukea n‰yttˆmakroista mahdolliset extraParamit taaksep‰in yhteensopivuuden takia
	NFmiDataStoringHelpers::ReadContainer(legacyExtraMapRowSettings, is);
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
    ::CombineMapRowSettings(itsMapRowSettings, legacyExtraMapRowSettings, 5);

	is >> itsMapViewDescTop;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsDipMapHelperList, is);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
	is >> extraData;
	// T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

}

// *****************************************
// *** MapViewDescTop osio loppuu t‰h‰n ****
// *****************************************
