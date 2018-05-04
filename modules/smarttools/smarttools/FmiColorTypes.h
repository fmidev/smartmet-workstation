//© Ilmatieteenlaitos/Persa.

// 1.3.1994   Ja tänään se presidentti vannoi virkavalansa	Persa
// 13.11.1997 Ja tänään se suunnittelija lisäs läpinäkyvyyden (alpha channel) väreihin  EL

#pragma once

//	typedef double FmiColorValue;
typedef float FmiColorValue;  // 10.10.2001/Marko Aikamoinen tarve ollut yhden värin kertomiseen kun
                              // ollut 4 x double = 32 tavua = 256 bittiä (kun truecolor saadaan
                              // aikaan 32 bitillä)

typedef struct
{
  FmiColorValue red, green, blue, alpha;
} FmiRGBColor;

