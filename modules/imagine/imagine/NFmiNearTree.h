// ======================================================================
//
// Nearest neighbour search tree from C++ Users Journal.
//
// Nearest Neighbor algorithm after Kalantari and McDonald,
// (IEEE Transactions on Software Engineering, v. SE-9, pp.631-634,1983)
// modified to use recursion instead of a double-linked tree
// and simplified so that it does a bit less checking for
// things like is the distance to the right less than the
// distance to the left; it was found that these checks little
// to no difference.
//
// copyright by Larry Andrews, 2001
// may be freely distributed or used as long as this copyright notice
// is included
//
// This template is used to contain a collection of objects. After the
// collection has been loaded into this structure, it can be quickly
// queried for which object is "closest" to some probe object of the
// same type. The major restriction on applicability of the near-tree
// is that the algorithm only works if the objects obey the triangle
// inequality. The triangle rule states that the length of any side of
// a triangle cannot exceed the sum of the lengths of the other two sides.
//
// The user of this class needs to provide at least the following
// functionality for the template to work. For the built-in
// numerics of C++, they are provided by the system.
//
//    a functor to calculate the distance
//    a copy constructor
//    a constructor would be nice
//    a destructor would be nice
//
// The provided interface is:
//
//    #include "NFmiNearTree.h"
//
//    NFmiNearTree( void )   // constructor
//       instantiated by something like:      NFmiNearTree<v,f> tree;
//       for some type v
//
//    void Insert( T & t )
//       where t is an object of the type v
//
//    bool NearestNeighbor ( const double & theRadius,
//                           T & theClosest,
//                           const T & thePoint ) const
//
//       theRadius is the largest radius within which to search; make it
//       very large if you want to include every point that was loaded;
//       theRadius is returned as the closest distance to the probe (or
//       the search radius if nothing is found)
//
//       theClosest is returned as the object that was found closest to
//       the probe point (if any were within radius theRadius of the probe)
//       thePoint is the probe point, used to search in the group of
//       points Insert'ed
//
//       return value is true if some object was found within the search
//       radius, false otherwise
//
//    bool FarthestNeighbor ( T & theFarthest, const T & thePoint ) const
//
//       theFarthest is returned as the object that was found farthest
//       to the probe point
//       thePoint is the probe point, used to search in the group of
//       points Insert'ed
//
//       return value is true if some object was found, false otherwise
//
//   long FindInSphere ( const double & theRadius,
//                       std::vector<T> & theClosest,
//                       const T & thePoint) const
//
//       theRadius is the radius within which to search; make it very
//       large if you want to include every point that was loaded;
//       theClosest is returned as the vector of objects that were
//       found within a radius theRadius of the probe point
//
//       thePoint is the probe point, used to search in the group of
//       points Insert'ed
//
//       return value is the number of objects found within the search radius
//
//    ~NFmiTree( void )  // destructor
//       invoked by  vTree.CNeartree<v>::~CNearTree
//       for an object vTree of some type v
//
// So a complete program is:
//
// #include "NFmiNearTree.h"
// #include <cstdio>
// void main()
// {
//   NFmiNearTree< double,perkele > dT;
//   double dist;
//   dT.Insert( 1.5 );
//   if ( dT.NearestNeighbor( 10000.0, dist, 2.0 )) printf( "%f\n",dRad );
// }
//
// and it should print 0.5 (that's how for 2.0 is from 1.5)
//
// ======================================================================

#pragma once

#include <vector>

namespace Imagine
{
template <typename T, class Distance>
class NFmiNearTree
{
  // Insert copies the input objects into a binary NEAR tree. When a
  // node has two entries, a descending node is used or created. The current
  // datum is put into the branch descending from the nearer of the two
  // objects in the current node.

  // NearestNeighbour retrieves the object nearest to some probe by
  // descending the tree to search out the appropriate object. Speed is
  // gained by pruning the tree if there can be no data below that are
  // nearer than the best so far found.

