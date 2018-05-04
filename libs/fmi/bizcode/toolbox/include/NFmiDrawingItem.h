//© Ilmatieteenlaitos/Persa.
//Original 13.10.1995
// 
// 
//                                  
//Ver. xx.xx.xxxx
// lisäys 250996/LW
// Changed 12.06.1998/Marko	Added new identifier for pixmap.
//--------------------------------------------------------------------------- NFmiDrawingItem.h

#pragma once

enum ShapeIdentifier
{
  kNotAnItem,
  kDrawingItem,
  kColorItem,
  kPenItem,
  kFillItem,
  kBitmap,  // +250996/LW, 091096/LW  Map->map
  kTextItem,
  kShape,
  kLineShape,
  kRectangleShape,
  kShapeList,
  kSymbol,
  kPolylineShape,
  kTextShape,
  kPixmap // 12.6.1998/Marko
} ;

class NFmiDrawingEnvironment;

//_________________________________________________________ NFmiDrawingItem
class NFmiDrawingItem
{
public:

	NFmiDrawingItem(const NFmiDrawingEnvironment *theEnvironment = 0
	,unsigned long theIdent = kDrawingItem);

	virtual ~NFmiDrawingItem(void); 

	bool  operator== (const NFmiDrawingItem &theDrawingItem) const;
	bool  operator!= (const NFmiDrawingItem &theDrawingItem) const;
	NFmiDrawingItem& operator= (const NFmiDrawingItem &theDrawingItem);


	ShapeIdentifier GetIdentifier(void) const {return static_cast<ShapeIdentifier>(itsIdent);}
	void SetIdentifier(ShapeIdentifier theIdentifier) {itsIdent = static_cast<unsigned long>(theIdentifier);}

	NFmiDrawingEnvironment *GetEnvironment(void) const;

	virtual void SetUpEnvironment(void);
	unsigned long  GetIdent(void) const {return itsIdent;};
	virtual void  CreateIdent(void);

private:

	unsigned long           itsIdent;
	NFmiDrawingEnvironment *itsEnvironment;
};

typedef NFmiDrawingItem* PNFmiDrawingItem;




//Inlines

//___________________________________________________________ operator==
inline
bool NFmiDrawingItem::operator== (const NFmiDrawingItem &theDrawingItem) const
{
  return (itsIdent == theDrawingItem.itsIdent);
}
//___________________________________________________________ operator!=
inline
bool NFmiDrawingItem::operator!= (const NFmiDrawingItem &theDrawingItem) const
{
  return !operator== (theDrawingItem);
}
//___________________________________________________________ GetEnvironment
inline
NFmiDrawingEnvironment *NFmiDrawingItem::GetEnvironment(void) const
{
  return itsEnvironment;
}


