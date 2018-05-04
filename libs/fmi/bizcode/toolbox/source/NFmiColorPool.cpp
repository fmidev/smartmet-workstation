/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse.               ncolpool.cpp

// Originaali 9. 10. 1996/Lasse. Tehty NFmiDataPoolista
// Värimatriisiluokka. Kolme komponenttia 8 bitillä.
//                        
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
// 6.4.98/Marko	Changed all SetRGB()-methods not to overwrite memory.
//--------------------------------------------------------------------------- ncolorpool.h

#include  <memory>
#include  <fstream>

#include "NFmiColorPool.h"

const unsigned char missingColor = 200;


/*-------------------------------------------------------------------------------------*/
NFmiColorPool::NFmiColorPool(const NFmiColorPool &theColorPool)
			: itsXNumber(theColorPool.itsXNumber)
			, itsYNumber(theColorPool.itsYNumber)
			, itsSize(theColorPool.itsSize)
			/*
			, itsRData(0)
			, itsGData(0)
			, itsBData(0)
			, itsAData(0)
			*/
			, itsRGBA(0)
			, itsIndex(theColorPool.itsIndex)
			, fFirst(true)
			, fLast(true)
{
	
//18.11.97/EL removed this: Init(itsXNumber, itsYNumber,theColorPool.itsRData,theColorPool.itsGData,theColorPool.itsBData,theColorPool.itsAData);
	Assign(theColorPool);
}


//-----------------------Init()
bool NFmiColorPool::Init(unsigned long theXNumber
										,unsigned long theYNumber
										,unsigned char *theRData
										,unsigned char *theGData
										,unsigned char *theBData)
{
/* EL removed this 24.11.97
	if(!theXNumber || !theYNumber)
		return false;

   itsXNumber = theXNumber;
   itsYNumber = theYNumber;
	itsSize = itsXNumber * itsYNumber; 

	itsRData = new unsigned char[itsSize];
	itsGData = new unsigned char[itsSize];
	itsBData = new unsigned char[itsSize];

	if(itsRData && itsGData && itsBData)   //korj.101296
	{
		memcpy(itsRData, theRData, itsSize);
      memcpy(itsGData, theGData, itsSize);
      memcpy(itsBData, theBData, itsSize);

		return true;
	}
	else
	{
	//	for(unsigned long i = 0; i < itsNumber; i++)
	//		Set(i);
		return false;
	}
*/

// NOTE: This Init() only stands for compatibility with the older color pool version.
//			Expects no alpha channel input 

	if(!theXNumber || !theYNumber)
		return false;

   itsXNumber = theXNumber;
   itsYNumber = theYNumber;
	itsSize = 4 * itsXNumber * itsYNumber; // NOTE: 4 bytes for each R-G-B-A

	if(itsRGBA)
		delete itsRGBA;

	itsRGBA = new unsigned char[itsSize];

	unsigned long i,j;

	for(i = 0, j = 0;i < itsSize;i += 4,j++)
		{
		itsRGBA[i]   = theRData[j];
		itsRGBA[i+1] = theGData[j];
		itsRGBA[i+2] = theBData[j];
		itsRGBA[i+3] = 0; // 100% opacity
		}

	return true;
}

//-----------------------Init() // 18.11.97/EL
bool NFmiColorPool::Init(unsigned long theXNumber
		 								,unsigned long theYNumber)
{
	if(!theXNumber || !theYNumber)
		return false;

   itsXNumber = theXNumber;
   itsYNumber = theYNumber;
	itsSize = 4 * itsXNumber * itsYNumber; // NOTE: 4 bytes for each R-G-B-A

	if(itsRGBA) delete [] itsRGBA; 
	itsRGBA = new unsigned char[itsSize];
	
	return true;
}

//-----------------------Assign() // 18.11.97/EL
bool NFmiColorPool::Assign(const NFmiColorPool &theColorPool)
{
	if(!Init(theColorPool.itsXNumber, theColorPool.itsYNumber))
		return false;

	memcpy(itsRGBA, theColorPool.itsRGBA, itsSize); // 18.11.97/EL

	return true;
}

//-----------------------UpdateTransparency(unsigned char *theAData) //12.11.97/EL
bool NFmiColorPool::UpdateTransparency(unsigned char *theAData)
{
	if(!theAData) return false;

	unsigned long i,j;

	for(i = 0, j = 0;i < itsSize;i += 4,j++)
		{
		itsRGBA[i+3] = theAData[j];
		}

	return true;
}

//------------------------Index(unsigned long newIndex)
bool NFmiColorPool::Index(unsigned long newIndex)
{
	// NOTE: The 'newIndex' is only assumed to tell the order number for each 4 byte RGBA item,
	// NOT the absolute byte index (21.11.97/EL)  
	if(IsInside(newIndex))
	{
	itsIndex = long(4*newIndex); // The byte index for current R-G-B-A
		if(itsIndex == 0)
			fFirst = true;
		else if(newIndex == itsSize)
			fLast = true;
		return true;
	}
	else
		return false;
}

