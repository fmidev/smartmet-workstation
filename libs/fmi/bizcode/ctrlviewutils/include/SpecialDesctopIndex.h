#pragma once

namespace CtrlViewUtils
{
	// SmartMetin eri näytöillä on indeksi, jolla ne voidaan erotella.
	// Tähän on laitettu muutama erikois indeksi.
	enum SpecialDesctopIndex
	{
        kFmiDescTopIndexNone = -1, // 'resetoitu' indeksi, joka ei osoita mihinkään
        kFmiMaxMapDescTopIndex = 2, // karttanäyttöjen indeksit 0-2
        kFmiSoundingView = 97,
        kFmiCrossSectionView = 98,
		kFmiTimeSerialView = 99
	};
}
