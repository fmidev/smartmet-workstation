# Kun tulee uusi versio, laitetaan uuden version bin‰‰rit tiettyyn hakemistoon (esim. MetEditor_5_13_28_0),
# josta sitten shortcutit luodaan eri konfiguraatioille.
# Versionumero annetaan pisteill‰ '.' eroteltuna, sen avulla rakennetaan myˆs jakelukansion osa polkua, miss‰ 
# versionumerot on taas eroteltu alaviivoilla '_'.
$Version = "5.13.22.1"
# Rakennetaan jakelukansion nimi versiosta.
$DistributionFolder = "MetEditor_" + $Version
$DistributionFolder = $DistributionFolder.replace('.', '_')
# Aseta shortcutien talletuspolku ja muista laittaa se Dropboxin ulkopuolelle,
# muuten mahdollisesti virheelliset shortcutit menev‰t v‰littˆm‰sti jakeluun.
$ShortcutSavePath = "D:\SmartMet"

# Laitetaan Dropbox polkuihin seuraava environmental variable, 
# josta kyseinen polku lˆytyy kultakin koneelta. N‰in saadaan yhtenev‰inen
# Windows shortcut kaikille k‰ytt‰jille. Lis‰ksi jos Dropbox temppuilee
# viel‰, riitt‰‰ kun muuttaa kyseisen asetuksen ja kaiken pit‰isi taas pelitt‰‰.
# Muuttujan nimen pit‰‰ olla %-merkkien v‰liss‰.
$FmiDropboxPathEnviVariable = "%FmiDropboxPath%"
# Dropboxissa olevan perus SmartMet kansion polku
$SmartMetFolder = "$FmiDropboxPathEnviVariable\SmartMet"
# Kunkin jakeluversion bin‰‰rikansio
$SmartMetBinFolder = "$SmartMetFolder\$DistributionFolder\bin_x64"

function set-shortcut 
{
# Funktiossa on kolme parametria:
# 1) K‰ytetyn p‰‰konfiguraation tiedostonimi
# 2) Konfiguraation nimen osa, joka laitetaan osaksi SmartMetin p‰‰karttan‰ytˆn otsikkoon
# 3) Konfiguraation nimen osa, joka laitetaan luodun shortcutin tiedostonimen osaksi
	
  param ( [string]$ConfFileName, [string]$TitleName , [string]$LinkTitleName)

  $WshShell = New-Object -comObject WScript.Shell


# Luodaan shortcutit Dropboxin ulkopuolelle, jotta ne saadaan kopioitua kerralla lopuksi oikeaan paikkaan.
# Shortcut linkkipolkua ei saa laittaa lainausmerkkien sis‰‰n ($WshShell.CreateShortcut ei salli sellaista)!
  $SourceLinkX = $ShortcutSavePath + "\" + $Version + " - " + $LinkTitleName + ".lnk"
  $SourceLinkX
  $Shortcut = $WshShell.CreateShortcut($SourceLinkX)

# SmartMet executablen koko polku
# Huom! Polku pit‰‰ laittaa lainausmerkkien sis‰‰n
# Huom2! $Shortcut.TargetPath arvo korjaa itsens‰, jos annetussa polussa on erilaiset kirjain koot,
# esim. alun 'smartmet' muuttuu 'SmartMet':iksi, jos juuren smartmet polku on oikeasti 'SmartMet'
  $TargetPathX = '"' + $SmartMetBinFolder + "\SmartMet.exe" + '"'
  $TargetPathX
  $Shortcut.TargetPath = $TargetPathX

# Argumentti osio pit‰‰ rakentaa osissa, muuten tulee hankaluuksia lainausmerkkien kanssa, jotka tulee polkuihin.
# Huomioi myˆs ett‰ -p ja -t argumentteihin pit‰‰ lis‰t‰ spaceja, muuten ei toimi.
  $Argument1 = "-p "
  $Argument2 = "`"$SmartMetFolder\Control\$ConfFileName`""
  $Argument3 = " -t "
  $Argument4 = "`"SmartMet $Version - $TitleName`""
  $ArgumentsX = $Argument1 + $Argument2 + $Argument3 + $Argument4
  $ArgumentsX
  $Shortcut.Arguments = $ArgumentsX

# Huom! $Shortcut.WorkingDirectory:iin ei saa sitten itse laittaa lainausmerkkej‰ polkuun, ne tulevat sinne sitten itsest‰‰n.
  $WorkingDirectoryX = "$SmartMetBinFolder\"
  $WorkingDirectoryX
  $Shortcut.WorkingDirectory = $WorkingDirectoryX

  $Shortcut.Save()

}

# Luodaan set-shortcut funktion avulla kaikille FMI:ss‰ k‰ytetyille konfiguraatioille omat shortcut tiedostot.

set-shortcut "smartmet_scand.conf" "Scand" "Scand"
set-shortcut "smartmet_sap_scand.conf" "ASK - Scand" "Scand - ASK"
set-shortcut "smartmet_tur_scand.conf" "TUR - Scand" "Scand - TUR"
set-shortcut "smartmet_regoff_scand.conf" "RegOff - Scand" "Scand - RegOff"
set-shortcut "smartmet_euro.conf" "Euro" "Euro"
set-shortcut "smartmet_regoff_euro.conf" "Euro - RegOff" "Euro - RegOff"
set-shortcut "smartmet_world.conf" "World" "World"

read-host ìPress ENTER to continue...î