//------------------------Next(unsigned long numberOfSteps)
bool NFmiColorPool::Next(unsigned long numberOfSteps)
{
	if(fFirst)
	{
		fFirst = false;
		return true;
	}

	if(numberOfSteps && IsInside(itsIndex += numberOfSteps))
		return  true;
	else
	{
//		itsIndex = 0;
		return false;
	}
}
//------------------------Previous(unsigned long numberOfSteps)
bool NFmiColorPool::Previous(unsigned long numberOfSteps)
{
	if(fLast)
	{
		fLast = false;
		return true;
	}

	if(numberOfSteps && IsInside(itsIndex -= numberOfSteps))
		return  true;
	else
	{
//		itsIndex = 0;
		return false;
	}
}
//------------------RValue(void) const
unsigned char NFmiColorPool::RValue(void) const
{
	if(!IsInside())
		return missingColor;

	//18.11.97/EL removed this: return itsRData[itsIndex];
	return itsRGBA[itsIndex];
}
//------------------GValue(void) const
unsigned char NFmiColorPool::GValue(void) const
{
	if(!IsInside())
		return missingColor;

	//18.11.97/EL removed this: return itsGData[itsIndex];
	return itsRGBA[itsIndex+1];
}
//------------------BValue(void) const
unsigned char NFmiColorPool::BValue(void) const
{
	if(!IsInside())
		return missingColor;

		//18.11.97/EL removed this: return itsBData[itsIndex];
	return itsRGBA[itsIndex+2];
}
//------------------AValue(void) const
unsigned char NFmiColorPool::AValue(void) const
{
	if(!IsInside())
		return missingColor;

	return itsRGBA[itsIndex+3];
}

//------------------RValue(unsigned long theIndex) const
unsigned char NFmiColorPool::RValue(unsigned long theIndex) const
{
	if(theIndex >= itsSize)
		return missingColor;

	//18.11.97/EL removed this: return itsRData[theIndex];
	return itsRGBA[itsIndex];
}
//------------------GValue(unsigned long theIndex) const
unsigned char NFmiColorPool::GValue(unsigned long theIndex) const
{
	if(theIndex >= itsSize)
		return missingColor;

	//18.11.97/EL removed this: 	return itsGData[theIndex];
	return itsRGBA[itsIndex+1];
}
//------------------BValue(unsigned long theIndex) const
unsigned char NFmiColorPool::BValue(unsigned long theIndex) const
{
	if(theIndex >= itsSize)
		return missingColor;

	//18.11.97/EL removed this: return itsBData[theIndex];
	return itsRGBA[itsIndex+2];
}
//------------------AValue(unsigned long theIndex) const
unsigned char NFmiColorPool::AValue(unsigned long theIndex) const
{
	if(theIndex >= itsSize)
		return missingColor;

	return itsRGBA[itsIndex+3];
}


// 18.11.97/EL
//------------------GetRGBA(void) const
NFmiColor NFmiColorPool::GetRGBA(void) const
{
	if(itsIndex >= itsSize)
		return NFmiColor(missingColor, missingColor, missingColor);

	float red = static_cast<float>(itsRGBA[itsIndex]/255.);
	float grn = static_cast<float>(itsRGBA[itsIndex+1]/255.);
	float blu = static_cast<float>(itsRGBA[itsIndex+2]/255.);
	float alp = static_cast<float>(itsRGBA[itsIndex+3]/255.);

	return NFmiColor(red, grn, blu, alp);
}

// 18.11.97/EL
//------------------GetRGBA(unsigned long theIndex) const
NFmiColor NFmiColorPool::GetRGBA(unsigned long theIndex) const
{
	if(theIndex >= itsSize)
		return NFmiColor(missingColor, missingColor, missingColor);

	float red = static_cast<float>(itsRGBA[theIndex]/255.);
	float grn = static_cast<float>(itsRGBA[theIndex+1]/255.);
	float blu = static_cast<float>(itsRGBA[theIndex+2]/255.);
	float alp = static_cast<float>(itsRGBA[theIndex+3]/255.);

	return NFmiColor(red, grn, blu, alp);
}

// 18.11.97/EL
//------------------SetRGBA(NFmiColor) const
void NFmiColorPool::SetRGBA(NFmiColor &theColor) const
{
	if(itsIndex >= itsSize)
		{
//		itsRGBA[itsIndex]	  = missingColor;
//		itsRGBA[itsIndex+1] = missingColor;
//		itsRGBA[itsIndex+2] = missingColor;
//		itsRGBA[itsIndex+3] = missingColor;
		return;
		}

	itsRGBA[itsIndex]	  = static_cast<unsigned char>(255. * theColor.Red());
	itsRGBA[itsIndex+1] = static_cast<unsigned char>(255. * theColor.Green());
	itsRGBA[itsIndex+2] = static_cast<unsigned char>(255. * theColor.Blue());
	itsRGBA[itsIndex+3] = static_cast<unsigned char>(255. * theColor.Alpha());
}

