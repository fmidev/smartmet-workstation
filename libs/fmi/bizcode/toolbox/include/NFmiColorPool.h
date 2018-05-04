//© Ilmatieteenlaitos/Lasse.
//Original 09.10.1996
// 
// 
//                                  
// Mukautettu NFmiDataPool:sta
// 260996/LW Data() lisätty
// 081096/LW  " pois
//
// 22.07.1997/Marko	Changed to derive from NFmiSaveBase and then 
//					added Read() and Write() -methods.
// 221096/LW missingColor lisätty
// 22.07.1997/Marko	Added Read() and Write() -methods.
//
// 10.09.97/EL added operator=
// 10.11.97/EL added non-filtering transparency (or "alpha channel") for color pool
// 12.11.97/EL added operator==
// 12.11.97/EL added operator+=
// 12.11.97/EL added UpdateTransparency()
// 18.11.97/EL combined separate R-, G-, and B-color channel arrays into a single
//					RGBA color array. Added Assign(), GetRGBA(), SetRGBA() and a new Init() method
//--------------------------------------------------------------------------- ncolorpool.h

#pragma once

#include "NFmiColor.h" //13.11.97/EL


//_________________________________________________________ NFmiDataPool
class NFmiColorPool
{
 public:

   NFmiColorPool(void)
			: itsSize(0)
			/*
			, itsRData(0)
			, itsGData(0)
			, itsBData(0)
			, itsAData(0)
			*/
			, itsRGBA(0) // 18.11.97/EL 
			, itsIndex(0)
              {}; 
   NFmiColorPool(const NFmiColorPool &thePool);

   virtual           ~NFmiColorPool(void) {
														 /*
														 if(itsRData) delete [] itsRData;
                                           if(itsGData) delete [] itsGData;
                                           if(itsBData) delete [] itsBData;
														 if(itsAData) delete [] itsAData;
														 */
														 if(itsRGBA)  delete [] itsRGBA; // 18.11.97/EL														 
														};
 

			bool	Init(unsigned long theXNumber //kun ColorGrid tehdään X ja YNumber voidaan siirtää sille
								 ,unsigned long theYNumber
								 ,unsigned char *theRData
								 ,unsigned char *theGData
								 ,unsigned char *theBData);

			bool  Init(unsigned long theXNumber // 19.11.97/EL
		 						 ,unsigned long theYNumber);

			bool	Assign(const NFmiColorPool &theColorPool);  // 19.11.97/EL 

		bool		UpdateTransparency(unsigned char *theAData);

//	   unsigned char* RData(void) {return itsRData;}; //testataan
      unsigned long  Size(void) const {return itsSize;};
      unsigned long  Index(void) const {return itsIndex;};
      unsigned long  XNumber(void) const {return itsXNumber;};
      unsigned long  YNumber(void) const {return itsYNumber;};


		/*!! 18.11.97/EL
			bool	IsInside(void) const {return itsIndex >= 0 && (unsigned long)itsIndex < itsSize;};
			bool	IsInside(unsigned long theIndex) const {return theIndex < itsSize;};
			bool	Index(unsigned long newIndex);
					void  First(void) {itsIndex = 0; fFirst = true;};
	            void  Last(void) {itsIndex = itsSize - 1; fLast = true;};
	      bool  Next(unsigned long numberOfSteps = 1);
	      bool  Previous(unsigned long numberOfSteps = 1);
		!!*/

			bool	IsInside(void) const { return (itsIndex < itsSize);}
			bool	IsInside(unsigned long theIndex) const {return theIndex < itsSize;};
			bool	Index(unsigned long newIndex);
					void  First(void) {itsIndex = 0; fFirst = true;};
	            void  Last(void) {itsIndex = itsSize - 4; fLast = true;};  // NOTE: -4 bytes for the last RGBA
	      bool  Next(unsigned long numberOfSteps = 4);  // NOTE: 4 bytes for  RGBA-RGBA-RGBA-....
			bool  Previous(unsigned long numberOfSteps = 4); // NOTE: 4 bytes for  RGBA-RGBA-RGBA-....


	   unsigned char  RValue(void) const;
	   unsigned char  GValue(void) const;
	   unsigned char  BValue(void) const;
	   unsigned char	AValue(void) const;

	   unsigned char  RValue(unsigned long theIndex) const;
	   unsigned char  GValue(unsigned long theIndex) const;
	   unsigned char  BValue(unsigned long theIndex) const;
	   unsigned char	AValue(unsigned long theIndex) const;

		// 18.11.97 /EL
		NFmiColor		GetRGBA(void) const;
		NFmiColor		GetRGBA(unsigned long theIndex) const;
		void				SetRGBA(NFmiColor &theColor) const;
		void				SetRGBA(unsigned long theIndex, NFmiColor &theColor) const;
		void				SetRGBA(unsigned char theRed, unsigned char theGrn, unsigned char theBlu, unsigned char theAlp = 0) const;
		void				SetRGBA(unsigned long theIndex, unsigned char theRed, unsigned char theGrn, unsigned char theBlu, unsigned char theAlp = 0) const;


		bool WriteRGB(const char *outputfilename);

		NFmiColorPool& operator= (const NFmiColorPool &theDataPool);     
		bool operator== (const NFmiColorPool &theColorPool);      
		void operator+= (NFmiColorPool &theDataPool);      
		friend NFmiColorPool operator+(const NFmiColorPool &pool1, const NFmiColorPool &pool2);

	   virtual std::ostream &Write(std::ostream &file) const;
	   virtual std::istream &Read(std::istream &file);

 public:
 
 private:
	 unsigned long  itsXNumber;
	 unsigned long  itsYNumber;
	 unsigned long  itsSize;
	 /*
	 unsigned char *itsRData; 
	 unsigned char *itsGData; 
	 unsigned char *itsBData;
	 unsigned char	*itsAData;
	 */

	 unsigned char	*itsRGBA;
	 
	 unsigned long  itsIndex;
	 bool     fFirst;
	 bool     fLast;
};
//@{ \name Globaalit NFmiColorPool-luokan uudelleenohjaus-operaatiot
inline std::ostream& operator<<(std::ostream& os, const NFmiColorPool& item){return item.Write(os);}
inline std::istream& operator>>(std::istream& is, NFmiColorPool& item){return item.Read(is);}
//@}

typedef NFmiColorPool* PNFmiColorPool;

