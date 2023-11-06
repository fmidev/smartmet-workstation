#Create SmartMet shortcuts
#Just set the desired version number and folder
$Version = "5.13.14.0"
$Folder = "MetEditor_5_13_14_0"

#Scand - D
$WshShell = New-Object -comObject WScript.Shell
#$Shortcut = $WshShell.CreateShortcut($env:USERPROFILE + "\Desktop\$Version - Scand.lnk")
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - Scand - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_scand.conf"" -t ""SmartMet $Version - Scand"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#Scand - C
#$WshShell = New-Object -comObject WScript.Shell
#$Shortcut = $WshShell.CreateShortcut("C:\smartmet\$Version - Scand - C.lnk")
#$Shortcut.TargetPath = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
#$Shortcut.Arguments = "-p ""C:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_scand.conf"" -t ""SmartMet $Version - Scand"""
#$Shortcut.WorkingDirectory = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
#$Shortcut.Save()

#Scand - ASK - D
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - Scand - ASK - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_sap_scand.conf"" -t ""SmartMet $Version - ASK - Scand"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#Scand - ASK - C
#$WshShell = New-Object -comObject WScript.Shell
#$Shortcut = $WshShell.CreateShortcut("C:\smartmet\$Version - Scand - ASK - C.lnk")
#$Shortcut.TargetPath = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
#$Shortcut.Arguments = "-p ""C:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_sap_scand.conf"" -t ""SmartMet $Version - ASK - Scand"""
#$Shortcut.WorkingDirectory = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
#$Shortcut.Save()

#Scand - TUR - D
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - Scand - TUR - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_tur_scand.conf"" -t ""SmartMet $Version - TUR - Scand"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#Scand - TUR - C
# $WshShell = New-Object -comObject WScript.Shell
# $Shortcut = $WshShell.CreateShortcut("C:\smartmet\$Version - Scand - TUR - C.lnk")
# $Shortcut.TargetPath = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
# $Shortcut.Arguments = "-p ""C:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_tur_scand.conf"" -t ""SmartMet $Version - TUR - Scand"""
# $Shortcut.WorkingDirectory = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
# $Shortcut.Save()

#Scand - RegOff - D
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - Scand - RegOff - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_regoff_scand.conf"" -t ""SmartMet $Version - RegOff - Scand"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#Euro - D
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - Euro - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_euro.conf"" -t ""SmartMet $Version - Euro"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#Euro - C
#$WshShell = New-Object -comObject WScript.Shell
#$Shortcut = $WshShell.CreateShortcut("C:\smartmet\$Version - Euro - C.lnk")
#$Shortcut.TargetPath = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
#$Shortcut.Arguments = "-p ""C:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_euro.conf"" -t ""SmartMet $Version - Euro"""
#$Shortcut.WorkingDirectory = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
#$Shortcut.Save()

#Euro - RegOff - D
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - Euro - RegOff - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_regoff_euro.conf"" -t ""SmartMet $Version - RegOff - Euro"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#World - D
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("D:\smartmet\$Version - World - D.lnk")
$Shortcut.TargetPath = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
$Shortcut.Arguments = "-p ""D:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_world.conf"" -t ""SmartMet $Version - World"""
$Shortcut.WorkingDirectory = "D:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
$Shortcut.Save()

#World - C
#$WshShell = New-Object -comObject WScript.Shell
#$Shortcut = $WshShell.CreateShortcut("C:\smartmet\$Version - World - C.lnk")
#$Shortcut.TargetPath = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\SmartMet.exe"
#$Shortcut.Arguments = "-p ""C:\smartmet\Dropbox (FMI)\SmartMet\Control\smartmet_world.conf"" -t ""SmartMet $Version - World"""
#$Shortcut.WorkingDirectory = "C:\smartmet\Dropbox (FMI)\SmartMet\$Folder\bin_x64\"
#$Shortcut.Save()