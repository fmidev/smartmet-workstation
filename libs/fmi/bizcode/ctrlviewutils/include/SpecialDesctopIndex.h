#pragma once

namespace CtrlViewUtils
{
	// SmartMetin eri näytöillä on indeksi, jolla ne voidaan erotella.
	// Tähän on laitettu muutama erikois indeksi.
	enum SpecialDesctopIndex
	{
		kFmiMaxMapDescTopIndex = 2, // karttanäyttöjen indeksit 0-2
		kFmiCrossSectionView = 98,
		kFmiTimeSerialView = 99
	};
}
