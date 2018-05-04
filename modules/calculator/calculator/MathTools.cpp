// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace MathTools
 */
// ======================================================================
/*!
 * \namespace MathTools
 *
 * \brief Various mathematical utitity functions
 *
 */
// ======================================================================

#include "MathTools.h"

#include <newbase/NFmiGlobals.h>
#include <cmath>

namespace MathTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Round the input value to the given integer precision
 *
 * \param theValue The floating value
 * \param thePrecision The precision >= 0
 * \return The rounded value
 */
// ----------------------------------------------------------------------

int to_precision(float theValue, int thePrecision)
{
  if (thePrecision <= 0) return static_cast<int>(round(theValue));
  const int value = static_cast<int>(round(theValue / thePrecision) * thePrecision);
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculates Pearson coefficient
 *
 * \param theValueVector The vector containg double values
 * \return The Pearson coefficient
 */
// ----------------------------------------------------------------------
double pearson_coefficient(const std::vector<double>& theValueVector)
{
  double coefficient = 0.0;

  double sumX = 0.0;
  double sumY = 0.0;
  double sumXY = 0.0;
  double xpow2 = 0.0;
  double ypow2 = 0.0;

  unsigned int vectorSize = theValueVector.size();

  for (unsigned int i = 0; i < vectorSize; i++)
  {
    sumX += (i + 1);
    sumY += theValueVector.at(i);
    sumXY += ((i + 1) * theValueVector.at(i));
    xpow2 += std::pow(static_cast<double>(i + 1), 2.0);
    ypow2 += std::pow(theValueVector.at(i), 2.0);
  }

  double numerator = (vectorSize * sumXY - (sumX * sumY));
  double denominator = std::sqrt(((vectorSize * xpow2) - std::pow(sumX, 2)) *
                                 ((vectorSize * ypow2) - std::pow(sumY, 2)));
  coefficient = (denominator == 0.0 ? 0.0 : numerator / denominator);

  return coefficient;
}

}  // namespace MathTools

// ======================================================================
