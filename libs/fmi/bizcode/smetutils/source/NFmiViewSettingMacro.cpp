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
    // Search one character at a time until a non-space is encountered (assumption: the stream always starts at the beginning of a line)
    } while(std::isspace(ch));
    if(ch == '/' || ch == '#')
    {
        // If there was a comment character at the start of the line (after the spaces), read the rest of the line away
        ::SkipRestOfTheLine(is);
        return true;
    }
    else
    {
        // If there was something else at the start, put the character back into the stream
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

// This is read from the raw file, without removing comments and without first reading the whole file into a stream in memory.
void NFmiLightWeightViewSettingMacro::Read(std::istream& is)
{
    fViewMacroOk = true;
    double versionNumber = -1;
    // First read the version number out of the way
    if(::GetNextNumberFromViewMacroFile(is, versionNumber))
    {
        ::SkipRestOfTheLine(is);
        double nameStringLength = -1;
        // Then read the number related to the name string
        if(::GetNextNumberFromViewMacroFile(is, nameStringLength))
        {
            ::SkipRestOfTheLine(is);
            double descriptionStringLength = -1;
            // Then read the number related to the description string
            if(::GetNextNumberFromViewMacroFile(is, descriptionStringLength))
            {
                int stringSize = static_cast<int>(descriptionStringLength);
                if(stringSize > 0)
                {
                    if(stringSize > 4000)
                        // Limit the size of the read description to 4000 characters
                        stringSize = 4000;
                    char ch;
                    // Read the space away
                    is.get(ch);
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
	// I.e. if this is a macroParam, its initFileName must be set correctly, so that
	// the macroParams can be loaded correctly from subdirectories too.
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
    // FShowTimeDifference is removed option, storing dummy value for backward compatibility
	os << fHidden << " " << fActive << " " << false << " " << fShowDifferenceToOriginalData << endl;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::Param::Write epäonnistui");
}

void NFmiViewSettingMacro::Param::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Param::Read failed";
    // Hopefully you have removed the comments from the stream being read!!
	if(is)
		is >> *itsDrawParam;
	itsDrawParam->ViewMacroDrawParam(true);
	if(is)
		is >> itsDataIdent;
	if(is)
		// The parameter must also be set correctly into the drawParam, otherwise all hell breaks loose
		itsDrawParam->Param(itsDataIdent);
	if(is)
		is >> itsLevel;
	if(is)
		// The level must also be set here right away
		itsDrawParam->Level(itsLevel);
	if(is)
	{
		int tmp = 0;
		is >> tmp;
		itsDataType = static_cast<NFmiInfoData::Type>(tmp);
	}
	if(is)
		is >> itsModelOrigTimeOffsetInHours;
    if(is)
    {
        // FShowTimeDifference is removed option, reading dummy value for backward compatibility
        bool removedOption_fShowTimeDifference = false;
		is >> fHidden >> fActive >> removedOption_fShowTimeDifference >> fShowDifferenceToOriginalData;
    }

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

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

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::Mask::Write epäonnistui");
}

void NFmiViewSettingMacro::Mask::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Mask::Read failed";
    // Hopefully you have removed the comments from the stream being read!!
	is >> itsParamSettings;
	is >> itsMaskSettings;
	is >> fMaskEnabled;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.
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

NFmiViewSettingMacro::MaskSettings::MaskSettings(const std::vector<Mask> &theMasks, bool theShowMasksOnMapView, bool theUseMasksInTimeSerialViews, bool theUseMasksWithFilterTool, bool theUseMaskWithBrush)
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
	os << "// std::vector<Mask>" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsMasks, os, string("\n"));
	os << "// ShowMasksOnMapView UseMasksInTimeSerialViews fUseMasksWithFilterTool fUseMaskWithBrush" << endl;
	os << fShowMasksOnMapView << " " << fUseMasksInTimeSerialViews << " " << fUseMasksWithFilterTool << " " << fUseMaskWithBrush << endl;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::MaskSettings::Write epäonnistui");
}

void NFmiViewSettingMacro::MaskSettings::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::MaskSettings::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
	NFmiDataStoringHelpers::ReadContainer(itsMasks, is);
	is >> fShowMasksOnMapView >> fUseMasksInTimeSerialViews >> fUseMasksWithFilterTool >> fUseMaskWithBrush;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
}

NFmiViewSettingMacro::MapRow::MapRow(void)
:itsRowParams()
{
}

NFmiViewSettingMacro::MapRow::MapRow(const std::vector<NFmiViewSettingMacro::Param>& theParams)
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
	os << "// std::vector<Param> itsRowParams" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsRowParams, os, string("\n"));

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::MapRow::Write epäonnistui");
}

