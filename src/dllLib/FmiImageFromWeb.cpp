#include "StdAfx.h"
#include "FmiImageFromWeb.h"

CFmiImageFromWeb::CFmiImageFromWeb(void)
{
}

CFmiImageFromWeb::~CFmiImageFromWeb(void)
{
}

#using <system.dll>
#using <system.drawing.Dll>

bool CFmiImageFromWeb::GetImage(void)
{
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	using namespace System;
	using namespace System::Net;

	String ^txtURL = gcnew String("http://geoscape.nrcan.gc.ca/calgary/images/satellite.jpg");
	HttpWebRequest ^myReq = dynamic_cast<HttpWebRequest^>(WebRequest::Create(txtURL));
	HttpWebResponse ^myRes = safe_cast<HttpWebResponse^>(myReq->GetResponse());

	Drawing::Image ^myImage = Drawing::Image::FromStream(myRes->GetResponseStream());

	myImage->Save("d:\\data\\imageFromWeb.png", Drawing::Imaging::ImageFormat::Jpeg);

	return false;
}
