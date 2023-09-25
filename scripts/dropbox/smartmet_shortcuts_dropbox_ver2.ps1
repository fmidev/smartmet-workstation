# Kun tulee uusi versio, laitetaan uuden version bin‰‰rit tiettyyn hakemistoon (esim. MetEditor_5_13_14_0),
# josta sitten shortcutit luodaan eri konfiguraatioille ja asemille (C ja D asemat ainakin aluksi).
$Version = "5.13.28.0"
$Folder = "MetEditor_5_13_28_0"
# Laita t‰h‰n koneessa olevan Dropbox hakemiston asemakirjain, mihin kaikki shortcutit talletetaan
$CurrentDropboxDriveLetter = "C"

# BaseFolder on perushakemisto ilman asemakirjainta
$BaseFolder = ":\smartmet"
$SmartMetFolder = "$BaseFolder\Dropbox (FMI)\SmartMet"
$SmartMetBinFolder = "$SmartMetFolder\$Folder\bin_x64"

function set-shortcut 
{
  param ( [string]$DriveLetter, [string]$ConfFileName, [string]$TitleName , [string]$LinkTitleName)

  $WshShell = New-Object -comObject WScript.Shell


# Luodaan shortcutit Dropboxin ulkopuolelle, jotta ne saadaan kopioitua kerralla lopuksi oikeaan paikkaan.
# Shortcut linkkipolkua ei saa laittaa lainausmerkkien sis‰‰n ($WshShell.CreateShortcut ei salli sellaista)!
  $SourceLinkX = $CurrentDropboxDriveLetter + $BaseFolder + "\" + $Version + " - " + $LinkTitleName + " - " + $DriveLetter + ".lnk"
  $SourceLinkX
  $Shortcut = $WshShell.CreateShortcut($SourceLinkX)

# SmartMet executablen koko polku
# Huom! Polku pit‰‰ laittaa lainausmerkkien sis‰‰n
# Huom2! $Shortcut.TargetPath arvo korjaa itsens‰, jos annetussa polussa on erilaiset kirjain koot,
# esim. alun 'smartmet' muuttuu 'SmartMet':iksi, jos juuren smartmet polku on oikeasti 'SmartMet'
  $TargetPathX = '"' + $DriveLetter + $SmartMetBinFolder + "\SmartMet.exe" + '"'
  $TargetPathX
  $Shortcut.TargetPath = $TargetPathX

# Argumentti osio pit‰‰ rakentaa osissa, muuten tulee hankaluuksia lainausmerkkien kanssa, jotka tulee polkuihin.
# Huomioi myˆs ett‰ -p ja -t argumentteihin pit‰‰ lis‰t‰ spaceja, muuten ei toimi.
  $Argument1 = "-p "
  $Argument2 = "`"$DriveLetter$SmartMetFolder\Control\$ConfFileName`""
  $Argument3 = " -t "
  $Argument4 = "`"SmartMet $Version - $TitleName`""
  $ArgumentsX = $Argument1 + $Argument2 + $Argument3 + $Argument4
  $ArgumentsX
  $Shortcut.Arguments = $ArgumentsX

# Huom! $Shortcut.WorkingDirectory:iin ei saa sitten itse laittaa lainausmerkkej‰ polkuun, ne tulevat sinne sitten itsest‰‰n.
  $WorkingDirectoryX = "$DriveLetter$SmartMetBinFolder\"
  $WorkingDirectoryX
  $Shortcut.WorkingDirectory = $WorkingDirectoryX

  $Shortcut.Save()

}

function set-shortcuts 
{
  param ( [string]$DriveLetter)

  set-shortcut $DriveLetter "smartmet_scand.conf" "Scand" "Scand"
  set-shortcut $DriveLetter "smartmet_sap_scand.conf" "ASK - Scand" "Scand - ASK"
  set-shortcut $DriveLetter "smartmet_tur_scand.conf" "TUR - Scand" "Scand - TUR"
  set-shortcut $DriveLetter "smartmet_regoff_scand.conf" "RegOff - Scand" "Scand - RegOff"
  set-shortcut $DriveLetter "smartmet_euro.conf" "Euro" "Euro"
  set-shortcut $DriveLetter "smartmet_regoff_euro.conf" "Euro - RegOff" "Euro - RegOff"
  set-shortcut $DriveLetter "smartmet_world.conf" "World" "World"
}

set-shortcuts "C"
set-shortcuts "D"

read-host ìPress ENTER to continue...î
