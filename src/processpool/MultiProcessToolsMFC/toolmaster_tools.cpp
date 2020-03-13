
#include "stdafx.h"
#include "toolmaster_tools.h"
#include "logging.h"
#include "NFmiObsDataGridding.h"
#include "NFmiGriddingProperties.h"

#include "NFmiRect.h"

#include "boost/lexical_cast.hpp"

#include <agx\agx.h>

// k‰ytet‰‰n avuksi least-squeare-metodin yhteydess‰
struct MultiplyFunctor
{
	MultiplyFunctor(float theFactor):itsFactor(theFactor){}
	float operator()(float theValue)
	{
		return theValue*itsFactor;
	}
	float itsFactor;
};

namespace toolmaster_tools
{

bool init(bool use_toolmaster, int &toolmasterContext)
{
	int tmStatus = -1;
    if(use_toolmaster)
	{
		XuLicenseQuery(&tmStatus);
		if(tmStatus == XuLICOK || tmStatus == XuTRIAL)
		{
			//<STRONG><A NAME="initial">Initialize Toolmaster</A>
			XuInitialize   (NULL, NULL);
			XuMessageTypes (XuON,                    // short messages in messagebox
							XuOFF);                  // long messages in text window
			XuMessageLevels(XuMESSAGE_CONTINUE, 	 // informational
							XuMESSAGE_CONTINUE,      // warnings
							XuMESSAGE_CONTINUE,      // errors
							XuMESSAGE_KEEP_STATE);   // fatals
            XuOpen        ((HWND)AfxGetMainWnd());        // pass any window handle for start up
			XuContextQuery (&toolmasterContext) ;     // needed as "emergency" context
			//</STRONG>
		}
	}

	std::string license_str("agx ToolMaster license query returned: ");
    logging::trivial::severity_level log_level = logging::trivial::error;
	if(tmStatus == XuLICOK)
    {
        log_level = logging::trivial::info;
		license_str += "XuLICOK, graphics-license is in use.";
    }
	else if(tmStatus == XuTRIAL)
		license_str += "XuTRIAL, graphics-license is in use.";
	else if(tmStatus == XuMAXUSER)
		license_str += "XuMAXUSER, graphics-license is not in use.";
	else if(tmStatus == XuEXPIRED)
		license_str += "XuEXPIRED, graphics-license is not in use.";
	else if(tmStatus == XuLICERROR)
		license_str += "XuLICERROR, graphics-license is not in use.";
	else if(tmStatus == -1)
    {
        log_level = logging::trivial::info;
		license_str = "agx ToolMaster graphics-license will not be used.";
    }
	else
        license_str += "Unknown error, graphics-license is not in use.";
    log_message(license_str, log_level);

    return tmStatus == XuLICOK;
}

void close(void)
{
    XuClose();
}

bool do_gridding(size_t size_x, size_t size_y, const NFmiRect &relative_rect, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, const NFmiGriddingProperties &griddingProperties, float theObservationRadiusRelative, std::vector<float> &gridding_values_out)
{
    gridding_values_out.resize(size_x * size_y, kFloatMissing);
    int arraySize = static_cast<int>(z_values.size());
    // T‰m‰ griddaus lasku osio on kopsattu ja muutettu NFmiDataParamControlPointModifier::DoDataGridding(std::vector<float> &xValues, .... ) -funktiosta,
    // smartmetbizcode\ModifyEditedData -kirjastosta.

    if(griddingProperties.function() == kFmiMarkoGriddingFunction)
    {
        NFmiObsDataGridding obsDataGridding;
		obsDataGridding.SearchRange(0.7f);
		obsDataGridding.AreaLimits(relative_rect);
        NFmiDataMatrix<float> gridData(size_x, size_y, 0);
		obsDataGridding.DoGridding(x_values, y_values, z_values, arraySize, gridData);

		for(unsigned int j=0; j<gridData.NY(); j++)
			for(unsigned int i=0; i<gridData.NX(); i++)
				gridding_values_out[j*gridData.NX() + i] = gridData[i][j]; // laitetaan lasketun hilan arvot paluu vektoriin
    }
    else
    {
        XuViewWorldLimits(relative_rect.Left(), relative_rect.Right(), relative_rect.Top(), relative_rect.Bottom(), 0, 0);
        XuUndefined(kFloatMissing, 2);
        XuGriddingSmoothLevel(griddingProperties.smoothLevel());
        XuGriddingLocalFitMethod(griddingProperties.localFitMethod());
        XuGriddingLocalFitDelta(griddingProperties.localFitDelta());
        XuGriddingLocalFitFilter(griddingProperties.localFitFilterRadius(), griddingProperties.localFitFilterFactor());
        if(theObservationRadiusRelative != kFloatMissing)
            XuGriddingLocalFitRadius(theObservationRadiusRelative);
        else
            XuGriddingLocalFitRadius(1.41); // T‰m‰ on laskentaruudun (~ 0,0 - 1,1) kulmapisteiden diagonaalinen et‰isyys, mik‰ on t‰m‰n radiuksen oletusarvo (= kaikkia pisteit‰ k‰ytet‰‰n aina laskuissa)

		switch(griddingProperties.function())
		{
		case kFmiXuGriddingThinPlateSplineCalc:
			XuGriddingThinPlateSplineCalc(x_values.data(), y_values.data(), z_values.data(), arraySize, gridding_values_out.data(), static_cast<int>(size_y), static_cast<int>(size_x));
			break;
		case kFmiXuGriddingFastLocalFitCalc:
			XuGriddingFastLocalFitCalc(x_values.data(), y_values.data(), z_values.data(), arraySize, gridding_values_out.data(), static_cast<int>(size_y), static_cast<int>(size_x));
			break;
		case kFmiXuGriddingLocalFitCalc:
            XuGriddingLocalFitCalc(x_values.data(), y_values.data(), z_values.data(), arraySize, gridding_values_out.data(), static_cast<int>(size_y), static_cast<int>(size_x));
			break;
		case kFmiXuGriddingTriangulationCalc: // t‰m‰ ei oikein toimi (joskus teki rumaa mutta j‰rkev‰‰ tulosta, nyt rikki)
			{
				// triangulaatio algoritmi vaatii paria tyˆ taulukkoa
				std::vector<int> int_array(31 * arraySize + (size_y * size_x) );
				std::vector<float> float_array(5 * arraySize);
				XuGriddingTriangulationCalc(x_values.data(), y_values.data(), z_values.data(), arraySize, gridding_values_out.data(), static_cast<int>(size_y), static_cast<int>(size_x), &int_array[0], &float_array[0]);
				break;
			}
		case kFmiXuGriddingLeastSquaresCalc: // en saa tekem‰‰n j‰rkev‰‰ tulosta
			{
				const double factor = 100.;
				MultiplyFunctor mul(factor);
				XuViewWorldLimits(0, factor, 0, factor, 0, 0);
				int side_length = static_cast<int>(factor/2); // t‰m‰ menee luultavasti pieleen, pit‰‰ olla int, mutta asetin maailman 0,0 - 1,1:n kokoiseksi
				int num_y_subgrid = 2;//itsGridYSize/10; // subgridin pit‰isi kai olla jaollinen originaali gridin koosta
				int num_x_subgrid = 2;//itsGridXSize/10;
				std::vector<float> tmpXValues(x_values.begin(), x_values.end());
				std::transform(tmpXValues.begin(), tmpXValues.end(), tmpXValues.begin(), mul);
				std::vector<float> tmpYValues(y_values.begin(), y_values.end());
				std::transform(tmpYValues.begin(), tmpYValues.end(), tmpYValues.begin(), mul);
                XuGriddingLeastSquaresCalc(tmpXValues.data(), tmpYValues.data(), z_values.data(), arraySize, gridding_values_out.data(), static_cast<int>(size_y), static_cast<int>(size_x), side_length, num_y_subgrid , num_x_subgrid);
				break;
			}
		default:
            std::string error_string("Unknown gridding function (");
            error_string += boost::lexical_cast<std::string>(griddingProperties.function());
            error_string += ") used in do_gridding_work_with_toolmaster -function, putting empty result";
            log_message(error_string, logging::trivial::error);
            gridding_values_out.clear(); // tyhjennet‰‰n arvo vektori virhetilanteissa, koska sit‰ saatetaan k‰ytt‰‰ tyhj‰n‰ ulkona!
			return false;
		}
    }
    return true;
}

} // namespace toolmaster_tools loppuu