void NFmiViewSettingMacro::MapRow::Read(std::istream& is)
{
	// Hopefully you have removed the comments from the stream being read!!
	NFmiDataStoringHelpers::ReadContainer(itsRowParams, is);
	if(is.fail())
		throw runtime_error("NFmiViewSettingMacro::MapRow::Read epäonnistui");
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

std::string removeComments(const std::string &prgm)
{
	int n = static_cast<int>(prgm.length());
	std::string res;

	// Flags to indicate that single line and multiple line comments
	// have started or not.
	bool s_cmt = false;
	bool m_cmt = false;


	// Traverse the given program
	for(int i = 0; i < n; i++)
	{
		// If single line comment flag is on, then check for end of it
		if(s_cmt == true && prgm[i] == '\n')
			s_cmt = false;

		// If multiple line comment is on, then check for end of it
		else if(m_cmt == true && prgm[i] == '*' && prgm[i + 1] == '/')
			m_cmt = false, i++;

		// If this character is in a comment, ignore it
		else if(s_cmt || m_cmt)
			continue;

		// Check for beginning of comments and set the approproate flags
		else if(prgm[i] == '/' && prgm[i + 1] == '/')
			s_cmt = true, i++;
		else if(prgm[i] == '/' && prgm[i + 1] == '*')
			m_cmt = true, i++;

		// If current character is a non-comment character, append it to res
		else  res += prgm[i];
	}
	return res;
}

void NFmiViewSettingMacro::TimeViewRow::SetMacroParamInitFileNames(const std::string& theRootPath)
{
	SetMacroParamInitFileNamesFunctor setterFunctor(theRootPath);
	setterFunctor(itsParam);
	std::for_each(itsSideParameters.begin(), itsSideParameters.end(), SetMacroParamInitFileNamesFunctor(theRootPath));
}

void NFmiViewSettingMacro::TimeViewRow::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::TimeViewRow::Write..." << endl;
	os << "// Param" << endl;
	os << itsParam << endl;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.

	// 1. an added feature is a list of side-parameters as a single string (NOTE! an empty list must also be added)
	std::stringstream stringOut;
	NFmiDataStoringHelpers::WriteContainer(itsSideParameters, stringOut, string("\n"));
	// Note! all comments must also be removed from the string to be stored first, otherwise it won't work!!!
	extraData.Add(::removeComments(stringOut.str()));

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TimeViewRow::Write epäonnistui");
}

void NFmiViewSettingMacro::TimeViewRow::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TimeViewRow::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
	is >> itsParam;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

	// 1. an added feature is a list of side-parameters as a single string
	itsSideParameters.clear();
	if(extraData.itsStringValues.size() >= 1)
	{
		std::stringstream stringIn(extraData.itsStringValues[0]);
		NFmiDataStoringHelpers::ReadContainer(itsSideParameters, stringIn);
	}

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
		{ 
            // This is ugly code, but I couldn't think of anything better right now. I.e. the relative path of the macroParam in the viewMacro
			// must be stored, which is saved into the drawParam. But this drawParam exists in two places
			// in a slightly different form, and the correct relative path can only be obtained from the macroParam's DrawParam found in the MacroParamSystem
			// and not from the DrawParamList found in the document.
            auto macroParamPtr = theMacroParamSystem.GetWantedMacro(drawParam->InitFileName());
			if(macroParamPtr)
				drawParam->MacroParamRelativePath(macroParamPtr->DrawParam()->MacroParamRelativePath());
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

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::GeneralDoc::Write epäonnistui");
}

