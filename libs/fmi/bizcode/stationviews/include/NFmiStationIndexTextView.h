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
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,FmiParameterName theParamIdent
							 ,NFmiIndexMessageList * theIndexedWordList
							 ,NFmiPoint theOffSet
							 ,NFmiPoint theSize
							 ,int theRowIndex
                             ,int theColumnIndex);
   ~NFmiStationIndexTextView (void);
   void Draw(NFmiToolBox *theGTB) override;
   std::string Value2ToolTipString(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType) override;
   bool IsSingleSymbolView(void) override { return true; }; // Tästä luokasta alaspäin tulostetaan aina vain yksi merkki (erikois fonteilla) tai muu symboli

protected:
   int GetApproxmationOfDataTextLength(std::vector<float>* sampleValues = nullptr) override;
   bool PrepareForStationDraw(void) override;
   float ViewFloatValue(bool doTooltipValue) override;
   void ModifyTextEnvironment (void) override;
   NFmiString GetPrintedText (float theValue) override;
   NFmiPoint SbdCalcFixedSymbolSize() const override;
   void SbdSetFontName() override;
   NFmiColor GetBasicParamRelatedSymbolColor(float theValue) const override;
   NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;

   NFmiIndexMessageList * itsIndexedWordList;
   bool fUseWeatherAndCloudinessForInterpolation;


};

class NFmiStationFogTextView : public NFmiStationIndexTextView
{

public:
    NFmiStationFogTextView (int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							 ,NFmiToolBox * theToolBox
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,FmiParameterName theParamIdent
							 ,NFmiIndexMessageList * theIndexedWordList
							 ,NFmiPoint theOffSet
							 ,NFmiPoint theSize
							 ,int theRowIndex
                             ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
													 ,theToolBox
													 ,theDrawParam
													 ,theParamIdent
													 ,theIndexedWordList
													 ,theOffSet
													 ,theSize
													 ,theRowIndex
                                                     ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText (float theValue) override;
   NFmiColor GetBasicParamRelatedSymbolColor(float theValue) const override;

private:
	float FogValueToSymbolIndex(float theFogValue) const; // NoFog = 0, ModerateFog = 1, DenseFog = 2;
};

class NFmiTotalCloudinessSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiTotalCloudinessSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
									,NFmiToolBox * theToolBox
									,boost::shared_ptr<NFmiDrawParam> &theDrawParam
									,FmiParameterName theParamIdent
									,NFmiIndexMessageList * theIndexedWordList
									,NFmiPoint theOffSet
									,NFmiPoint theSize
									,int theRowIndex
                                    ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
															,theToolBox
															,theDrawParam
															,theParamIdent
															,theIndexedWordList
															,theOffSet
															,theSize
															,theRowIndex
                                                            ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText(float theValue) override;
   NFmiColor GetBasicParamRelatedSymbolColor(float theValue) const override;
   NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;
};

class NFmiPrecipitationFormSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiPrecipitationFormSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
										,NFmiToolBox * theToolBox
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
										,FmiParameterName theParamIdent
										,NFmiIndexMessageList * theIndexedWordList
										,NFmiPoint theOffSet
										,NFmiPoint theSize
										,int theRowIndex
                                        ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
																,theToolBox
																,theDrawParam
																,theParamIdent
																,theIndexedWordList
																,theOffSet
																,theSize
																,theRowIndex
                                                                ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText(float theValue) override;
   NFmiColor GetBasicParamRelatedSymbolColor(float theValue) const override;

};

class NFmiRawMirriFontSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiRawMirriFontSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

protected:
    void ModifyTextEnvironment(void) override;
    NFmiString GetPrintedText(float theValue) override;
    NFmiPoint GetSpaceOutFontFactor(void) override;
    NFmiColor GetBasicParamRelatedSymbolColor(float theValue) const override;
    NFmiPoint SbdCalcFixedSymbolSize() const override;
	void SbdSetFontName() override;
    NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;

};

