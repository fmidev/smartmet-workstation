
#pragma once

/*********************************************************************

Parameters:
  n          - the number of control points minus 1
  t          - the degree of the polynomial plus 1
  control    - control point array made up of point stucture
  output     - array in which the calculate spline points are to be put
  num_output - how many points on the spline are to be calculated

Pre-conditions:
  n+2>t  (no curve results if n+2<=t)
  control array contains the number of points specified by n
  output array is the proper size to hold num_output point structures.


**********************************************************************/

struct point {
  double x;
  double y;
  double z;
};

void bspline(int n, int t, point *control, point *output, int num_output);