void NFmiViewSettingMacro::GeneralDoc::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::GeneralDoc::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
	is >> itsProjectionCurvatureInfo;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsCPLocationVector, is);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

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
	os << "// std::vector<MapRow> itsMapRowSettings" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsMapRowSettings, os, string("\n"));
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status fShowWindow" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus) << endl;

	os << "// CrossSectionSystem" << endl;
	os << itsCrossSectionSystem;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::CrossSectionView::Write epäonnistui");

}

void NFmiViewSettingMacro::CrossSectionView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::CrossSectionView::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
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
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

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

static std::vector<NFmiViewSettingMacro::Param> MakeParamVector(NFmiDrawParamList& theDrawParamList)
{
	std::vector<NFmiViewSettingMacro::Param> paramVector;
	for(theDrawParamList.Reset(); theDrawParamList.Next(); )
	{
		auto drawParam = theDrawParamList.Current();
		NFmiViewSettingMacro::Param param(drawParam, drawParam->Level(), drawParam->DataType(), 0);
		paramVector.push_back(param);
	}
	return paramVector;
}

void NFmiViewSettingMacro::TimeView::SetAllSideParameters(CombinedMapHandlerInterface::SideParametersContainer& theSideParameterList)
{
	size_t rowIndex = 0;
	for(const auto& sideParameters : theSideParameterList)
	{
		if(rowIndex < itsRows.size())
		{
			itsRows[rowIndex].SideParameters(::MakeParamVector(*sideParameters));
		}
		rowIndex++;
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

void NFmiViewSettingMacro::TimeView::SetMacroParamInitFileNames(const std::string& theRootPath)
{
	std::for_each(itsRows.begin(), itsRows.end(), SetMacroParamInitFileNamesFunctor(theRootPath));
}

void NFmiViewSettingMacro::TimeView::Write(std::ostream& os) const
{
	os << "// NFmiViewSettingMacro::TimeView::Write..." << endl;
	os << "// std::vector<TimeViewRow> itsRows" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsRows, os, string("\n"));
	os << "// AbsolutRect" << endl;
	os << itsAbsolutRect;
	os << "// Status ShowWindow ShowHelpData" << endl;
    ::WriteMfcViewStatus(os, itsViewStatus);
	os << " " << fShowHelpData << endl;
	os << "// itsStartTimeOffset itsEndTimeOffset" << endl;
	os << itsStartTimeOffset << " " << itsEndTimeOffset << endl;

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.

    // Add the helpData2, 3 and 4 settings to the view macro
    extraData.Add(static_cast<double>(fShowHelpData2));
    extraData.Add(static_cast<double>(fShowHelpData3));
    extraData.Add(static_cast<double>(fShowHelpData4));

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	std::string timeBagStr = NFmiDataStoringHelpers::GetTimeBagOffSetStr(usedViewMacroTime, itsTimeBag);
	// Add as the 1st extra string data the time window's timebag as an offset to the current time
	extraData.Add(timeBagStr);
	if(itsPreciseTimeSerialLatlonPoint != NFmiPoint::gMissingLatlon)
	{
		// Add as the 2nd extra string data the position of the latlon point selected for the time series
		extraData.Add(CtrlViewUtils::Point2String(itsPreciseTimeSerialLatlonPoint)); 
	}
	
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TimeView::Write epäonnistui");
}

void NFmiViewSettingMacro::TimeView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TimeView::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
	NFmiDataStoringHelpers::ReadContainer(itsRows, is);
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
	is >> fShowHelpData;
	is >> itsStartTimeOffset >> itsEndTimeOffset;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

    // The helpData2 and 3 settings from the view macro
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
	{
		// Read as the 1st extra data the time window's timebag as an offset to the current time
		if(is.fail())
			throw std::runtime_error(exceptionErrorMessage);
		NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
		itsTimeBag = NFmiDataStoringHelpers::GetTimeBagOffSetFromStr(usedViewMacroTime, extraData.itsStringValues[0]);
		fTimeBagUpdated = true;
	}

	// By default the latlon point is set to missing, in which case the current point in use is left in effect.
	itsPreciseTimeSerialLatlonPoint = NFmiPoint::gMissingLatlon;
	if(extraData.itsStringValues.size() >= 2)
	{
		// Read as the 2nd extra string data the latlon point selected for the time window.
		if(is.fail())
			throw std::runtime_error(exceptionErrorMessage);
		itsPreciseTimeSerialLatlonPoint = CtrlViewUtils::String2Point(extraData.itsStringValues[1]);
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

void NFmiViewSettingMacro::TempView::MTATempSystem(const NFmiMTATempSystem &theData, bool disableWindowManipulations)
{
	itsMTATempSystem.InitFromViewMacro(theData, disableWindowManipulations);
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
	// From here onwards is version 2 stuff
	// ************************************
	os << "// MTATempSystem" << endl;
	os << itsMTATempSystem << endl;
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TempView::Write epäonnistui");
}

void NFmiViewSettingMacro::TempView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TempView::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
    is >> fShowHirlam >> fShowEcmwf >> fShowRealSounding;

	if(itsCurrentVersionNumber > 1.0)
	{
		// Read the new ver 2.0 and later stuff
		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		is >> itsMTATempSystem;

		if(is.fail())
			throw runtime_error(exceptionErrorMessage);
		// Finally, the possible extra data
		NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
		is >> extraData;
		// Here new variables that may have appeared are then taken from the extra data
		// i.e. if there are new variables or values, handle them here.
	}
	else
	{
		// Otherwise make suitable initializations for the new variables
		// go with the default values of the itsMTATempSystem object
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
	itsTrajectorySystem.Init(theData, false);
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

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::TrajectoryView::Write epäonnistui");
}

void NFmiViewSettingMacro::TrajectoryView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::TrajectoryView::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	is >> itsTrajectorySystem;
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

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

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.

    // FShowHakeMessages is the 1st new 'double' value
    extraData.Add(static_cast<double>(fShowHakeMessages));
    // FShowKaHaMessages is the 2nd new 'double' value
    extraData.Add(static_cast<double>(fShowKaHaMessages));
    // ItsMinimumTimeRangeForWarningsOnMapViewsInMinutes is the 3rd new 'double' value
    extraData.Add(static_cast<double>(itsMinimumTimeRangeForWarningsOnMapViewsInMinutes));

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::WarningCenterView::Write epäonnistui");
}

void NFmiViewSettingMacro::WarningCenterView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::WarningCenterView::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
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
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

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

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::SynopDataGridView::Write epäonnistui");
}

