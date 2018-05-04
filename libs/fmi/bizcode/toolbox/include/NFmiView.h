//© Ilmatieteenlaitos/Persa.
//Original 02.12.1994
// 
// 
//                                  
//Ver. 02.12.1994
//Ver. 24.04.1997/Persa
//Ver. 11.05.1997/Persa itsBackgroundColor added
//Ver. 14.05.1997/Persa SetBorderWidth method
//--------------------------------------------------------------------------- NFmiView.h

#pragma once

#include "NFmiRect.h"
#include "NFmiDrawingItem.h"

class NFmiToolBox;
class NFmiVoidPtrList;
class NFmiColor;
//_________________________________________________________ NFmiView
class NFmiView : public NFmiDrawingItem
{
 public:

                               NFmiView(const NFmiRect &theRect
                                       ,NFmiToolBox *theToolBox = 0
                                       ,const NFmiView *theEnclosure = 0
                                       ,NFmiDrawingEnvironment *theEnvironment = 0
                                       ,unsigned long theIdent = kNotAnItem);
                                         
                               NFmiView(const NFmiView &/*theView*/) : NFmiDrawingItem() {};
                                 
                      virtual ~NFmiView(void); 

                   bool  operator== (const NFmiView &theView) const;
                   bool  operator!= (const NFmiView &theView) const;
                     NFmiView &operator= (const NFmiView &theView);

                     const NFmiRect&  GetFrame(void) const {return *itsAbsoluteRect;};
                  NFmiToolBox *GetToolBox(void) const {return itsToolBox;};
//////                         void  SetRect(const NFmiRect &theRect);
                         void  SetAbsoluteRect(void);
                         void  SetRelativeRect(const NFmiRect &theRect);

					 const NFmiRect&  GetAbsoluteRect(void) const {return *itsAbsoluteRect;};
                     const NFmiRect&  GetRelativeRect(void) const {return *itsRelativeRect;};
                     NFmiRect  GetDrawingArea(void) const;
                         int   GetBorderWidth(void) const 
                                     {return static_cast<int>(itsBorderWidth);}
                   NFmiPoint   GetRelativeBorderWidth(void) const; 


                         void  AddSubView(NFmiView *theSubView);

///////                 virtual void  CalcRect(void);
   
                        void  SetBackColor(const NFmiColor &newColor);
                        void  SetBorderWidth(int &newWidth) 
						        {itsBorderWidth = static_cast<double>(newWidth);}
            virtual NFmiRect  BuildFrame(const NFmiColor &theFrameColor
                                        ,const NFmiColor &theFillColor
                                        ,NFmiVoidPtrList &theDrawingList);
            virtual NFmiRect  BuildBackground(const NFmiColor &theFillColor
                                             ,NFmiVoidPtrList &theDrawingList);

 protected:
    NFmiColor *itsBackgroundColor;

// private:
  
  NFmiToolBox *itsToolBox;
  NFmiRect *itsRelativeRect;
  NFmiRect *itsAbsoluteRect;
  const NFmiView *itsEnclosure;
  double itsBorderWidth;

  bool itsFrame;

   NFmiVoidPtrList *itsSubViews;
};

typedef NFmiView* PNFmiView;




//Inlines

//___________________________________________________________ operator==
inline
bool NFmiView::operator== (const NFmiView &theView) const
{
  return (GetIdent() == theView.GetIdent());
}
//___________________________________________________________ operator==
inline
bool NFmiView::operator!= (const NFmiView &theView) const
{
  return !operator== (theView);
}
//___________________________________________________________ GetDrawingArea
inline
NFmiRect NFmiView::GetDrawingArea(void) const
{
  NFmiRect theFrame(GetFrame());
  NFmiPoint relBorderWidth(GetRelativeBorderWidth());
  theFrame.Inflate(-relBorderWidth.X(), -relBorderWidth.Y());
  return theFrame;
}