  // The tree is built in time O(n log n), and retrievals take place in
  // time O(log n).

 private:
  T* itsLeftObject;              // first object stored in this node
  T* itsRightObject;             // second object stored in this node
  double itsMaxLeft;             // max distance from itsLeftObject to itsLeftBranch
  double itsMaxRight;            // max distance from itsRightObject to itsRightBranch
  NFmiNearTree* itsLeftBranch;   // nodes closer to itsLeftObject
  NFmiNearTree* itsRightBranch;  // nodes closer to itsRightObject

 public:
  //=======================================================================
  // NFmiNearTree ( )
  //
  // Default constructor for class NFmiNearTree
  // creates an empty tree with no right or left node and with the
  // dMax-below set to negative values so that any match found will be
  // stored since it will greater than the negative value
  //
  //=======================================================================

  NFmiNearTree(void)
  {
    itsLeftObject = 0;
    itsRightObject = 0;
    itsLeftBranch = 0;
    itsRightBranch = 0;
    itsMaxLeft = -1.0;
    itsMaxRight = -1.0;
  }

  //=======================================================================
  //  ~NFmiNearTree ( )
  //
  //  Destructor for class NFmiNearTree
  //
  //=======================================================================

  ~NFmiNearTree(void)
  {
    delete itsLeftBranch;
    itsLeftBranch = 0;
    delete itsRightBranch;
    itsRightBranch = 0;
    delete itsLeftObject;
    itsLeftObject = 0;
    delete itsRightObject;
    itsRightObject = 0;
  }

  //=======================================================================
  //  void Insert ( const T & theObject )
  //
  //  Function to insert some "point" as an object into a NFmiNearTree for
  //  later searching
  //
  //     t is an object of the templated type which is to be inserted into a
  //     Neartree
  //
  //  Three possibilities exist: put the datum into the left
  //  postion (first test),into the right position, or else
  //  into a node descending from the nearer of those positions
  //  when they are both already used.
  //
  //=======================================================================

  void Insert(const T& theObject)
  {
    double dist_right = 0;
    double dist_left = 0;

    if (itsRightObject != 0)
    {
      dist_right = Distance(theObject, *itsRightObject);
      dist_left = Distance(theObject, *itsLeftObject);
    }

    if (itsLeftObject == 0)
      itsLeftObject = new T(theObject);

    else if (itsRightObject == 0)
      itsRightObject = new T(theObject);

    else if (dist_left > dist_right)
    {
      if (itsRightBranch == 0) itsRightBranch = new NFmiNearTree;

      // note that the next line assumes that itsMaxRight is
      // negative for a new node

      if (itsMaxRight < dist_right) itsMaxRight = dist_right;

      itsRightBranch->Insert(theObject);
    }

    else
    {
      if (itsLeftBranch == 0) itsLeftBranch = new NFmiNearTree;

      // note that the next line assumes that itsMaxLeft is
      // negative for a new node

      if (itsMaxLeft < dist_left) itsMaxLeft = dist_left;

      itsLeftBranch->Insert(theObject);
    }
  }

  //=======================================================================
  // bool NearestNeighbour ( const double& theRadius,
  //                         T & theClosest,
  //                         const T& thePoint ) const
  //
  // Function to search a NFmiNearTree for the object closest to some probe
  // point, thePoint. This function is only here so that the function
  // Nearest can be called without having theRadius const
  //
  // theRadius is the maximum search radius - any point farther than
  // theRadius from the probe point will be ignored
  // theClosest is an object of the templated type and is the returned
  //            nearest point to the probe point that can be found in
  //            the NFmiNearTree
  // thePoint  is the probe point
  //
  // the return value is true only if a point was found
  //
  //=======================================================================

  bool NearestNeighbour(const double& theRadius, T& theClosest, const T& thePoint) const
  {
    double searchradius = theRadius;
    return (Nearest(searchradius, theClosest, thePoint));
  }

