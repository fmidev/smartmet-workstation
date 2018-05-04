
#pragma once

/*-------------------------------------------------------------------------------------*/
//© Ilmatieteenlaitos/Lasse               NFmiGramIterator.h

// GramIteraattori perusluokka

// Muodostettu 18.9.96

#include "NFmiDataIdent.h"

//-----------------------------------------------------------------------------------
class NFmiGramIterator
{
 public:
				NFmiGramIterator(void);
	virtual ~NFmiGramIterator(void);

	virtual unsigned short DataNumber(void)const=0;                                                                 
	virtual void           Reset     (void)=0;
	virtual bool     Next      (void)=0;
	virtual bool     Previous  (void)=0;
	virtual NFmiString     ParamName (void)const=0;
	virtual NFmiDataIdent  Param     (void)const=0;
	virtual	float          Value     (void)const=0;
	virtual	float          Value     (FmiCounter theSubIndex)const=0;

protected:

private:
};

