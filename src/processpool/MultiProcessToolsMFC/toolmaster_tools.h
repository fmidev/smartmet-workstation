#pragma once

#include <vector>

class NFmiRect;

// T‰m‰ enum on kopsattu smartmetbizcode\ModifyEditedData -kirjastosta include\NFmiEditorControlPointManager.h -tiedostosta.
// En halunnut j‰rjest‰‰ enumia esim. newbaseen, enk‰ halunnut riippuvuutta smartmetbizcode\ModifyEditedData -kirjastoon.
enum FmiGriddingFunction
{
	kFmiMarkoGriddingFunction = 0,
	kFmiXuGriddingFastLocalFitCalc = 1,
	kFmiXuGriddingLocalFitCalc = 2,
	kFmiXuGriddingTriangulationCalc = 3,
	kFmiXuGriddingLeastSquaresCalc = 4,
	kFmiXuGriddingThinPlateSplineCalc = 5,
	kFmiErrorGriddingFunction // virhetilanteita varten
};

namespace toolmaster_tools
{

bool init(bool use_toolmaster, int &toolmasterContext);
void close(void);
bool do_gridding(size_t size_x, size_t size_y, const NFmiRect &relative_rect, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, FmiGriddingFunction griddingFunction, std::vector<float> &gridding_values_out);

}
