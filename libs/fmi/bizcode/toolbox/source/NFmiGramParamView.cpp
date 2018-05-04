//© Ilmatieteenlaitos/Persa.
//12.5.1995   Persa
//
//Ver. 12.5.1995
//Ver. 11.5.1997/Persa BuildBackground changed
//--------------------------------------------------------------------------- ngrparav.cpp
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "NFmiGramParamView.h"

//___________________________________________________________ NFmiGramParamView
NFmiGramParamView::NFmiGramParamView(const NFmiRect &theRect
                                    ,NFmiToolBox *theToolBox
                                    ,NFmiView *theEnclosure
                                    ,NFmiDrawingEnvironment *theEnvironment
                                    ,unsigned long theIdent)
         : NFmiMetaFileView(theRect
                           ,theToolBox
                           ,theEnclosure
                           ,theEnvironment
                           ,theIdent)
{
//  itsDataArea = new NFmiRect(NFmiPoint(0.1, 0.1),NFmiPoint(0.9, 0.9));
//  itsDataView = new NFmiGramParamDataView(*itsDataArea, 0, this);
//  AddSubView(itsDataView);
}
//___________________________________________________________ ~NFmiGramParamView
NFmiGramParamView::~NFmiGramParamView(void)
{
}
//_________________________________________________________________________ CollectOwnDrawingItems

NFmiVoidPtrList &NFmiGramParamView::CollectOwnDrawingItems(NFmiVoidPtrList &theDrawingList)
{

  // Onko funktiolla sivuvaikutuksia? Tulosta ei kayteta
  static_cast<void>(BuildBackground(*itsBackgroundColor, theDrawingList));

///  NFmiRect theFrame(GetDrawingArea());

/*****
  
  NFmiDrawingEnvironment theEnvironment;
  theEnvironment.SetFrameColor(TFmiColor(0,0,1));
  int x_size = 2;
  int y_size = 2;
  theEnvironment.SetPenSize(NFmiPoint(x_size,y_size));

  NFmiLine *aLine = new NFmiLine(NFmiPoint(theFrame.Right(),theFrame.Top())
                                ,NFmiPoint(theFrame.Left(),theFrame.Bottom())
                                ,0  //this
                                ,&theEnvironment);

  theDrawingList.Add((void *)aLine);
  
							//TŠmŠn pitŠisi olla View-luokassa
//  itsDataView->CollectDrawingItems(theDrawingList);
*****/
  return theDrawingList;
}
