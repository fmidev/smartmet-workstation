//**********************************************************
// C++ Class Name : NFmiEditorControlPoint 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiEditorControlPoint.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : editori virityksiä 2000 syksy 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : interpolation luokka 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Tues - Nov 7, 2000 
// 
//  Change Log     : xxx yyy
// 
//**********************************************************
#include "NFmiEditorControlPoint.h"
#include <iostream>
#include <iterator>

using namespace std;

NFmiEditorControlPoint::NFmiEditorControlPoint(int theTimeCount)
:itsTimeCount(theTimeCount)
,itsTimeIndex(-1)
,itsChangeValueVector(theTimeCount)
{
}

//--------------------------------------------------------
// Read 
//--------------------------------------------------------
istream& NFmiEditorControlPoint::Read(istream& file)
{
	file >> itsTimeCount;
	file >> itsTimeIndex;
	itsChangeValueVector.resize(itsTimeCount);
	for(int i=0; i<itsTimeCount; i++)
		file >> itsChangeValueVector[i];
	return file;
}
//--------------------------------------------------------
// Write 
//--------------------------------------------------------
ostream& NFmiEditorControlPoint::Write(ostream& file) const
{
	file << itsTimeCount << " " << itsTimeIndex << endl;
	std::copy(itsChangeValueVector.begin(), itsChangeValueVector.end(), ostream_iterator<double>(file, " "));
	file << endl;

	return file;
}
//--------------------------------------------------------
// Init 
//--------------------------------------------------------
bool NFmiEditorControlPoint::Init(int newSize)
{
	itsTimeCount = newSize;
	itsChangeValueVector.resize(newSize, 0);
	return true;
}
//--------------------------------------------------------
// Size 
//--------------------------------------------------------
int NFmiEditorControlPoint::Size(void)
{
   return itsTimeCount;
}
//--------------------------------------------------------
// Size 
//--------------------------------------------------------
void NFmiEditorControlPoint::Resize(int newSize)
{
   Init(newSize);
}
//--------------------------------------------------------
// ClearValues 
//--------------------------------------------------------
void NFmiEditorControlPoint::ClearValues(double theClearValue)
{
	for(size_t i=0; i < itsChangeValueVector.size(); i++)
		itsChangeValueVector[i] = theClearValue;
}

bool NFmiEditorControlPoint::IsTimeIndexOk(int theTimeIndex)
{
	if(theTimeIndex >= 0 && theTimeIndex < static_cast<int>(itsChangeValueVector.size()))
		return true;
	else
		return false;
}

//--------------------------------------------------------
// ChangeValue 
//--------------------------------------------------------
double NFmiEditorControlPoint::ChangeValue(void)
{
	if(IsTimeIndexOk(itsTimeIndex))
		return itsChangeValueVector[itsTimeIndex];
	else
		return 0;
}

std::vector<double>& NFmiEditorControlPoint::ChangeValues(void)
{
   return itsChangeValueVector;
}

//--------------------------------------------------------
// ChangeValue 
//--------------------------------------------------------
void NFmiEditorControlPoint::ChangeValue(double newValue)
{
	if(IsTimeIndexOk(itsTimeIndex))
		itsChangeValueVector[itsTimeIndex] = newValue;
}

std::ostream& operator<<(ostream& os, const NFmiEditorControlPoint& item)
{
	item.Write(os); 
	return os;
}
std::istream& operator>>(istream& is, NFmiEditorControlPoint& item)
{
	item.Read(is); 
	return is;
}
