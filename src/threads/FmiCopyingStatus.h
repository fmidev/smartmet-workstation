#pragma once

enum CFmiCopyingStatus
{
    kFmiNoCopyNeeded = 0,
    kFmiCopyWentOk = 1,
    kFmiCopyNotSuccessfull = 2,
    kFmiGoOnWithCopying = 3,
    kFmiUnpackIsDoneInSeparateProcess = 4
};