void NFmiViewSettingMacro::SynopDataGridView::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::SynopDataGridView::Read failed";

    // Hopefully you have removed the comments from the stream being read!!
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
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

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

	// Extra-data storage is not used here, because it is already in the NFmiSynopPlotSettings class
// Finally, the possible extra data
//	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::SynopPlotSettings::Write epäonnistui");
}

void NFmiViewSettingMacro::SynopPlotSettings::Read(std::istream& is)
{
	is >> itsSynopPlotSettings;
	if(is.fail())
		throw runtime_error("NFmiViewSettingMacro::SynopPlotSettings::Read epäonnistui");
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

	// Extra-data storage is not used here, because it is already in the NFmiSynopPlotSettings class
// Finally, the possible extra data
//	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::ObsComparisonInfo::Write epäonnistui");
}

void NFmiViewSettingMacro::ObsComparisonInfo::Read(std::istream& is)
{
	is >> itsObsComparisonInfo;
	if(is.fail())
		throw runtime_error("NFmiViewSettingMacro::ObsComparisonInfo::Read epäonnistui");
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
,fKeepMapAspectRatio(false)
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
		// The directory hack is not meant to be stored
		return ;

	os << "// NFmiViewSettingMacro::Write..." << endl;
	os << "// version number" << endl;
	// Always written with the latest version number
	itsCurrentVersionNumber = itsLatestVersionNumber;
	os << itsCurrentVersionNumber << endl;
	os << "// itsName" << endl;
	// I store itsName and itsDescription as NFmiStrings, so that reading and writing work ok even if there are white spaces
	NFmiString tmp1(itsName);
	// Must replace possible comment characters, because comments are removed when reading and then we'd be in trouble
	ChangePossibleComments(tmp1);
	os << tmp1;
	os << "// Description" << endl;
	NFmiString tmp2(itsDescription);
	// Must replace possible comment characters, because comments are removed when reading and then we'd be in trouble
	ChangePossibleComments(tmp2);
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
    // For backward/forward compatibility of view macros, one boolean variable must be stored and read
    bool legacy_UseChangeSpreaderTool = false;
	os << fUseBrushTool << " " << fUseAnalyzeTool << " " << false << " " << legacy_UseChangeSpreaderTool << " " << fUseControlPoinTool << " " << fUseAnimationTool << endl;
	os << "// itsAnimationStartPosition itsAnimationEndPosition itsAnimationDelayInMS" << endl;
	os << itsAnimationStartPosition << " " << itsAnimationEndPosition << " " << itsAnimationDelayInMS << endl;

	if(os.fail())
		throw runtime_error(exceptionErrorMessage);

	// ************************************
	// From here onwards is version 2 stuff
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

    // Finally, the possible extra data:
    // When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
    // previous versions don't get tangled up even though new data has appeared.
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; 
    // KeepMapAspectRatio (the F10 function in SmartMet) is thus the 1st of the new double extra parameters
    extraData.Add(fKeepMapAspectRatio);

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
	// Store the original version
	itsOriginalLoadVersionNumber = itsCurrentVersionNumber;

    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::Read failed";

	NFmiString tmp1;
	is >> tmp1;
	itsName = tmp1;
	NFmiString tmp2;
	is >> tmp2;
	itsDescription = tmp2;
	is >> itsGeneralDoc;
	is >> itsTimeView;

	// The version number must be passed forward
	itsTempView.CurrentVersionNumber(itsCurrentVersionNumber);
	is >> itsTempView;
	is >> itsMaskSettings;
	is >> fIsPrinterPortrait;
    bool removedLegacyUseGsmToolflag = false;
    // For backward/forward compatibility of view macros, one boolean variable must be stored and read
    bool legacy_UseChangeSpreaderTool = false;
    is >> fUseBrushTool >> fUseAnalyzeTool >> removedLegacyUseGsmToolflag >> legacy_UseChangeSpreaderTool >> fUseControlPoinTool >> fUseAnimationTool;
	is >> itsAnimationStartPosition >> itsAnimationEndPosition >> itsAnimationDelayInMS;

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	if(itsCurrentVersionNumber > 1.0)
	{
		// Read the new ver 2.0 and later stuff
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
		// Finally, the possible extra data
		NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
		is >> extraData;
		// Here new variables that may have appeared are then taken from the extra data
		// i.e. if there are new variables or values, handle them here.

        // This is thus the default value for KeepMapAspectRatio (i.e. don't force keeping the correct map aspect ratios)
        fKeepMapAspectRatio = false;
        if(extraData.itsDoubleValues.size() >= 1)
            fKeepMapAspectRatio = extraData.itsDoubleValues[0] != 0;
    }
	else
	{
		// Otherwise make suitable initializations for the new variables

		// Don't touch the trajectory system after all
		// itsTrajectoryView ;
		// Don't touch the crosssection system after all
		// itsCrossSectionView ;
		// Don't touch the synop-plot settings after all
		// itsSynopPlotSettings ;
		// Don't touch the warning-center settings after all
		// itsWarningCenterView ;
		// Don't touch the synop-table settings after all
		// itsSynopDataGridView ;
	}

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

	// Always continue with the latest version number
	itsCurrentVersionNumber = itsLatestVersionNumber;
}

