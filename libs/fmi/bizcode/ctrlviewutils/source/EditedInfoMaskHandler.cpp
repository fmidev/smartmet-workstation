#include "EditedInfoMaskHandler.h"
#include "NFmiFastQueryInfo.h"

EditedInfoMaskHandler::EditedInfoMaskHandler(boost::shared_ptr<NFmiFastQueryInfo> &editedInfo, unsigned long newMask)
    :editedInfo_(editedInfo)
{
    oldMask_ = editedInfo_->MaskType();
    editedInfo_->MaskType(newMask);
}

EditedInfoMaskHandler::~EditedInfoMaskHandler()
{
    editedInfo_->MaskType(oldMask_);
}