class NFmiClCmChSymbolTextView : public NFmiTotalCloudinessSymbolTextView
{

public:
    NFmiClCmChSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamIdent
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex)
    :NFmiTotalCloudinessSymbolTextView(theMapViewDescTopIndex, theArea
															,theToolBox
															,theDrawParam
															,theParamIdent
															,0
															,theOffSet
															,theSize
															,theRowIndex
                                                            ,theColumnIndex)
    {};

protected:
   NFmiString GetPrintedText(float theValue) override;
};

class NFmiCloudSymbolTextView : public NFmiStationIndexTextView
{

public:
    NFmiCloudSymbolTextView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamIdent
							,NFmiIndexMessageList * theIndexedWordList
							,NFmiPoint theOffSet
							,NFmiPoint theSize
							,int theRowIndex
                            ,int theColumnIndex)
    :NFmiStationIndexTextView(theMapViewDescTopIndex, theArea
													,theToolBox
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
	NFmiString GetPrintedText(float theValue) override;
    void ModifyTextEnvironment(void) override;
    NFmiColor GetBasicParamRelatedSymbolColor(float theValue) const override;
	bool SbdIsFixedSymbolSize() const override;
    NFmiPoint SbdCalcChangingSymbolSize(float value) const override;
    NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;

	NFmiPoint itsGeneralFontSize; // tämä otetaan talteen ModifyTextEnvironment jotta fontti koko voidaan muuttaa sitten cloud symbolista riippuen
};

class NFmiImageBasedSymbolView : public NFmiStationIndexTextView
{

public:
    NFmiImageBasedSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea>& theArea
        , NFmiToolBox* theToolBox
        , boost::shared_ptr<NFmiDrawParam>& theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList* theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

protected:
    NFmiPoint SbdCalcFixedSymbolSize() const override;
    NFmiSymbolColorChangingType SbdGetSymbolColorChangingType() const override;
    NFmiPoint GetSpaceOutFontFactor(void) override;
    void ModifyTextEnvironment(void) override;
    NFmiPoint SbdCalcFixedRelativeDrawObjectSize() const override;
    double CalcSymbolSizeInMM() const;

};

class NFmiBetterWeatherSymbolView : public NFmiImageBasedSymbolView
{

public:
    NFmiBetterWeatherSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

    static void InitBetterWeatherSymbolMap(const std::string &theWomlDirectory);
    static NFmiImageMap& GetBetterWeatherSymbolMap() { return itsBetterWeatherSymbolMap; }

protected:
    NFmiSymbolBulkDrawType SbdGetDrawType() const override;

    static NFmiImageMap itsBetterWeatherSymbolMap;
};

class NFmiSmartSymbolView : public NFmiImageBasedSymbolView
{

public:
    NFmiSmartSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

    static void InitSmartSymbolMap(const std::string &theWomlDirectory);
    static NFmiImageMap& GetSmartSymbolMap() { return itsSmartSymbolMap; }

protected:
    NFmiSymbolBulkDrawType SbdGetDrawType() const override;

    static NFmiImageMap itsSmartSymbolMap;
};

class NFmiCustomSymbolView : public NFmiImageBasedSymbolView
{

public:
    NFmiCustomSymbolView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
        , NFmiToolBox * theToolBox
        , boost::shared_ptr<NFmiDrawParam> &theDrawParam
        , FmiParameterName theParamIdent
        , NFmiIndexMessageList * theIndexedWordList
        , NFmiPoint theOffSet
        , NFmiPoint theSize
        , int theRowIndex
        , int theColumnIndex);

    static void InitCustomSymbolMap(const std::string &theWomlDirectory);
    static NFmiImageMap& GetCustomSymbolMap() { return itsCustomSymbolMap; }

protected:
    NFmiSymbolBulkDrawType SbdGetDrawType() const override;

    static NFmiImageMap itsCustomSymbolMap;
};
