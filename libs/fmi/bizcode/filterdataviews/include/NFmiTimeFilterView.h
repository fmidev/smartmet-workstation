// NFmiTimeFilterView.h: interface for the NFmiTimeFilterView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"

class NFmiTimeFilterView : public NFmiZoomView
{

public:
   bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove (const NFmiPoint & thePlace, unsigned long theKey) override;
   void Draw(NFmiToolBox * theGTB) override;
   NFmiTimeFilterView(NFmiToolBox * theToolBox
					 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					 ,const NFmiRect& theRect
					 ,int theIndex);
   ~NFmiTimeFilterView();
   void Update(void) override;

private:
   void DrawTimeTexts(void);
   void DrawHourLines(void);
   void ChangeTimeFilterRect(const NFmiPoint & thePlace, unsigned long theKey);
   void MoveTimeFilterRect(const NFmiPoint & thePlace, unsigned long theKey);
   NFmiPoint Relative2TimeFilterPoint(const NFmiPoint& theRelativePoint);

   NFmiRect itsFilterRelativeTimeRect; // sininen muokkauslaatikko (alue, jolta laskelmat tehd‰‰n)
   int itsGridXSize; // kuinka monta laatikkoa on aikahilassa
   int itsGridXOrigo; // t‰ll‰ tehd‰‰n origon siirto tietyiss‰ laskuissa
   int itsIndex; // dialogissa on 2 t‰ll‰ist‰ n‰yttˆ‰ (tulevaisuudessa enemm‰n?), joiden indeksi t‰ss‰
   int itsExtraHourTimeLineCount; // jos datan resoluutio on yli 1 tunti, piirret‰‰n apuviivat aikasiirtoon tunnin tarkkuudella
								  // jos arvo = 0, ei piirret‰ ylim‰‰r‰isi‰ viivoja, jos >= 2 piirret‰‰n apuviivoja
								  // esim. jos resoluutio on 3 tuntia, arvoksi tulee 3
};

