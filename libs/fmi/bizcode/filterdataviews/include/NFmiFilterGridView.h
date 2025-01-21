// NFmiFilterGridView.h: interface for the NFmiZoomView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"
#include "NFmiColor.h"

class NFmiFilterGridView : public NFmiZoomView
{

public:
   bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove (const NFmiPoint & thePlace, unsigned long theKey) override;
   void Draw(NFmiToolBox * theGTB) override;
   NFmiFilterGridView(NFmiToolBox * theToolBox
					 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					 ,const NFmiRect& theRect
					 ,int theIndex);
   ~NFmiFilterGridView();
   void Update(void) override;

protected:
   virtual void InitGridValues(void);
   void ChangeFilterAreaRect(const NFmiPoint & thePlace);
   void MoveFilterAreaRect(const NFmiPoint & thePlace);
   NFmiPoint Relative2FolterGridPoint(const NFmiPoint& thrRelativePoint);
   void AdjustFilterGridRect(NFmiRect& theRect, bool fKeepSize = true, FmiDirection theSolidCorner = kUpLeft);

   NFmiRect itsFilterRelativeAreaRect; // sininen muokkauslaatikko (alue, jolta laskelmat tehd��n)
   int itsGridXSize; // doc:issa on m��ritelty min-,max-x ja min-,maxy filtergridille
   int itsGridYSize; // n�ist� arvoista lasketaan x- ja y-koot maxx - minx ja maxy-miny laskuilla
   int itsGridXOrigo; // t�ll� tehd��n origon siirto tietyiss� laskuissa
   int itsGridYOrigo; // t�ll� tehd��n origon siirto tietyiss� laskuissa
   int itsIndex; // dialogissa on 2 t�ll�ist� n�ytt�� (tulevaisuudessa enemm�n?), joiden indeksi t�ss�
   NFmiRect itsFilterAreaRect; // saadaan itsDoc->AreaFilterRange(itsIndex)-metodilla
   NFmiColor itsBackgroundColor;
};

