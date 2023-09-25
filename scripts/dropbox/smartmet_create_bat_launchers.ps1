# Kun tulee uusi versio, laitetaan uuden version bin‰‰rit tiettyyn hakemistoon (esim. MetEditor_5_13_28_0),
# josta sitten eri konfiguraatioiden k‰ynnistys batch tiedostot luodaan.
# Versionumero annetaan pisteill‰ '.' eroteltuna, sen avulla rakennetaan myˆs jakelukansion osa polkua, miss‰ 
# versionumerot on taas eroteltu alaviivoilla '_'.
$Version = "5.13.32.0"
# Rakennetaan jakelukansion nimi versiosta.
$DistributionFolder = "MetEditor_" + $Version
$DistributionFolder = $DistributionFolder.replace('.', '_')
# Aseta shortcutien talletuspolku ja muista laittaa suhteessa t‰m‰n powerShell skriptin k‰ynnistyshakemistoon,
# muuten generoidut suhteelliset batch skriptit eiv‰t toimi oikein.
# PowerShell skriptin sijainti: path_to_dropbox\SmartMet
# Luotavien launch-batch skriptien sijainti: path_to_this_PowerShellScript\DropboxSecure_SmartMetLaunchers\versionNumberDirectory
$LaunchBatSaveDirectoryPath = "DropboxSecure_SmartMetLaunchers\" + $Version

# Varmistetaan ett‰ kansio on olemassa, -Force optio est‰‰ varoitukset skriptin ajossa, jos hakemisto on jo olemassa.
New-Item -ItemType Directory -Force -Path $LaunchBatSaveDirectoryPath

# Kunkin jakeluversion bin‰‰rikansio suhteessa launch-batch tiedoston sijaintiin
$SmartMetBinFolder = "..\..\$DistributionFolder\bin_x64"

function createLaunchBatchFile
{
# Funktiossa on kolme parametria:
# 1) K‰ytetyn p‰‰konfiguraation tiedostonimi
# 2) Konfiguraation nimen osa, joka laitetaan osaksi SmartMetin p‰‰karttan‰ytˆn otsikkoon
# 3) Konfiguraation nimen osa, joka laitetaan luodun k‰ynnistysbatch tiedostonimen osaksi
	
  param ( [string]$ConfFileName, [string]$TitleName , [string]$LauncherTitleName)

  $LauncherBatFile = $LaunchBatSaveDirectoryPath + "\" + $Version + " - " + $LauncherTitleName + ".bat"

  New-Item -ItemType File -Force -Path $LauncherBatFile
# 1. rivi tehd‰‰n Set-Content:illa, jolloin kirjoitetaan mahdollisen aikaisemman tiedoston yli
  $ValueLine1 = "rem Making relative jump to wanted SmartMet version's binary directory (should be Dropbox safe)"
  Set-Content $LauncherBatFile $ValueLine1
# Seuraavat rivit tehd‰‰n Add-Content:illa, jolloin ne tulevat vain 1. rivin per‰‰n
  $ValueLine2 = "cd " + $SmartMetBinFolder
  Add-Content $LauncherBatFile $ValueLine2
  Add-Content $LauncherBatFile ''
  $ValueLine3 = "rem Using start to launch SmartMet, this prevents the opening of the extra dos-box window"
  Add-Content $LauncherBatFile $ValueLine3
  $ValueLine4 = "start" + " `"`"" + " `"SmartMet.exe`"" + " -p `"..\Control\$ConfFileName`"" + " -t `"SmartMet $Version $TitleName`""
  Add-Content $LauncherBatFile $ValueLine4

}

# Luodaan set-shortcut funktion avulla kaikille FMI:ss‰ k‰ytetyille konfiguraatioille omat shortcut tiedostot.

createLaunchBatchFile "smartmet_scand.conf" "Scand" "Scand"
createLaunchBatchFile "smartmet_sap_scand.conf" "ASK - Scand" "Scand - ASK"
createLaunchBatchFile "smartmet_tur_scand.conf" "TUR - Scand" "Scand - TUR"
createLaunchBatchFile "smartmet_regoff_scand.conf" "RegOff - Scand" "Scand - RegOff"
createLaunchBatchFile "smartmet_euro.conf" "Euro" "Euro"
createLaunchBatchFile "smartmet_regoff_euro.conf" "Euro - RegOff" "Euro - RegOff"
createLaunchBatchFile "smartmet_world.conf" "World" "World"

read-host ìPress ENTER to continue...î
