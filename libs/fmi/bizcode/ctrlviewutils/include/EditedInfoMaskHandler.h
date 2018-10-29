#pragma once

#include "boost/shared_ptr.hpp"

class NFmiFastQueryInfo;

// Luokka asettaa editoidun datan maskin halutuksi, ottaa vanhan asetuksen talteen
// Ja kun tullaan pois laskenta scopesta, palauttaa vanhan maskin takaisin.
class EditedInfoMaskHandler
{
    unsigned long oldMask_ = 0;
    boost::shared_ptr<NFmiFastQueryInfo> &editedInfo_;
public:
    EditedInfoMaskHandler(boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, unsigned long newMask);
    ~EditedInfoMaskHandler();
};
