#pragma once

namespace CtrlViewUtils
{
    enum FmiSmartMetEditingMode
    {
        kFmiEditingModeNormal = 0, // t‰m‰ on normaali operatiivinen moodi, jossa dataa ladataan automaattisesti
        kFmiEditingModeViewOnly = 1, // t‰m‰ moodi on ns. katselu moodi, jossa editointi tyˆkalut piilotetaan ja estet‰‰n
        kFmiEditingModeStartUpLoading = 2 // Uusi automaatti datan lataus moodi, jota k‰ytet‰‰n vain kun SmartMet k‰ynnistet‰‰n, jolloin data ladataan k‰ytt‰j‰n huomaamatta
    };
}
