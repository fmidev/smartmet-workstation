#pragma once

namespace CtrlViewUtils
{
// struckti jonne talletetaan erilaisia graafisia faktoja kuten:
// näytön pixel per mm suhde jne.
struct GraphicalInfo
{
	GraphicalInfo(void)
	:itsScreenWidthInMM(0)
	,itsScreenHeightInMM(0)
	,itsScreenWidthInPixels(0)
	,itsScreenHeightInPixels(0)
	,itsViewHeightInMM(0)
	,itsViewWidthInMM(0)
	,itsPixelsPerMM_x(0)
	,itsPixelsPerMM_y(0)
	,itsDpiX(0)
	,itsDpiY(0)
	,fInitialized(false)
	{}

	int itsScreenWidthInMM;
	int itsScreenHeightInMM;
	int itsScreenWidthInPixels;
	int itsScreenHeightInPixels;
	double itsViewHeightInMM;
	double itsViewWidthInMM;
	double itsPixelsPerMM_x;
	double itsPixelsPerMM_y;
	int itsDpiX;
	int itsDpiY;

	bool fInitialized; // arvot tarvitsee alustaa vain kerran, tässä tieto onko niin tehty
};

}
