//© Ilmatieteenlaitos/software by Marko
//  Original 29.09.1998
//
// Luokka itseasiassa piirtaa HSADE1:n erikoisella synop-fontilla.
//
//Ver. xx.xx.xxxx/Marko
//-------------------------------------------------------------------- NFmiStationIndexTextView.h

#pragma once

#include "NFmiStationTextView.h"
#include "NFmiImageMap.h"
//_________________________________________________________ NFmiStationIndexTextView

class NFmiToolBox;
class NFmiLocation;
class NFmiArea;
class NFmiIndexMessageList;
class NFmiDrawParam;

class NFmiStationIndexTextView : public NFmiStationTextView
{

public:
    NFmiStationIndexTextView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							 ,NFmiToolBox * theToolBox
							 ,NFmiDrawingEnvironment * theDrawingEnvi
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,FmiParameterName theParamIdent
							 ,NFmiIndexMessageList * theIndexedWordList
							 ,NFmiPoint theOffSet
							 ,NFmiPoint theSize
							 ,int theRowIndex
                             ,int theColumnIndex);
    virtual  ~NFmiStationIndexTextView (void);
   void Draw(NFmiToolBox *theGTB);
   std::string Value2ToolTipString(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType);
   bool IsSingleSymbolView(void) { return true; }; // Tästä luokasta alaspäin tulostetaan aina vain yksi merkki (erikois fonteilla) tai muu symboli

protected:
   int GetApproxmationOfDataTextLength(void);
   bool PrepareForStationDraw(void) override;
   float ViewFloatValue(void) override;
   void DrawData (void);
   void ModifyTextEnvironment (void);
   NFmiString GetPrintedText (float theValue);

  NFmiColor GetBasicParamRelatedSymbolColor(float theValue) override;
  void ModifyTextColor(float theValue) override;

   NFmiIndexMessageList * itsIndexedWordList;
   bool fUseWeatherAndCloudinessForInterpolation;


};

class NFmiStationFogTextView : public NFmiStationIndexTextView
{

public:
    NFmiStationFogTextView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							 ,NFmiToolBox * theToolBox
							 ,NFmiDrawingEnvironment * theDrawingEnvi
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,FmiParameterName theParamIdent
							 ,NFmiIndexMessageList * theIndexedWordList
							 ,NFmiPoint theOffSet
							 ,NFmiPoint theSize
							 ,int theRowIndex
                             ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
													 ,theToolBox
													 ,theDrawingEnvi
													 ,theDrawParam
													 ,theParamIdent
													 ,theIndexedWordList
													 ,theOffSet
													 ,theSize
													 ,theRowIndex
                                                     ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText (float theValue);
   void ModifyTextColor(float theValue) override;

private:
	float FogValueToSymbolIndex(float theFogValue); // NoFog = 0, ModerateFog = 1, DenseFog = 2;
};

class NFmiTotalCloudinessSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiTotalCloudinessSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
									,NFmiToolBox * theToolBox
									,NFmiDrawingEnvironment * theDrawingEnvi
									,boost::shared_ptr<NFmiDrawParam> &theDrawParam
									,FmiParameterName theParamIdent
									,NFmiIndexMessageList * theIndexedWordList
									,NFmiPoint theOffSet
									,NFmiPoint theSize
									,int theRowIndex
                                    ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
															,theToolBox
															,theDrawingEnvi
															,theDrawParam
															,theParamIdent
															,theIndexedWordList
															,theOffSet
															,theSize
															,theRowIndex
                                                            ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText(float theValue);
   NFmiColor GetBasicParamRelatedSymbolColor(float theValue) override;
};

class NFmiPrecipitationFormSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiPrecipitationFormSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
										,NFmiToolBox * theToolBox
										,NFmiDrawingEnvironment * theDrawingEnvi
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
										,FmiParameterName theParamIdent
										,NFmiIndexMessageList * theIndexedWordList
										,NFmiPoint theOffSet
										,NFmiPoint theSize
										,int theRowIndex
                                        ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
																,theToolBox
																,theDrawingEnvi
																,theDrawParam
																,theParamIdent
																,theIndexedWordList
																,theOffSet
																,theSize
																,theRowIndex
                                                                ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText(float theValue);
   NFmiColor GetBasicParamRelatedSymbolColor(float theValue) override;

};

class NFmiRawMirriFontSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiRawMirriFontSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , NFmiDrawingEnvironment * theDrawingEnvi
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

protected:
    void ModifyTextEnvironment(void);
    NFmiString GetPrintedText(float theValue);
    NFmiPoint GetSpaceOutFontFactor(void);
    NFmiColor GetBasicParamRelatedSymbolColor(float theValue) override;

};

class NFmiClCmChSymbolTextView : public NFmiTotalCloudinessSymbolTextView
{

public:
    NFmiClCmChSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,NFmiDrawingEnvironment * theDrawingEnvi
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamIdent
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex)
    :NFmiTotalCloudinessSymbolTextView(theMapViewDescTopIndex, theArea
															,theToolBox
															,theDrawingEnvi
															,theDrawParam
															,theParamIdent
															,0
															,theOffSet
															,theSize
															,theRowIndex
                                                            ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText(float theValue);
};

class NFmiCloudSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiCloudSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,NFmiDrawingEnvironment * theDrawingEnvi
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamIdent
							,NFmiIndexMessageList * theIndexedWordList
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
													,theToolBox
													,theDrawingEnvi
													,theDrawParam
													,theParamIdent
													,theIndexedWordList
													,theOffSet
													,theSize
													,theRowIndex
                                                    ,theColumnIndex)
    ,itsGeneralFontSize()
	{};

protected:
	void DrawData(void);
	NFmiString GetPrintedText(float theValue);
    void ModifyTextEnvironment(void);
    NFmiColor GetBasicParamRelatedSymbolColor(float theValue) override;

	NFmiPoint itsGeneralFontSize; // tämä otetaan talteen ModifyTextEnvironment jotta fontti koko voidaan muuttaa sitten cloud symbolista riippuen
};

class NFmiBetterWeatherSymbolView : public NFmiStationIndexTextView
{

public:
    NFmiBetterWeatherSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , NFmiDrawingEnvironment * theDrawingEnvi
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

    static void InitBetterWeatherSymbolMap(const std::string &theWomlDirectory);

protected:
    void DrawSymbols(void);
    void DrawData(void);
    NFmiPoint GetSpaceOutFontFactor(void);
    void ModifyTextEnvironment(void);

    static NFmiImageMap itsBetterWeatherSymbolMap;
};

class NFmiSmartSymbolView : public NFmiStationIndexTextView
{

public:
    NFmiSmartSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , NFmiDrawingEnvironment * theDrawingEnvi
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

    static void InitSmartSymbolMap(const std::string &theWomlDirectory);

protected:
    void DrawSymbols(void);
    void DrawData(void);
    NFmiPoint GetSpaceOutFontFactor(void);
    void ModifyTextEnvironment(void);

    static NFmiImageMap itsSmartSymbolMap;
};