// 18.11.97/EL
//------------------SetRGBA(NFmiColor) const
void NFmiColorPool::SetRGBA(unsigned long theIndex, NFmiColor &theColor) const
{
	if(theIndex >= itsSize)
		{
//		itsRGBA[theIndex]	  = missingColor;
//		itsRGBA[theIndex+1] = missingColor;
//		itsRGBA[theIndex+2] = missingColor;
//		itsRGBA[theIndex+3] = missingColor;
		return;
		}

	itsRGBA[theIndex]	  = static_cast<unsigned char>(255. * theColor.Red());
	itsRGBA[theIndex+1] = static_cast<unsigned char>(255. * theColor.Green());
	itsRGBA[theIndex+2] = static_cast<unsigned char>(255. * theColor.Blue());
	itsRGBA[theIndex+3] = static_cast<unsigned char>(255. * theColor.Alpha());
}

// 18.11.97/EL
//------------------SetRGBA(NFmiColor) const
void NFmiColorPool::SetRGBA(unsigned char theRed, unsigned char theGrn, unsigned char theBlu, unsigned char theAlp) const
{
	// Inserts a byte-valued RGBA color into current color pool on current index
 
	if(itsIndex >= itsSize)
		{
//		itsRGBA[itsIndex]	  = missingColor;
//		itsRGBA[itsIndex+1] = missingColor;
//		itsRGBA[itsIndex+2] = missingColor;
//		itsRGBA[itsIndex+3] = missingColor;
		return;
		}

	itsRGBA[itsIndex]	  = theRed;
	itsRGBA[itsIndex+1] = theGrn;
	itsRGBA[itsIndex+2] = theBlu;
	itsRGBA[itsIndex+3] = theAlp;
}

// 18.11.97/EL
//------------------SetRGBA(NFmiColor) const
void NFmiColorPool::SetRGBA(unsigned long theIndex, unsigned char theRed, unsigned char theGrn, unsigned char theBlu, unsigned char theAlp) const
{
	// Inserts a byte-valued RGBA color into current color pool at input index

	if(theIndex >= itsSize)
		{
//		itsRGBA[theIndex]	  = missingColor;
//		itsRGBA[theIndex+1] = missingColor;
//		itsRGBA[theIndex+2] = missingColor;
//		itsRGBA[theIndex+3] = missingColor;
		return;
		}

	itsRGBA[theIndex]	  = theRed;
	itsRGBA[theIndex+1] = theGrn;
	itsRGBA[theIndex+2] = theBlu;
	itsRGBA[theIndex+3] = theAlp;
}

//----------------------------------------------------------------------------
bool NFmiColorPool::WriteRGB(const char *outputfilename)
{
	// Save the render output file in RAW 24-bit format
	std::ofstream out(outputfilename, std::ios::binary);
	
	if(!out)
		return false;
//	cout = out;

	NFmiColor color;

	First();

	while(Next())
		{		
		  // color.SetRGB((NFmiColor &) GetRGBA());
		  color.SetRGB(static_cast<const NFmiColor &>(GetRGBA()));

		out << static_cast<unsigned char>(color.Red()  *255.)
			  << static_cast<unsigned char>(color.Green()*255.)
			  << static_cast<unsigned char>(color.Blue() *255.);
		}

	out.close();
	return true;
}


//----------
// operators
//----------
//___________________________________________________________ operator=
NFmiColorPool& NFmiColorPool::operator= (const NFmiColorPool &theColorPool)      
{
	// Assigns input color pool into current color pool.
	// NOTE: current color pool size don't matter as input color pool size
	//	will override it anyway
	
	itsSize = theColorPool.itsSize;
	itsXNumber = theColorPool.itsXNumber;
	itsYNumber = theColorPool.itsYNumber;

	/*
	if(itsRData) delete itsRData;
	if(itsGData) delete itsGData;
	if(itsBData) delete itsBData;
	if(itsAData) delete itsAData;
	*/

//	if(itsRGBA) delete itsRGBA; // 18.11.97/EL

	itsIndex = theColorPool.itsIndex;
	fFirst = true;
	fLast = true;

	Assign(theColorPool);

   return *this;
}

//___________________________________________________________ operator=
bool NFmiColorPool::operator== (const NFmiColorPool &theColorPool)      
{
	if((itsSize    != theColorPool.itsSize)    ||
		(itsXNumber != theColorPool.itsXNumber) ||
		(itsYNumber != theColorPool.itsYNumber))
		return false;

	return (memcmp(static_cast<const void *>(itsRGBA),
				   static_cast<const void *>(theColorPool.itsRGBA),
				   itsSize) == 0) ? true : false;
}

