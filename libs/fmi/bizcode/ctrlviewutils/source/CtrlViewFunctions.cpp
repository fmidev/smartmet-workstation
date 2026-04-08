#include "CtrlViewFunctions.h"
#include "NFmiStringTools.h"
#include "NFmiAngle.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiColor.h"
#include "NFmiRect.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiProducerSystem.h"
#include "ColorStringFunctions.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParam.h"
#include "NFmiPathUtils.h"
#include "catlog/catlog.h"

#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/round.hpp>
#include <filesystem>
#include <regex>
#include <numeric>

#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace CtrlViewUtils
{
    NFmiPoint CalcTimeScaleFontSizeInPixels(double thePixelsPerMMinX)
    {
        static const double statScaleFontSizeInMM = 2.6;
        int fontSizeY = static_cast<int>(statScaleFontSizeInMM * thePixelsPerMMinX * 1.88);
        return NFmiPoint(fontSizeY, fontSizeY);
    }

    std::string GetLatLonValue2MinuteStr(double value, int decimals) // voi olla lat/lon arvo josta on jo otettu absoluuttiarvo
    {

        int valueInt = static_cast<int>(value);
        double valueRemainder = value - valueInt;
        double minutes = valueRemainder*60.;
        if(decimals)
            minutes = round(minutes * 10) / 10.; // n�in saadaan py�ristettyn� arvona luku yhden desimaalin tarkkuudella
        else
            minutes = round(minutes); // ei desimaaleja mukaan
        if(minutes >= 60.)
        { // jos minuutit py�ristyiv�t 60:een, pit�� lis�t� asteisiin 1 ja laittaa minuutit 0:ksi
            valueInt += 1;
            minutes = 0;
        }

        std::string retStr;
        retStr += NFmiStringTools::Convert<int>(valueInt);
        retStr += "�";
        size_t strSize = retStr.size();
        if(minutes < 10.)
            retStr += "0"; // lis�t��n etu nolla
        retStr += NFmiStringTools::Convert<double>(minutes);
        if(decimals)
        {
            if(retStr.size() - strSize == 1)
                retStr += "0"; // lis�t��n desimaali nolla
            else if(retStr.size() - strSize == 2)
                retStr += ".0"; // lis�t��n desimaali piste ja nolla
        }
        retStr += "'";

        return retStr;
    }

    std::string GetLatitudeMinuteStr(double lat, int decimals)
    {
        bool northernSide = lat >= 0.;
        lat = ::fabs(lat);
        std::string retStr;
        if(lat < 10.)
            retStr += "0"; // lis�t��n tarvittaessa etu nolla
        retStr += GetLatLonValue2MinuteStr(lat, decimals);
        if(northernSide)
            retStr += "N";
        else
            retStr += "S";

        return retStr;
    }

    static double FixLongitudeToEuropeCenteredValue(double lon)
    {
        if(lon > 180 && lon <= 360)
        { // fiksataan longitudet, jos ollaan PacificView maailmassa ja ollaan yli 180 asteen takaisin -180 - 180 maailmaan
            NFmiLongitude longitude(lon, false);
            lon = longitude.Value();
        }
        return lon;
    }

    std::string GetLongitudeMinuteStr(double lon, int decimals)
    {
        lon = FixLongitudeToEuropeCenteredValue(lon);
        bool eastSide = (lon >= 0. && lon <= 180.);
        lon = ::fabs(lon);
        std::string retStr;
        if(lon < 10.)
            retStr += "00"; // lis�t��n tarvittaessa etu nollia
        else if(lon < 100.)
            retStr += "0"; // lis�t��n tarvittaessa etu nolla
        retStr += GetLatLonValue2MinuteStr(lon, decimals);
        if(eastSide)
            retStr += "E";
        else
            retStr += "W";

        return retStr;
    }

    std::string MakeSizeString(const NFmiPoint &theSize)
    {
        if(std::isnormal(theSize.X()) && std::isnormal(theSize.Y()))
        {
            std::string str;
            str += boost::lexical_cast<std::string>(boost::math::iround(theSize.X()));
            str += " x ";
            str += boost::lexical_cast<std::string>(boost::math::iround(theSize.Y()));
            return str;
        }
        else
            return "";
    }

    std::string MakeMapPortionPixelSizeStringForStatusbar(const NFmiPoint &theSize, bool longVersion)
    {
        std::string str;
        if(longVersion)
        {
            str += ::GetDictionaryString("View's map portion pixel size is");
            str += " ";
            str += MakeSizeString(theSize);
        }
        else
        {
            str += " (" + MakeSizeString(theSize) + " pix)";
        }
        return str;
    }

    bool AreVectorValuesInRisingOrder(const std::vector<float> &theValueVector)
    {
        size_t ssize = theValueVector.size();
        if(ssize > 1)
        {
            for(size_t i = 0; i < ssize - 1; i++)
            {
                if(theValueVector[i] >= theValueVector[i + 1])
                    return false;
            }
        }
        return true;
    }

    NFmiPoint ConvertPointFromRect1ToRect2(const NFmiPoint &thePointInRect1, const NFmiRect &theRect1, const NFmiRect &theRect2)
    { // palauttaa pisteen joka on samassa suhteessa rect2:ssa kuin annettu piste

      // HUOM!!! ei ole t�ydellisesti tehty, molemmat rect:it alkavat t�ss� 0,0 pisteest�!!!!!!!!!
        return NFmiPoint(thePointInRect1.X() * theRect2.Width() / theRect1.Width(), thePointInRect1.Y() * theRect2.Height() / theRect1.Height());
    }

    // t�m� tekee sellaisen timebagin, miss� on annetut alku ja loopu ajat s��dettyin� l�himp��n aikasteppiin
    // kuitenkin siten ett� maksimi aikasteppi on 60 minuuttia.
    NFmiTimeBag GetAdjustedTimeBag(const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime, int theTimeStepInMinutes)
    {
        int timebagAdjustmentTimeStepInMinutes = (theTimeStepInMinutes > 60) ? 60 : theTimeStepInMinutes; // ei yli 60 minuutin s��t�� lopullisen timebagin aikoihin

        NFmiMetTime newStartTime = theStartTime;
        newStartTime.SetTimeStep(timebagAdjustmentTimeStepInMinutes);
        NFmiMetTime newEndTime = theEndTime;
        newEndTime.SetTimeStep(timebagAdjustmentTimeStepInMinutes);
        if(newStartTime >= newEndTime)
        {
            newEndTime = newStartTime;
            newEndTime.NextMetTime();
        }
        NFmiTimeBag newTimeBag(newStartTime, newEndTime, timebagAdjustmentTimeStepInMinutes);
        return newTimeBag;
    }

    std::string GetIdString(unsigned long theId)
    {
        std::string idStr("(=");
        idStr += NFmiStringTools::Convert<unsigned long>(theId);
        idStr += ")";
        return idStr;
    }

    // Jos annettu positiivinen arvo thelongerProducerNameMaxCharCount -parametrille, 
    // t�ll�in haetaan pitemp�� nime� tuottajalta ja lopuksi se leikataan haluttuun maksimi pituuteen.
    std::string GetProducerName(NFmiProducerSystem &theProducerSystem, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fAddProdId, size_t theLongerProducerNameMaxCharCount)
    {
        bool betaProductCase = theLongerProducerNameMaxCharCount > 0;
        std::string prodNameStr = "X?"; // asetetaan aluksi nimeksi tuntematon tuottaja nimi, laitetaan siihen sitten my�hemmin parempaa, jos l�ytyy
                                        // etsi mallin nimi
        unsigned int modelIndex = theProducerSystem.FindProducerInfo(*(theDrawParam->Param().GetProducer()));
        if(modelIndex > 0)
            prodNameStr = betaProductCase ? theProducerSystem.Producer(modelIndex).Name() : theProducerSystem.Producer(modelIndex).UltraShortName();
        else
        {
            // on tapauksia, miss� drawParamissa ei ole oikeaa tuottajaa (esim. synop-plot), t�ll�in katsotaan l�ytyyk� l�ydetyn infon avulla tuottaja nime�
            if(theInfo)
            {
                unsigned int modelIndex2 = theProducerSystem.FindProducerInfo(*(theInfo->Producer()));
                if(modelIndex2 > 0)
                    prodNameStr = betaProductCase ? theProducerSystem.Producer(modelIndex2).Name() : theProducerSystem.Producer(modelIndex2).UltraShortName();
            }
        }

        if(fAddProdId && theInfo)
            prodNameStr += GetIdString(theInfo->Producer()->GetIdent());

        if(betaProductCase && theLongerProducerNameMaxCharCount < prodNameStr.size())
            prodNameStr.resize(theLongerProducerNameMaxCharCount); // leikataan nime� jos tarvis

        return prodNameStr;
    }

    bool IsConsideredAsNewData(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, int modelRunIndex, bool isMacroParam)
    {
        if(isMacroParam)
        {
            return false;
        }

        return (modelRunIndex == 0 && theInfo && theInfo->ElapsedTimeFromLoadInSeconds() < 5 * 60);
    }

    std::string GetParamNameString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fCrossSectionInfoWanted, bool fAddIdInfos, bool fMakeTooltipXmlEncode, size_t theLongerProducerNameMaxCharCount, bool fTimeSerialViewCase, bool doNewDataHighlight, bool fShowModelOriginTime, boost::shared_ptr<NFmiFastQueryInfo> possibleInfo)
    {
        CtrlViewDocumentInterface* ctrlViewDocumentInterface = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation();
        std::string normalOrigTimeFormat = ::GetDictionaryString("MapViewToolTipOrigTimeNormal");
        std::string minuteOrigTimeFormat = ::GetDictionaryString("MapViewToolTipOrigTimeMinute");
        bool betaProductCase = theLongerProducerNameMaxCharCount > 0;
        NFmiInfoData::Type dataType = theDrawParam->DataType();
        bool isMacroParamCase = theDrawParam->IsMacroParamCase(true);
        std::string str;
        boost::shared_ptr<NFmiFastQueryInfo> info = possibleInfo ? possibleInfo : ctrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, fCrossSectionInfoWanted, true);
        if(doNewDataHighlight && IsConsideredAsNewData(info, theDrawParam->ModelRunIndex(), isMacroParamCase))
        {
            // Korostetaan uudet datat jollain merkill�
            str += ParameterStringHighlightCharacter;
        }

        if(theDrawParam->IsModelRunDataType())
        {
            if(info)
            {
                str += GetProducerName(ctrlViewDocumentInterface->ProducerSystem(), theDrawParam, info, fAddIdInfos, theLongerProducerNameMaxCharCount);

                if(theDrawParam->UseArchiveModelData())
                { // laitetaan arkisto datan kohdalle viittaus verrattuna viimeiseen malli dataan esim. H[-1] joka on siis edellinen malli jne.
                    str += "[";
                    if(theDrawParam->ModelRunIndex() != 0)
                        str += NFmiStringTools::Convert<int>(theDrawParam->ModelRunIndex());
                    else
                        str += "?"; // katsotaan my�hemmin miten absoluutti aika laitetaan
                    str += "]";
                }
                else
                {
                    if(fShowModelOriginTime)
                    {
                        if(betaProductCase)
                            str += " ";
                        str += info->OriginTime().ToStr(NFmiString(normalOrigTimeFormat));
                    }
                }

                if(fTimeSerialViewCase && theDrawParam->TimeSerialModelRunCount() > 0)
                { // laitetaan multimodelrun merkit n�kyviin
                    str += " (";
                    str += NFmiStringTools::Convert<int>(theDrawParam->TimeSerialModelRunCount());
                    str += "x)";
                }

                str += " ";
            }
            else // jos infoa ei l�ytynyt, laitetaan merkit mahdollisen tuottaja nimen ymp�rille sen merkiksi
            {
                str += "#";
                str += GetProducerName(ctrlViewDocumentInterface->ProducerSystem(), theDrawParam, info, fAddIdInfos, theLongerProducerNameMaxCharCount);
                str += "# ";
            }
        }
        else if(dataType == NFmiInfoData::kSingleStationRadarData && info)
        { // t�h�n laitetaan toistaiseksi tuottaja suoraan infon parametrista
            NFmiProducer *prod = info->Producer();
            if(prod)
            {
                str += prod->GetName();
                if(fAddIdInfos)
                    str += GetIdString(prod->GetIdent());
                str += " ";
            }
        }
        else if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kAnalyzeData)
        {
            // etsi havainto tuottajan nimi
            str += GetProducerName(ctrlViewDocumentInterface->ObsProducerSystem(), theDrawParam, info, fAddIdInfos, theLongerProducerNameMaxCharCount);
            str += " ";
        }
        else if(dataType == NFmiInfoData::kEditingHelpData)
        {
            if(betaProductCase)
                str += "HelpEdit";
            else
                str += GetEditingDataString("HE", info, ctrlViewDocumentInterface->Language(), minuteOrigTimeFormat);
        }
        else if(dataType == NFmiInfoData::kKepaData)
        {
            if(betaProductCase)
                str += "Official";
            else
                str += GetEditingDataString("OF", info, ctrlViewDocumentInterface->Language(), minuteOrigTimeFormat);
        }

        bool makeNameErrorMarks = isMacroParamCase ? !ctrlViewDocumentInterface->IsMacroParamOk(theDrawParam) : false;

        if(dataType == NFmiInfoData::kCopyOfEdited)
        {str += "(c)"; }// kopioidut parametrit merkit��n (c):ll�

        if(makeNameErrorMarks)
        {str += "#"; }
        str += theDrawParam->ParameterAbbreviation();
        if(makeNameErrorMarks)
        {str += "#"; }
        if(fAddIdInfos)
            str += GetIdString(theDrawParam->Param().GetParamIdent());

        if(fCrossSectionInfoWanted == false && info && info->SizeLevels() > 1)
        { 
            // muualla kuin poikkileikkauksissa halutaan level infoa, jos datassa on enemm�n kuin 1 leveli
            if(dataType == NFmiInfoData::kHybridData || theDrawParam->Level().LevelType() == kFmiHybridLevel)
            { // laitetaan hybrid datalle parametrin per��n viel "xx", miss� xx on hybrid levelin numero
                str += "_L" + NFmiStringTools::Convert(theDrawParam->Level().LevelValue());
            }
            else if(theDrawParam->Level().LevelType() == kFmiHeight)
            { // laitetaan hybrid datalle parametrin per��n viel "xx", miss� xx on hybrid levelin numero
                str += "_" + NFmiStringTools::Convert(theDrawParam->Level().LevelValue()) + "m";
            }
            else if(theDrawParam->Level().LevelType() == kFmiPressureLevel || theDrawParam->Level().LevelType() == kFmiSoundingLevel)
            { // laitetaan painepinta datalle parametrin per��n viel "xxx", miss� xxx on painepinta levelin arvo tarvittavien etu nollien kera
              // tehd��n se my�s kFmiSpSoundingPlot eli sounding plot paramille
                std::string levStr;
                levStr += "_";
                if(theDrawParam->Level().LevelValue() < 10)
                    levStr += "00";
                else if(theDrawParam->Level().LevelValue() < 100)
                    levStr += "0";

                levStr += NFmiStringTools::Convert(theDrawParam->Level().LevelValue());
                str += levStr;
            }
        }
        
        if(theDrawParam->ShowDifferenceToOriginalData())
            str += "(o)";

        if(fMakeTooltipXmlEncode)
            str = CtrlViewUtils::XmlEncode(str);
        return str;
    }

    NFmiColor GetParamTextColor(NFmiInfoData::Type dataType, bool useArchiveModelData)
    {
        if(useArchiveModelData)
            return NFmiColor(0.016f, 0.64f, 0.68f);

        if(NFmiDrawParam::IsMacroParamCase(dataType))
            return NFmiColor(0.5f, 0.f, 0.5f);

        switch(dataType)
        {
        case NFmiInfoData::kSatelData:
            return NFmiColor(0.f, 0.45f, 0.9f);
        case NFmiInfoData::kViewable:
        case NFmiInfoData::kModelHelpData:
        case NFmiInfoData::kHybridData:
        case NFmiInfoData::kTrajectoryHistoryData:
            return NFmiColor(0.f, 0.5f, 0.f);
        case NFmiInfoData::kObservations:
        case NFmiInfoData::kFlashData:
        case NFmiInfoData::kSingleStationRadarData:
            return NFmiColor(0.f, 0.f, 1.f);
        case NFmiInfoData::kAnalyzeData:
            return NFmiColor(0.94f, 0.139f, 0.126f);
        case NFmiInfoData::kKepaData:
            return NFmiColor(0.78f, 0.082f, 0.52f);
        case NFmiInfoData::kConceptualModelData:
        case NFmiInfoData::kCapData:
            return NFmiColor(1.f, 0.f, 0.5f);
        case NFmiInfoData::kEditingHelpData:
            return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->HelpColor();
        case NFmiInfoData::kClimatologyData:
            return NFmiColor(0.f, 0.5f, 0.f);
        case NFmiInfoData::kWmsData:
            return NFmiColor(0.67f, 0.3f, 0.07f);
        case NFmiInfoData::kMapLayer:
            return NFmiColor(0.8f, 0.22f, 0.f);
        default:
            return NFmiColor(0.f, 0.f, 0.f);
        }
    }

    std::string GetEditingDataString(const std::string &theNameStr, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiLanguage lang, const std::string &theOrigTimeFormat)
    {
        std::string returnStr = theNameStr;
        if(theInfo)
        {
            returnStr += theInfo->OriginTime().ToStr(NFmiString(theOrigTimeFormat), lang);
        }
        returnStr += " ";

        return returnStr;
    }

    std::string GetTotalMapViewStatusBarStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiPoint &theLatlon)
    {
        std::string str(" Location: ");
        double lat = theLatlon.Y();
        str += GetLatitudeMinuteStr(lat, 1);
        str += " ";
        double lon = theLatlon.X();
        lon = FixLongitudeToEuropeCenteredValue(lon);
        str += GetLongitudeMinuteStr(lon, 1);
        str += " [";
        str += GetFixedLatlonStr(theLatlon);
        str += "] Elevation: ";
        float elevation = kFloatMissing;
        boost::shared_ptr<NFmiFastQueryInfo> topoInfo = theCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kStationary);
        if(topoInfo)
        {
            if(topoInfo->Param(kFmiTopoGraf))
                elevation = topoInfo->InterpolatedValue(theLatlon);
        }

        if(elevation == kFloatMissing)
            str += "?";
        else
        {
            str += NFmiStringTools::Convert<int>(static_cast<int>(elevation));
            str += " m";
        }
        return str;
    }

    std::string GetFixedLatlonStr(const NFmiPoint& theLatlon)
    {
        if(theLatlon == NFmiPoint::gMissingLatlon)
            return "----N/S ----E/W";

        std::stringstream out;

        double lat = theLatlon.Y();
        out << std::fixed << std::setprecision(4) << std::setw(7) << std::setfill('0') << std::right << ::fabs(lat) << ((lat >= 0) ? "N" : "S");
        out << " ";

        double lon = theLatlon.X();
        lon = FixLongitudeToEuropeCenteredValue(lon);
        out << std::fixed << std::setprecision(4) << std::setw(8) << std::setfill('0') << std::right << ::fabs(lon) << ((lon >= 0 && lon < 180) ? "E" : "W");
        return out.str();
    }

    // Tein oman tavan kirjoittaa NFmiPoint, koska en halunnut rivinvaihtoa loppuun
    std::string Point2String(const NFmiPoint& p)
    {
        std::stringstream out;
        out << p.X() << "," << p.Y(); 
        return out.str();
    }

    NFmiPoint String2Point(const std::string& str)
    {
        std::stringstream in(str);
        double x = 0;
        double y = 0;
        char ch = 0; // pilkku pit�� lukea v�list�
        in >> x >> ch >> y;
        return NFmiPoint(x, y);
    }

    boost::shared_ptr<NFmiFastQueryInfo> GetLatestLastTimeObservation(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, bool fCrossSectionInfoWanted)
    {
        NFmiInfoData::Type dataType = theDrawParam->DataType();
        if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kAnalyzeData || dataType == NFmiInfoData::kSingleStationRadarData || dataType == NFmiInfoData::kFlashData)
        {
            auto producerId = theDrawParam->Param().GetProducer()->GetIdent();
            if(producerId != kFmiSYNOP && producerId != kFmiFlashObs)
                return theCtrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, fCrossSectionInfoWanted, true);
            else
            {
                boost::shared_ptr<NFmiArea> dummyArea;
                std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infos;
                theCtrlViewDocumentInterface->MakeDrawedInfoVectorForMapView(infos, theDrawParam, dummyArea);
                if(infos.size())
                {
                    size_t latestIndex = 0;
                    NFmiMetTime latestLastTime = infos[0]->TimeDescriptor().LastTime();
                    for(size_t i = 1; i < infos.size(); i++)
                    {
                        if(latestLastTime < infos[i]->TimeDescriptor().LastTime())
                        {
                            latestLastTime = infos[i]->TimeDescriptor().LastTime();
                            latestIndex = i;
                        }
                    }
                    return infos[latestIndex];
                }
            }
        }
        return boost::shared_ptr<NFmiFastQueryInfo>();
    }

    std::string GetLatestObservationTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const std::string &theTimeFormat, bool fCrossSectionInfoWanted)
    {
        std::string str;
        boost::shared_ptr<NFmiFastQueryInfo> info = GetLatestLastTimeObservation(theDrawParam, theCtrlViewDocumentInterface, fCrossSectionInfoWanted);
        if(info)
        {
            str += " (";
            str += ::GetDictionaryString("latest obs.");
            str += ": ";
            str += info->TimeDescriptor().LastTime().ToStr(NFmiString(theTimeFormat));
            str += ")";
        }
        return str;
    }

    std::string GetArchiveOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fGetCurrentDataFromQ2Server, const std::string &theDictionaryTokenForOrigTimeFormat)
    {
        std::string str;
        if(theDrawParam->UseArchiveModelData())
        {
            str += "<b><font color=";
            str += ColorString::Color2HtmlColorStr(NFmiColor(0.016f, 0.64f, 0.68f));
            str += ">";
            str += "\nOrigTime: ";
            NFmiMetTime archiveOrigTime;
            if(fGetCurrentDataFromQ2Server == false && theInfo)
                archiveOrigTime = theInfo->OriginTime();
            else
                archiveOrigTime = theCtrlViewDocumentInterface->GetModelOrigTime(theDrawParam);
            str += archiveOrigTime.ToStr(::GetDictionaryString(theDictionaryTokenForOrigTimeFormat.c_str()), theCtrlViewDocumentInterface->Language());
            str += "</font></b>";
        }
        return str;
    }

    // E.g. tooltip system uses html. So if original text has characters like '<' or '>', they have to
    // be replaced with &lt; and &gt; strings so that html output won't be broken.
    std::string XmlEncode(const std::string &src)
    {
        std::stringstream dst;
        for(char ch : src) 
        {
            switch(ch) {
            case '&': dst << "&amp;"; break;
            case '\'': dst << "&apos;"; break;
            case '"': dst << "&quot;"; break;
            case '<': dst << "&lt;"; break;
            case '>': dst << "&gt;"; break;
            default: dst << ch; break;
            }
        }
        return dst.str();
    }

    FmiDirection CalcFollowingParamWindowViewPosition(FmiDirection currentPosition, bool forward)
    {
        if(forward)
        {
            // kTopLeft -> kTopRight -> kBottomRight -> kBottomLeft -> kNoDirection (piilossa) -> kTopLeft...
            switch(currentPosition)
            {
            case kTopLeft:
                return kTopRight;
                break;
            case kTopRight:
                return kBottomRight;
                break;
            case kBottomRight:
                return kBottomLeft;
                break;
            case kBottomLeft:
                return kNoDirection;
                break;
            default:
                return kTopLeft;
                break;
            }
        }
        else
        {
            // kTopLeft -> kNoDirection (piilossa) -> kBottomLeft -> kBottomRight -> kTopRight -> kTopLeft...
            switch(currentPosition)
            {
            case kNoDirection:
                return kBottomLeft;
                break;
            case kTopLeft:
                return kNoDirection;
                break;
            case kBottomRight:
                return kTopRight;
                break;
            case kBottomLeft:
                return kBottomRight;
                break;
            default:
                return kTopLeft;
                break;
            }
        }
    }

    FmiDirection MoveTimeBoxPositionForward(FmiDirection currentPosition)
    {
        // kBottomLeft -> kTopLeft -> kTopCenter -> kTopRight -> kBottomRight -> kBottomCenter -> kBottomLeft ...
        switch(currentPosition)
        {
        case kBottomLeft:
            return kTopLeft;
        case kTopLeft:
            return kTopCenter;
        case kTopCenter:
            return kTopRight;
        case kTopRight:
            return kBottomRight;
        case kBottomRight:
            return kBottomCenter;
        case kBottomCenter:
            return kBottomLeft;
        default:
            return kBottomLeft;
        }
    }

    FileNameWithTimeList TimeSortFiles(FileNameWithTimeList filesWithTimesCopy, bool descending)
    {
        filesWithTimesCopy.sort(
            [=](const auto& pair1, const auto& pair2)
            {
                if(descending)
                    return pair1.second > pair2.second;
                else
                    return pair1.second < pair2.second;
            }
        );
        return filesWithTimesCopy;
    }

    // Calculate direction from point1 to point2.
    // Result is in degrees (0 -> 360), 'north' is 0 and goes clockwise (east 90, south 180, west 270).
    // If points are the same, result is 0.
    double CalcAngle(const NFmiPoint& point1, const NFmiPoint& point2)
    {
        auto x = point2.X() - point1.X();
        auto y = point2.Y() - point1.Y();
        return CalcAngle(x, y);
    }

    double CalcAngle(double x, double y)
    {
        if(x == 0 && y == 0)
            return 0;
        auto origDirection = atan2(x, y) * 180 / kPii;
        if(origDirection < 0)
            return 360 + origDirection;
        else
            return origDirection;
    }

    bool IsModularParameter(FmiParameterName theParam)
    {
        switch(theParam)
        {
        case kFmiWindDirection:
        case kFmiWaveDirection:
        case kFmiWaveDirectionBandB:
        case kFmiWaveDirectionBandC:
        case kFmiWaveDirectionSwell0:
        case kFmiWaveDirectionSwell1:
        case kFmiWaveDirectionSwell2:
            return true;
        default:
            return false;
        }
    }

    std::string GetMacroParamFormula(NFmiMacroParamSystem& macroParamSystem, const boost::shared_ptr<NFmiDrawParam>& theDrawParam)
    {
        auto macroParamPtr = macroParamSystem.GetWantedMacro(theDrawParam->InitFileName());
        if(macroParamPtr)
            return macroParamPtr->MacroText();
        throw std::runtime_error(std::string(__FUNCTION__) + ": couldn't found macro parameter: " + theDrawParam->ParameterAbbreviation());
    }

    std::string MakeMacroParamRelatedFinalErrorMessage(const std::string& baseMessage, const std::exception* exceptionPtr, boost::shared_ptr<NFmiDrawParam>& theDrawParam, const std::string& macroParamSystemRootPath)
    {
        std::string errorMessage = baseMessage;
        if(exceptionPtr)
        {
            errorMessage += ": \n";
            errorMessage += exceptionPtr->what();
        }
        errorMessage += ", in '";
        errorMessage += PathUtils::getRelativeStrippedFileName(theDrawParam->InitFileName(), macroParamSystemRootPath, "dpa");
        errorMessage += "'";
        return errorMessage;
    }

    void SetMacroParamErrorMessage(const std::string& errorText, boost::shared_ptr<NFmiDrawParam>& triggerDrawParam, CtrlViewDocumentInterface& ctrlViewDocumentInterface, std::string* possibleTooltipErrorTextOut)
    {
        // Lokitetaan virheviesti
        CatLog::logMessage(errorText, CatLog::Severity::Error, CatLog::Category::Macro, true);
        // Jos kyse toolpit laskuista, laitetaan viesti talteen ExtraMacroParamData:an, jotta viesti voidaan laittaa tooltippiin
        if(possibleTooltipErrorTextOut)
            *possibleTooltipErrorTextOut = errorText;

        // talletetaan virheteksti aikaleimalla, ett� k�ytt�j� voi tarkastella sit� sitten smarttool dialogissa
        NFmiTime aTime;
        std::string timeString = aTime.ToStr("YYYY.MM.DD HH:mm:SS\n").CharPtr();
        auto dialogErrorString = timeString + errorText;
        ctrlViewDocumentInterface.SetLatestMacroParamErrorText(dialogErrorString);
        ctrlViewDocumentInterface.SetMacroErrorText(dialogErrorString, triggerDrawParam);
    }

    void ClearMacroParamErrorMessage(boost::shared_ptr<NFmiDrawParam>& triggerDrawParam, CtrlViewDocumentInterface& ctrlViewDocumentInterface)
    {
        ctrlViewDocumentInterface.SetMacroErrorText("", triggerDrawParam);
    }

    std::string wildcardToRegex(const std::string& wildcard) 
    {
        std::string regexStr = wildcard;

        // Escape characters with special meaning in regex
//        regexStr = std::regex_replace(regexStr, std::regex("([\\.\\[\\{\\(\\)\\*\\+\\?\\^\\$\\|])"), "\\$1");

        // Convert wildcard symbols to regex equivalents
        boost::replace_all(regexStr, ".", "\\.");
        boost::replace_all(regexStr, "*", ".*");
        boost::replace_all(regexStr, "?", ".");

        // Match the whole string by adding ^ and $ to the beginning and end
//        regexStr = "^" + regexStr + "$";

        return regexStr;
    }


    namespace fs = std::filesystem;

    void DeleteFilesWithPattern(const std::string& directoryPath, const std::string& fileNamePattern, int keepMaxFiles, std::list<std::string> *deletedFileNamesOut)
    {
        std::vector<fs::path> filePaths;
        auto regexWildCardPatternString = wildcardToRegex(fileNamePattern);
        std::regex regexPattern(regexWildCardPatternString);
        for(const auto& entry : fs::directory_iterator(directoryPath)) 
        {
            if(fs::is_regular_file(entry) && std::regex_match(entry.path().filename().string(), regexPattern))
            {
                filePaths.push_back(entry.path());
            }
        }

        // Sort file paths by creation time in descending order
        std::sort(filePaths.begin(), filePaths.end(),
            [](const fs::path& path1, const fs::path& path2) 
            {
                return fs::last_write_time(path1) > fs::last_write_time(path2);
            });

        // Keep the newest files as much keepMaxFiles number indicates, and remove the rest of older files
        int fileCounter = 0;
        for(const auto& filePath : filePaths)
        {
            if(fileCounter >= keepMaxFiles)
            {
                fs::remove(filePath);
                if(deletedFileNamesOut)
                {
                    deletedFileNamesOut->push_back(filePath.filename().string());
                }
            }
            fileCounter++;
        }
    }

    void DeleteFilesWithPattern(const std::string& filePathPattern, int keepMaxFiles, std::list<std::string>* deletedFileNamesOut)
    {
        fs::path fullPath(filePathPattern);
        std::string directoryPath = fullPath.parent_path().string();
        std::string fileNamePattern = fullPath.filename().string();
        DeleteFilesWithPattern(directoryPath, fileNamePattern, keepMaxFiles, deletedFileNamesOut);
    }

    void DeleteFilesWithPatternAndLog(const std::string& filePathPattern, const std::string& logMessageStart, CatLog::Severity severity, CatLog::Category category, int keepMaxFiles)
    {
        std::list<std::string> deletedFiles;
        DeleteFilesWithPattern(filePathPattern, keepMaxFiles, &deletedFiles);
        if(!deletedFiles.empty())
        {
            auto deletedFilesListString = MakeCommaSeparatedStringFromStrings(deletedFiles);
            std::string logMessage = logMessageStart;
            logMessage += deletedFilesListString;
            CatLog::logMessage(logMessage, severity, category);
        }
    }

    // Huom! std::filesystem::path::extension metodi palauttaa my�s pisteen eli jos
    // filePath:in arvo oli "c:\polku\filename.txt" palautetaan arvo ".txt"
    std::string GetFileExtension(const std::string& filePath)
    {
        fs::path path(filePath);
        return path.extension().string();
    }

    std::string GetParentPath(const std::string& filePath)
    {
        fs::path path(filePath);
        return path.parent_path().string();
    }

    void CleanDirectory(const std::string& theDirectory, double theKeepHours, std::list<std::string>* deletedFilesOut)
    {
        try 
        {
            fs::path dirPath = theDirectory;
            long long ageLimitInSeconds = boost::math::iround(theKeepHours * 3600.);
            // Get current time
            auto now = fs::file_time_type::clock::now();

            // Iterate over directory
            for(const auto& entry : fs::directory_iterator(dirPath)) 
            {
                // Check if it's a regular file
                if(entry.is_regular_file()) 
                {
                    try
                    {
                        // Get the last write time of the file
                        auto fileTime = fs::last_write_time(entry);

                        auto ageInSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - fileTime).count();

                        // If the file is older than the age limit, delete it
                        if(ageInSeconds > ageLimitInSeconds)
                        {
                            fs::remove(entry);
                            if(deletedFilesOut)
                            {
                                deletedFilesOut->push_back(entry.path().string());
                            }
//                            CatLog::logMessage(std::string("CleanDirectory delete file: ") + entry.path().string(), CatLog::Severity::Debug, CatLog::Category::Operational);
                        }
                    }
                    catch(...)
                    {
                        // Ei tehd� mit��n, estet��n vain loopin lopetus.
                        // Esim. kun yritet��n deletoida vanhaa, mutta Smartmetin k�yt�ss� (= ei voi deletoida) olevaa tiedostoa, lent�� poikkeus ja se on ihan ok
                    }
                }
            }
        }
        catch(...) 
        {
            // Ei tehd� mit��n, estet��n vain funktiosta ulostulo poikkeuksella.
            // Esim. kun yritet��n puhdistaa hakemistoa, jota ei ole, t�ll�in lent�� poikkeus ja se on ihan ok
        }
    }

} // namespace CtrlViewUtils
