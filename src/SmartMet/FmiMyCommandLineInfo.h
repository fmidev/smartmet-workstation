#pragma once

class CFmiMyCommandLineInfo : public CCommandLineInfo
{
public:
	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
};