  //=======================================================================
  // bool FarthestNeighbor ( const double& theRadius,
  //                          T & theFarthest,
  //                          const T & thePoint ) const
  //
  // Function to search a NFmiNearTree for the object closest to some
  // probe point, thePoint. This function is only here so that the
  // function FarthestNeighbor can be called without the user having
  // to input a search radius and so the search radius can be guaranteed
  // to be negative at the start.
  //
  // theFarthest is an object of the templated type and is the returned
  //             farthest point from the probe point that can be found
  //             in the NFmiNearTree
  // thePoint is the probe point
  //
  // the return value is true only if a point was found (should only
  // be false for an empty tree)
  //
  //=======================================================================

  bool FarthestNeighbor(T& theFarthest, const T& thePoint) const
  {
    double searchradius = -1.0;
    return (FindFarthest(searchradius, theFarthest, thePoint));
  }

  //=======================================================================
  // long FindInSphere ( const double& theRadius,
  //                      std::vector<T>& theClosest,
  //                      const T & thePoint ) const
  //
  // Function to search a NFmiNearTree for the set of objects closer
  // to some probe point, thePoint, than theRadius. This is only here so
  // that theClosest can be cleared before starting the work.
  //
  // theRadius is the maximum search radius - any point farther than
  //           theRadius from the probe point will be ignored
  // theClosest is a vector of objects of the templated type and is the
  //            returned set of nearest points to the probe point that
  //            can be found in the NFmiNearTree
  // thePoint is the probe point
  //
  // return value is the number of points found within theRadius of the
  // probe point
  //
  //=======================================================================

  long FindInSphere(const double& theRadius, std::vector<T>& theClosest, const T& thePoint) const
  {
    // clear the contents of the return vector so that things
    // don't accidentally accumulate

    theClosest.clear();
    return (InSphere(theRadius, theClosest, thePoint));
  }

 private:
  //=======================================================================
  // long InSphere ( const double& theRadius,
  //                 std::vector<T> & theClosest,
  //                 const T & thePoint ) const
  //
  // Private function to search a NFmiNearTree for the object closest
  // to some probe point, thePoint
  // This function is only called by FindInSphere.
  //
  // theRadius is the search radius
  // theClosest is a vector of objects of the templated type found
  //            within theRadius of the probe point
  // thePoint is the probe point
  //
  // the return value is the number of points found within theRadius
  // of the probe
  //
  //=======================================================================

  long InSphere(const double& theRadius, std::vector<T>& theClosest, const T& thePoint) const
  {
    long npoints = 0;

    // first test each of the left and right positions to see if
    // one holds a point nearer than the search radius.

    if ((itsLeftObject != 0) && (Distance(thePoint, *itsLeftObject) <= theRadius))
    {
      theClosest.push_back(*itsLeftObject);
      npoints++;
    }
    if ((itsRightObject != 0) && (Distance(thePoint, *itsRightObject) <= theRadius))
    {
      theClosest.push_back(*itsRightObject);
      npoints++;
    }

    // Now we test to see if the branches below might hold an object
    // nearer than the search radius. The triangle rule is used
    // to test whether it's even necessary to descend.

    if ((itsLeftBranch != 0) && ((theRadius + itsMaxLeft) >= Distance(thePoint, *itsLeftObject)))
      npoints += itsLeftBranch->InSphere(theRadius, theClosest, thePoint);

    if ((itsRightBranch != 0) && ((theRadius + itsMaxRight) >= Distance(thePoint, *itsRightObject)))
      npoints += itsRightBranch->InSphere(theRadius, theClosest, thePoint);

    return (npoints);
  }

