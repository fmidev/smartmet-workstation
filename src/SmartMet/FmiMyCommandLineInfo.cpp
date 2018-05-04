
#include "stdafx.h"
#include "FmiMyCommandLineInfo.h"

void CFmiMyCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	static bool skipNextParam = false;
	if(skipNextParam)
	{
		skipNextParam = false;
	}
	else
	{
		if(bFlag && pszParam[0] == 'p')
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 't')
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 's')
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'd')
		{
			skipNextParam = false;
		}
		else if(bFlag && pszParam[0] == 'n')
		{
			skipNextParam = false;
		}
		else if(bFlag && pszParam[0] == 'v')
		{
			skipNextParam = false;
		}
        else if(bFlag && pszParam[0] == 'c')
        {
            skipNextParam = false;
        }
        else
		{
			if (bFlag)
				CCommandLineInfo::ParseParamFlag(CT2A(pszParam));
			else
				CCommandLineInfo::ParseParamNotFlag(pszParam);
			ParseLast(bLast);
		}
	}
}