// If macroParam support is to be added to some other views as well,
// the initializations of that view's macroParam paths must be added here.
void NFmiViewSettingMacro::SetMacroParamInitFileNames(const std::string &theRootPath)
{
	itsTimeView.SetMacroParamInitFileNames(theRootPath);
	itsCrossSectionView.SetMacroParamInitFileNames(theRootPath);
	for(size_t i=0; i<itsExtraMapViewDescTops.size(); i++)
		itsExtraMapViewDescTops[i].SetMacroParamInitFileNames(theRootPath);
}

// ****************************************
// *** MapViewDescTop section starts here ****
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

void NFmiViewSettingMacro::MapViewDescTop::Clear(std::vector<MapRow> &theMapRowSettings)
{
	theMapRowSettings.clear();
}

void NFmiViewSettingMacro::MapViewDescTop::Add(std::vector<MapRow> &theMapRowSettings, const MapRow &theMapRow)
{
	theMapRowSettings.push_back(theMapRow);
}

void NFmiViewSettingMacro::MapViewDescTop::SetMapViewDescTop(const NFmiMapViewDescTop &theData, NFmiMapViewWinRegistry &theMapViewWinRegistry, bool disableWindowManipulations)
{
	itsMapViewDescTop.InitForViewMacro(theData, theMapViewWinRegistry ,true, disableWindowManipulations);
}