  //=======================================================================
  // bool Nearest ( double & theRadius,
  //                T & theClosest,
  //                const T& thePoint ) const
  //
  // Private function to search a NFmiNearTree for the object closest
  // to some probe point, thePoint.
  //
  // This function is only called by NearestNeighbour.
  //
  // theRadius is the smallest currently known distance of an object from
  // the probe point.
  //
  // theClosest is an object of the templated type and is the returned
  // closest point to the probe point that can be found in the NFmiNearTree
  //
  // theObject  is the probe point
  //
  // the return value is true only if a point was found within theRadius
  //
  //=======================================================================

  bool Nearest(double& theRadius, T& theClosest, const T& thePoint) const
  {
    double tmpradius;
    bool found = false;

    // first test each of the left and right positions to see if
    // one holds a point nearer than the nearest so far discovered.

    if ((itsLeftObject != 0) && ((tmpradius = Distance(thePoint, *itsLeftObject)) <= theRadius))
    {
      theRadius = tmpradius;
      theClosest = *itsLeftObject;
      found = true;
    }

    if ((itsRightObject != 0) && ((tmpradius = Distance(thePoint, *itsRightObject)) <= theRadius))
    {
      theRadius = tmpradius;
      theClosest = *itsRightObject;
      found = true;
    }

    // Now we test to see if the branches below might hold an object
    // nearer than the best so far found. The triangle rule is used
    // to test whether it's even necessary to descend.

    if ((itsLeftBranch != 0) && ((theRadius + itsMaxLeft) >= Distance(thePoint, *itsLeftObject)))
      found |= itsLeftBranch->Nearest(theRadius, theClosest, thePoint);

    if ((itsRightBranch != 0) && ((theRadius + itsMaxRight) >= Distance(thePoint, *itsRightObject)))
      found |= itsRightBranch->Nearest(theRadius, theClosest, thePoint);

    return (found);
  }

  //=======================================================================
  // bool FindFarthest ( double & theRadius,
  //                     T & theFarthest,
  //                     const T & thePoint ) const
  //
  // Private function to search a NFmiNearTree for the object farthest
  // from some probe point, thePoint.
  //
  //  This function is only called by FarthestNeighbor.
  //
  // theRadius is the largest currently known distance of an object from
  //           the probe point.
  //
  // theFarthest is an object of the templated type and is the returned
  //             farthest point from the probe point that can be found
  //             in the NFmiNearTree
  //
  // thePoint is the probe point
  //
  // the return value is true only if a point was found (should only
  // be false for an empty tree)
  //
  //=======================================================================

  bool FindFarthest(double& theRadius, T& theFarthest, const T& thePoint) const
  {
    double tmpradius;
    bool found = false;

    // first test each of the left and right positions to see if
    // one holds a point farther than the farthest so far discovered.
    // the calling function is presumed initially to have set theRadius to a
    // negative value before the recursive calls to FindFarthestNeighbor

    if ((itsLeftObject != 0) && ((tmpradius = Distance(thePoint, *itsLeftObject)) >= theRadius))
    {
      theRadius = tmpradius;
      theFarthest = *itsLeftObject;
      found = true;
    }

    if ((itsRightObject != 0) && ((tmpradius = Distance(thePoint, *itsRightObject)) >= theRadius))
    {
      theRadius = tmpradius;
      theFarthest = *itsRightObject;
      found = true;
    }

    // Now we test to see if the branches below might hold an object
    // farther than the best so far found. The triangle rule is used
    // to test whether it's even necessary to descend.

    if ((itsLeftBranch != 0) && ((theRadius - itsMaxLeft) <= Distance(thePoint, *itsLeftObject)))
      found |= itsLeftBranch->FindFarthest(theRadius, theFarthest, thePoint);

    if ((itsRightBranch != 0) && ((theRadius - itsMaxRight) <= Distance(thePoint, *itsRightObject)))
      found |= itsRightBranch->FindFarthest(theRadius, theFarthest, thePoint);
    return (found);
  }
};

}  // namespace Imagine


// ======================================================================
