#pragma once

namespace CtrlViewUtils
{
    // SmartMetin karttanäytön näyttöruudukon aika-parametri moodi (rivi vs sarake)
    enum class MapViewMode
    {
        kNormal = 0, // rivi-parametrit, sarake-ajat
        kOneTime = 1, // joka ruudussa oman rivin parametrit, kaikille sama aika
        kRunningTime = 2 // vain yhden rivin parametrit joka ruudussa, aika eri jokaisessa ruudussa
    };

}