// Examines which view row has the last parameters. This aims to save
// on storage, so that if e.g. after the 3rd row there are no parameters, empty rows after that are not stored,
// because even empty rows take up space.
static int CalcStoredMapRowCount(const std::vector<NFmiViewSettingMacro::MapRow> &theMapRowSettings)
{
	size_t totalSize = theMapRowSettings.size();
	size_t lastRowWithParams = 0;
	for(size_t i=0; i < totalSize; i++)
	{
		if(theMapRowSettings[i].RowParams().size() > 0)
			// Must add 1, because indices start from 0
			lastRowWithParams = i+1;
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
    // Store rows only as far as there is something to store (so now there can potentially be up to 50 rows)
    int mapRowWriteSize = ::CalcStoredMapRowCount(itsMapRowSettings); 
    NFmiDataStoringHelpers::WriteContainer(itsMapRowSettings, os, string("\n"), mapRowWriteSize);

	os << "// vector<MapRow> ExtraMapRowSettings" << endl;
    // Must store an empty vector to the view macro for backward compatibility
    std::vector<MapRow> emptyLegacyMapRowSettings;
    NFmiDataStoringHelpers::WriteContainer(emptyLegacyMapRowSettings, os, string("\n"), 0);

	os << "// MapViewDescTop" << endl;
	os << itsMapViewDescTop << endl;

	os << "// DipMapHelperList" << endl;
	NFmiDataStoringHelpers::WriteContainer(itsDipMapHelperList, os, "\n");

	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	// When new variables appear, add extra-data filling here, so that it gets stored to the file in such a way that
	// previous versions don't get tangled up even though new data has appeared.
	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw runtime_error("NFmiViewSettingMacro::MapViewDescTop::Write epäonnistui");

}

// This is a legacy function that moves the rows stored in the legacy extraMapRow into the main vector.
// If there is stuff in the theLegacyExtraMapRowSettings vector, move it to the end of theMapRowSettings.
// If the size of theMapRowSettings is smaller than preferredSizeAfterExtraRowsAreAppended, grow its size to the given number.
static void CombineMapRowSettings(std::vector<NFmiViewSettingMacro::MapRow> &theMapRowSettings, std::vector<NFmiViewSettingMacro::MapRow> &theLegacyExtraMapRowSettings, int preferredSizeAfterExtraRowsAreAppended)
{
    if(theLegacyExtraMapRowSettings.size())
    {
        if(theMapRowSettings.size() > preferredSizeAfterExtraRowsAreAppended)
            // This is some error situation, and I do nothing
            return;
        if(theMapRowSettings.size() < preferredSizeAfterExtraRowsAreAppended)
            theMapRowSettings.resize(preferredSizeAfterExtraRowsAreAppended);
        theMapRowSettings.insert(theMapRowSettings.end(), theLegacyExtraMapRowSettings.begin(), theLegacyExtraMapRowSettings.end());
    }
}

void NFmiViewSettingMacro::MapViewDescTop::Read(std::istream& is)
{ 
    static const std::string exceptionErrorMessage = "NFmiViewSettingMacro::MapViewDescTop::Read failed";
    // Hopefully you have removed the comments from the stream being read!!
	is >> itsAbsolutRect;
    ::ReadMfcViewStatus(is, itsViewStatus);
    if(is.fail())
		throw runtime_error(exceptionErrorMessage);
	NFmiDataStoringHelpers::ReadContainer(itsMapRowSettings, is);
	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

    // Must read the possible extraParams from the view macros for backward compatibility
    std::vector<MapRow> legacyExtraMapRowSettings;
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
	// Finally, the possible extra data
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData;
	is >> extraData;
	// Here new variables that may have appeared are then taken from the extra data
	// i.e. if there are new variables or values, handle them here.

	if(is.fail())
		throw runtime_error(exceptionErrorMessage);

}

// *****************************************
// *** MapViewDescTop section ends here ****
// *****************************************
