/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse

//Originaali 5. 12.1994

//Muutettu   .1.1995/LW  expansion-konstruktori lisätty
//Muutettu   .1.1995/LW  expansion-konstruktori lisätty
//Muutettu   19.10.1995/LW  GetTime(float)
//Muutettu    6.11.1995/LW
//Muutettu   10.11.1995/LW Expand()
// 061196/LW ValueToTime() publiciksi
//060697/LW TFmiTime->NFmiTime
//*-------------------------------------------------------------------------------------*/

#pragma once

#include "NFmiScale.h"
#include "NFmiTime.h"

//_________________________________________________________ NFmiTimeScale
													 // Rel. times in minutes
class NFmiTimeScale : public NFmiScale
{
public:
  NFmiTimeScale(void);
  NFmiTimeScale(const NFmiTime& theStartTime,
				const NFmiTime& theEndTime,
				unsigned long theExpansionInMinutes = 0); // before and after
  NFmiTimeScale(const NFmiTimeScale& anOtherScale);
  virtual ~NFmiTimeScale(void);
  NFmiTimeScale& operator=(const NFmiTimeScale& anOtherScale);

  using NFmiScale::StartFromZeroOptionally;
  virtual void StartFromZeroOptionally(void) {}
  void SetStartTime(const NFmiTime& theTime);
  void SetEndTime(const NFmiTime& theTime);
  void SetTimes(const NFmiTime& theStartTime, const NFmiTime& theEndTime);
  NFmiTime StartTime(void) const;
  NFmiTime EndTime(void) const;
  NFmiTime GetTime(float theLocation) const;
  float RelTimeLocation(const NFmiTime& theTime)const;
  virtual void ExpandIfEqual(float theAdditionToBothEnds);
  virtual void Expand(float theAdditionToBothEnds);

  void Move(const NFmiTime& newTime);
  void Resize(const NFmiTime& newTime);
  void MoveLeft(const NFmiTime& newTime);
  void MoveRight(const NFmiTime& newTime);
  NFmiTime ValueToTime (float theLocation) const;

 protected:

// NFmiTime ValueToTime (float theLocation) const;
  bool TimeInside(const NFmiTime& theTime)const;
  float RelTime(const NFmiTime& theTime)const;

  NFmiTime itsStartTime;

 private:
};

