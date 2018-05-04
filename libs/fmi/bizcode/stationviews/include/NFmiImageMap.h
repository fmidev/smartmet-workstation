#pragma once

#include <map>

namespace Gdiplus
{
    class Bitmap;
}

// T‰h‰n luokkaan luetaan mm. Mirwa analyyseiss‰ k‰ytettyj‰ kuvia.
// Kuvat mapataan taikasanan taakse std::map:iin.
// Kullekin kuvalle on kaksi kokoa, yksi ruudulle ja toinen printterille.
class NFmiImageMap
{
public:
    class ImageHolder
    {
        public:
        ImageHolder(void);
        ImageHolder(const std::string &theImageFile);

        void ReadImage(void);

        // HUOM! Gdiplus::Bitmap-olioita ei tarvitse tuhota, Gdiplus huolehtii siit‰ automaattisesti.
    	Gdiplus::Bitmap *itsImage;
        std::string itsImageFile;
    };

    class ImagesHolder
    {
        public:
        ImagesHolder(void);
        ImagesHolder(const std::string &theScreenImageFile, const std::string &thePrintedImageFile);

        void ReadImages(void);

        ImageHolder itsScreenImageHolder;
        ImageHolder itsPrinterImageHolder;
    };

    NFmiImageMap(void);
	// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
    // HUOM! heitt‰‰ poikkeuksia ep‰onnistuessaan
	void Initialize(const std::string &theImageBaseFolder, const std::string &theInitFile);  
    bool Initialized(void) const {return fInitialized;}
    bool Operational(void) const {return fOperational;}

    Gdiplus::Bitmap* GetSymbolImage(const std::string &theSymbolCodeStr, bool fPrinted);
    Gdiplus::Bitmap* GetRightSizeImage(double drawedSymbolSizeInPixels, bool isPrinting, const std::string &theSymbolCode);

private:
    std::pair<std::string, ImagesHolder> GetImageHolderInfo(const std::string &theConfFileLine, const std::string &theConfFileName);

	std::string itsImageBaseFolder;
    std::map<std::string, ImagesHolder> itsSymbolMap;

    // Onko Initialize -metodia kutsuttu?
	bool fInitialized;
    // Onnistuiko initialisointi?
	bool fOperational;
};