//___________________________________________________________ operator+=
void NFmiColorPool::operator+= (NFmiColorPool &theColorPool)      
{
	// Merges current "background" colorpool and the input "foreground" colorpool into one
	// color pool based on the transparency of the input foreground color pool only.
	// NOTE: the resulting "background" + "foreground" color pool itself will always be
	// completely opaque.


	if((itsXNumber != theColorPool.itsXNumber) ||
		(itsYNumber != theColorPool.itsYNumber))
		return; // Color pools should be of the same size

	if(itsRGBA)
		{
		// Transparency channel of the input color pool may be used.

		First();
		theColorPool.First();

		while(Next())
			{
			theColorPool.Next();
			
			// Create background color from the current color pool color
			NFmiColor backgroundColor(GetRGBA());

			// Create foreground color from the input colorpool color
			NFmiColor foregroundColor(theColorPool.GetRGBA());
			
			// Blend background and foreground colors
			NFmiColor summedColor(backgroundColor);
			summedColor.Overlay(foregroundColor);

			// Save blended color into current color pool
			SetRGBA(summedColor);
			}			
		}
}

//------------------------Write(ostream_withassign &file)
std::ostream& NFmiColorPool::Write(std::ostream &file) const
{
	using namespace std;

	file << itsXNumber << endl;
	file << itsYNumber << endl;
	file << itsSize << endl;
	file << itsIndex << endl;
	file << fFirst << endl;
	file << fLast << endl;
//	file << binary;
	/*
	file.write(itsRData,itsSize);//only binary mode
	file.write(itsGData,itsSize);//only binary mode
	file.write(itsBData,itsSize);//only binary mode
	*/
	file.write(reinterpret_cast<char *>(itsRGBA),itsSize);//only binary mode
//	file << text;

	return file;	
} 

//------------------------Read(ostream_withassign &file)
std::istream& NFmiColorPool::Read(std::istream &file)
{
	file >> itsXNumber;
	file >> itsYNumber;
	file >> itsSize;
	file >> itsIndex;
	file >> fFirst;
	file >> fLast;
	char buff;
	file.get(buff);  //read (lf) from stream
//	file >> binary;

	/*
	if(itsRData)
		delete [] itsRData;
	itsRData = new unsigned char[itsSize];

	if(itsGData)
		delete [] itsGData;
	itsGData = new unsigned char[itsSize];

	if(itsBData)
		delete [] itsBData;
	itsBData = new unsigned char[itsSize];

	if(itsAData)
		delete [] itsAData;
	itsAData = new unsigned char[itsSize];

	file.read(itsRData,itsSize);//only binary mode
	file.read(itsGData,itsSize);//only binary mode
	file.read(itsBData,itsSize);//only binary mode
	*/

	if(itsRGBA)
		delete [] itsRGBA;

	itsRGBA = new unsigned char[itsSize];

	file.read(reinterpret_cast<char *>(itsRGBA),itsSize);//only binary mode

//	file >> text;

	return file;	
}

NFmiColorPool operator+(const NFmiColorPool &pool1, const NFmiColorPool &pool2)
{
	long xSize1 = pool1.XNumber(), 
		 ySize1 = pool1.YNumber(),
		 xSize2 = pool2.XNumber(),
		 ySize2 = pool2.YNumber();
	long xSize = std::max(xSize1, xSize2);
	long ySize = std::max(ySize1, ySize2);
	NFmiColorPool returnPool;
	returnPool.Init(xSize,ySize);

	long index=0,
		 index1,
		 index2;
	long xInd,yInd;
	long xInd1,yInd1;
	long xInd2,yInd2;
	NFmiColor color1;
	NFmiColor color2;
	
	for(returnPool.First(); returnPool.Next(); index++)
	{
		xInd = index%xSize;
		yInd = index/xSize;
		xInd1 = static_cast<int>(static_cast<float>(xInd) * xSize1 / xSize);
		yInd1 = static_cast<int>(static_cast<float>(yInd) * ySize1 / ySize);
		xInd2 = static_cast<int>(static_cast<float>(xInd) * xSize2 / xSize);
		yInd2 = static_cast<int>(static_cast<float>(yInd) * ySize2 / ySize);
		index1 = yInd1 * xSize1 + xInd1;
		index2 = yInd2 * xSize2 + xInd2;
		color1 = pool1.GetRGBA(index1*4);
		color2 = pool2.GetRGBA(index2*4);
		color1.Overlay(color2);
		returnPool.SetRGBA(color1);
	}
	return returnPool;
}

// NFmiColor GetRGBA(unsigned long theIndex)
