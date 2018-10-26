#pragma once

#include <vector>

class NFmiRect;
class NFmiGriddingProperties;

namespace toolmaster_tools
{

bool init(bool use_toolmaster, int &toolmasterContext);
void close(void);
bool do_gridding(size_t size_x, size_t size_y, const NFmiRect &relative_rect, std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values, const NFmiGriddingProperties &griddingProperties, float theObservationRadiusRelative, std::vector<float> &gridding_values_out);

}
