#include "stdafx.h"

#ifndef DISABLE_UNIRAS_TOOLMASTER

#include "ToolMasterHelperFunctions.h"
#include "NFmiGriddingHelperInterface.h"
#include "NFmiDrawParam.h"
#include "ToolMasterColorCube.h"
#include "ToolMasterDrawingFunctions.h"
#include "NFmiIsoLineData.h"
#include "NFmiGriddingProperties.h"
#include "catlog/catlog.h"

#include "agX/agx.h"

namespace
{
    int m_defaultContext; // ToolMasterin initialisointiin liittyv� muuttuja
    int m_toolmasterContext; // T�m� on tosiasiallinen toolmaster context (en oikein tajua t�t� context juttua)

    // k�ytet��n avuksi least-squeare-metodin yhteydess�
    struct MultiplyFunctor
    {
        MultiplyFunctor(float theFactor) :itsFactor(theFactor) {}
        float operator()(float theValue)
        {
            return theValue*itsFactor;
        }
        float itsFactor;
    };
}

namespace Toolmaster
{
    void SetToolMastersDC(CDC* theDC, const CRect &theClientRect)
    {
        XuWindowSize(theClientRect.Width(), theClientRect.Height());
        XuWindowSelect(theDC->GetSafeHdc());
    }

    void DoToolMasterGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, const NFmiRect &theRelativeRect, const NFmiGriddingProperties &griddingProperties, float theObservationRadiusRelative, const NFmiDataMatrix<float> &gridData, std::vector<float> &toolMasterGridValuesOut)
    {
        // Toolmaster griddaus funktioiden k�ytt�
        XuViewWorldLimits(theRelativeRect.Left(), theRelativeRect.Right(), theRelativeRect.Top(), theRelativeRect.Bottom(), 0, 0);
        XuUndefined(kFloatMissing, 2);
        XuGriddingSmoothLevel(griddingProperties.smoothLevel());
        XuGriddingLocalFitMethod(griddingProperties.localFitMethod());
        XuGriddingLocalFitDelta(griddingProperties.localFitDelta());
        XuGriddingLocalFitFilter(griddingProperties.localFitFilterRadius(), griddingProperties.localFitFilterFactor());

        if(theObservationRadiusRelative != kFloatMissing)
            XuGriddingLocalFitRadius(theObservationRadiusRelative);
        else
            XuGriddingLocalFitRadius(1.41); // T�m� on laskentaruudun (~ 0,0 - 1,1) kulmapisteiden diagonaalinen et�isyys, mik� on t�m�n radiuksen oletusarvo (= kaikkia pisteit� k�ytet��n aina laskuissa)

        switch(griddingProperties.function())
        {
        case kFmiXuGriddingThinPlateSplineCalc:
            XuGriddingThinPlateSplineCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()));
            break;

        case kFmiXuGriddingFastLocalFitCalc:
            XuGriddingFastLocalFitCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()));
            break;
        case kFmiXuGriddingLocalFitCalc:
            XuGriddingLocalFitCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()));
            break;
        case kFmiXuGriddingTriangulationCalc: // t�m� ei oikein toimi (joskus teki rumaa mutta j�rkev�� tulosta, nyt rikki)
        {
            // triangulaatio algoritmi vaatii paria ty� taulukkoa
            std::vector<int> int_array(31 * arraySize + (gridData.NY() * gridData.NX()));
            std::vector<float> float_array(5 * arraySize);
            XuGriddingTriangulationCalc(&xValues[0], &yValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()), &int_array[0], &float_array[0]);
            break;
        }
        case kFmiXuGriddingLeastSquaresCalc: // en saa tekem��n j�rkev�� tulosta
        {
            const double factor = 100.;
            MultiplyFunctor mul(factor);
            XuViewWorldLimits(0, factor, 0, factor, 0, 0);
            int side_length = static_cast<int>(factor / 2); // t�m� menee luultavasti pieleen, pit�� olla int, mutta asetin maailman 0,0 - 1,1:n kokoiseksi
            int num_y_subgrid = 2;//itsGridYSize/10; // subgridin pit�isi kai olla jaollinen originaali gridin koosta
            int num_x_subgrid = 2;//itsGridXSize/10;
            std::vector<float> tmpXValues(xValues.begin(), xValues.end());
            std::transform(tmpXValues.begin(), tmpXValues.end(), tmpXValues.begin(), mul);
            std::vector<float> tmpYValues(yValues.begin(), yValues.end());
            std::transform(tmpYValues.begin(), tmpYValues.end(), tmpYValues.begin(), mul);
            XuGriddingLeastSquaresCalc(&tmpXValues[0], &tmpYValues[0], &zValues[0], arraySize, &toolMasterGridValuesOut[0], static_cast<int>(gridData.NY()), static_cast<int>(gridData.NX()), side_length, num_y_subgrid, num_x_subgrid);
            break;
        }
        default:
            break;
        }
    }

    std::string MakeAvsToolmasterVersionString()
    {
        std::string avsToolmasterVersion = "";
        avsToolmasterVersion += NFmiStringTools::Convert(XuVERSION);
        avsToolmasterVersion += ".";
        avsToolmasterVersion += NFmiStringTools::Convert(XuREVISION);
        avsToolmasterVersion += ".";
        if(XuUPDATE_LEVEL == ' ')
            avsToolmasterVersion += NFmiStringTools::Convert(2); // tiet��kseni t�ll� hetkell�
        else
            avsToolmasterVersion += NFmiStringTools::Convert(XuUPDATE_LEVEL);

        return avsToolmasterVersion;
    }

    void CloseToolMaster()
    {
        // You must return to the initial context before deleting this one
        XuContextSelect(m_defaultContext);
        XuContextDelete(m_toolmasterContext);
    }

    int InitToolMaster(CWnd *mainWindow, bool useToolMasterIfAvailable)
    {
        bool toolMasterInUse = false;
        int tmStatus = -1;
        if(useToolMasterIfAvailable)
        {
            XuLicenseQuery(&tmStatus);
            if(tmStatus == XuLICOK || tmStatus == XuTRIAL)
            {
                toolMasterInUse = true;
                //Initialize Toolmaster</A>
                XuInitialize(NULL, NULL);
                XuMessageTypes(XuON,                    // short messages in messagebox
                    XuOFF);                  // long messages in text window
                XuMessageLevels(XuMESSAGE_CONTINUE, 	 // informational
                    XuMESSAGE_CONTINUE,      // warnings
                    XuMESSAGE_CONTINUE,      // errors
                    XuMESSAGE_KEEP_STATE);   // fatals
                XuOpen((HWND)mainWindow);        // pass any window handle for start up
                XuContextQuery(&m_defaultContext);     // needed as "emergency" context
            }
        }

        InitToolMasterColors(toolMasterInUse);

        return tmStatus;
    }

    void InitToolMasterColors(bool fToolMasterAvailable)
    {
        if(fToolMasterAvailable)
        {
            // Create a Toolmaster Graphics Context
            COLORREF f; float rgb[3], dummy[5];

            // Create a new Toolmaster graphics context and select it
            XuContextCreate(&(m_toolmasterContext));
            XuContextSelect(m_toolmasterContext);

            // Change color table 1 to RGB 0-255
            XuColorTableChange(1, 255, XuLOOKUP, XuRGB, 255.);

            // Copy Windows' text color to Toolmaster's foreground
            f = GetSysColor(COLOR_WINDOWTEXT);
            rgb[0] = GetRValue(f);
            rgb[1] = GetGValue(f);
            rgb[2] = GetBValue(f);
            XuColor(XuCOLOR, 1, rgb, dummy);
            XuColorDeviceLoad(1);

            // Copy Windows' window color to Toolmasters background
            f = GetSysColor(COLOR_WINDOW);
            rgb[0] = GetRValue(f);
            rgb[1] = GetGValue(f);
            rgb[2] = GetBValue(f);
            XuColor(XuCOLOR, 0, rgb, dummy);
            XuColorDeviceLoad(0);
        }

        ToolMasterColorCube::InitDefaultColorTable(fToolMasterAvailable); // Varmistetaan color-cuben alustus
    }

    bool DoToolMasterInitialization(CWnd *mainWindow, bool useToolMasterIfAvailable)
    {
        bool useToolMaster = false;
        int tmStatus = InitToolMaster(mainWindow, useToolMasterIfAvailable);
        if(tmStatus == XuLICOK || tmStatus == XuTRIAL)
        {
            useToolMaster = true;
        }

        std::string toolmasterLicenseStr("agx ToolMaster license query returned: ");
        CatLog::Severity errorLevel = CatLog::Severity::Error;
        if(tmStatus == XuLICOK)
        {
            errorLevel = CatLog::Severity::Info;
            toolmasterLicenseStr += "XuLICOK, graphics-license is in use.";
        }
        else if(tmStatus == XuTRIAL)
        {
            errorLevel = CatLog::Severity::Info;
            toolmasterLicenseStr += "XuTRIAL, graphics-license is in use.";
        }
        else if(tmStatus == XuMAXUSER)
            toolmasterLicenseStr += "XuMAXUSER, graphics-license is not in use.";
        else if(tmStatus == XuEXPIRED)
            toolmasterLicenseStr += "XuEXPIRED, graphics-license is not in use.";
        else if(tmStatus == XuLICERROR)
            toolmasterLicenseStr += "XuLICERROR, graphics-license is not in use.";
        else if(tmStatus == -1)
        {
            errorLevel = CatLog::Severity::Warning;
            toolmasterLicenseStr = "agx ToolMaster graphics-license will not be used.";
        }
        else
            toolmasterLicenseStr += "Unknown error, graphics-license is not in use.";

        CatLog::logMessage(toolmasterLicenseStr, errorLevel, CatLog::Category::Configuration);

        return useToolMaster;
    }
}

#endif // DISABLE_UNIRAS_TOOLMASTER
