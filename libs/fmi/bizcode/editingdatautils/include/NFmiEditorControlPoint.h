//**********************************************************
// C++ Class Name : NFmiEditorControlPoint 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiEditorControlPoint.h 
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
//  Change Log     : 
// 
//**********************************************************

#pragma once

#include <iosfwd>
#include <vector>

class NFmiEditorControlPoint 
{

public:
   NFmiEditorControlPoint(int theTimeCount = 0);
   bool Init (int newSize) ;
   int Size (void) ;
   void Resize (int newSize) ;
   void ClearValues (float theClearValue) ;
   float ChangeValue (void) ;
   void ChangeValue (float newValue) ;
   std::vector<float>& ChangeValues(void); // tämä on hieman vaarallinen metodi, mutta optimointia varten tehty
   inline void TimeCount (int value) {itsTimeCount = value;}
   inline int TimeCount () const {return itsTimeCount;}
   inline void TimeIndex (int value) {itsTimeIndex = value;}
   inline int TimeIndex () const {return itsTimeIndex;}

   std::istream& Read (std::istream& file) ;
   std::ostream& Write (std::ostream& file) const;
private:
	bool IsTimeIndexOk(int theTimeIndex);

   int itsTimeCount;
   int itsTimeIndex;
   std::vector<float> itsChangeValueVector;

};

std::ostream& operator<<(std::ostream& os, const NFmiEditorControlPoint& item);
std::istream& operator>>(std::istream& is, NFmiEditorControlPoint& item);

