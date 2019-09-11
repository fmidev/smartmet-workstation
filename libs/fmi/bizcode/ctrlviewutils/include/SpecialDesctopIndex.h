#pragma once

namespace CtrlViewUtils
{
	// SmartMetin eri n�yt�ill� on indeksi, jolla ne voidaan erotella.
	// T�h�n on laitettu muutama erikois indeksi.
	enum SpecialDesctopIndex
	{
        kFmiDescTopIndexNone = -1, // 'resetoitu' indeksi, joka ei osoita mihink��n
        kFmiMaxMapDescTopIndex = 2, // karttan�ytt�jen indeksit 0-2
        kFmiSoundingView = 97,
        kFmiCrossSectionView = 98,
		kFmiTimeSerialView = 99
	};
}
