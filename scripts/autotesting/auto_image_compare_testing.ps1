# This script does automation test for SmartMet generated images.
# There is *reference* folder which contains proven correct images in it's subfolders.
# There is *current* folder which contains images which are just produced with Smartmet and that are wanted to be tested.
#

# These are paths to the baseline testing directory trees
$global:UsedReferenceDirectoryRoot = ""
$global:UsedCurrentDirectoryRoot = ""
$global:UsedDifferenceImagesDirectoryRoot = ""
# These are paths to the real testing directory trees
#$ReferenceDirectoryRoot = "D:\autotesting\reference"
#$CurrentDirectoryRoot = "D:\autotesting\current"
#$DifferenceImagesDirectoryRoot = "D:\autotesting\differenceimages"


$DotSlash = ".\"
$DescriptionFileName = "description.txt"
$ImageFileFilter = "*.png"
$MagickMetric = "DSSIM"
[int]$MagickExitCodeSimilar = 0
[int]$MagickExitCodeDissimilar = 1
[int]$MagickExitCodeError = 2
$MagickMetricValueGood = 0
$MagickMetricValueBad = 1
$GeneralOkString = "[OK]"
$GeneralWarningString = "[Warning]"
$GeneralErrorString = "[ERROR]"
$ComparisonSideReference = "reference"
$ComparisonSideCurrent = "current"
$ImageOkColor = "green"
$ImageWarningColor = "yellow"
$ImageErrorColor = "red"
$ImageCorruptedColor = "cyan"
$MissingFileOrDirectoryColor = "Magenta"

[int]$global:ImagesTotalCounter = 0
[int]$global:ImagesOkCounter = 0
[int]$global:ImagesWarningCounter = 0
[int]$global:ImagesErrorCounter = 0
[int]$global:ImagesCorruptedCounter = 0
[int]$global:ImagesReferenceSideOnlyCounter = 0
[int]$global:ImagesCurrentSideOnlyCounter = 0
[int]$global:DirectoriesTotalCounter = 0
[int]$global:DirectoriesReferenceSideOnlyCounter = 0
[int]$global:DirectoriesCurrentSideOnlyCounter = 0


function RemoveDotSlashFromStart
{
  # In case of relative path like ".\xxx\yyy", removes the ".\" part from the start,
  # we don't need it on Windows, and the relative path is output for log and path 
  # without the dot-slahs is more clearer to the viewer.
  param ( [string]$CheckedPath)
  if($CheckedPath.StartsWith($DotSlash))
  { 
    $CheckedPath.Remove(0, $DotSlash.Length);
  }
  else
  {
    $CheckedPath
  }
}

function GetRelativePath
{
  # Get relative path from CheckedPath against the RootPath.
  # Also wanted to remove possible .\ from the start because it's not necessary in Windows 
  # and want also to print the relative path to user as clear as possible.
  param ( [string]$RootPath, [string]$CheckedPath)

  try
  { 
    push-location $RootPath
    resolve-path -relative $CheckedPath
  } 
  finally
  { 
    pop-location 
  }
}

function Write-ColorOutput
{
	param ([string]$ForegroundColor, [string]$OutputText)
	
    # save the current color
    $OldColor = $host.UI.RawUI.ForegroundColor

    # set the new color
    $host.UI.RawUI.ForegroundColor = $ForegroundColor

    # output
    Write-Output $OutputText

    # restore the original color
    $host.UI.RawUI.ForegroundColor = $OldColor
}

function OutputDescriptionFile
{
  param ([string]$AbsoluteDirectoryPath, [string]$RelativeDirectoryPath)
  
  $AbsoluteDescriptionFilaPath = $AbsoluteDirectoryPath + "\" + $DescriptionFileName
  if(Test-Path $AbsoluteDescriptionFilaPath -PathType Leaf)
  {
	$output = "Test description for directory " + $RelativeDirectoryPath + ":"
    Write-Output $output
    Get-Content -Path $AbsoluteDescriptionFilaPath
  }
}

function MakeGetChildItemPathWithIncludeParam
{
  param ([string]$RootDirectory, [string]$RelativeDirectory)
  # Constructing used -Path parameter for Get-childitem cmdlet, rootdir + '\' + relativedir.
  # But when Get-childitem is used with -Include parameter, you have to add "\*" at the end.
  $FinalPath = $RootDirectory + "\" + $RelativeDirectory + "\*"
  return $FinalPath
}

function MakeMagickExitCodeString
{
  param ([int]$MagicExitCode)

  $ExitCodeString = "images similar"
  if($MagicExitCode -eq $MagickExitCodeDissimilar)
  {
    $ExitCodeString = "images dissimilar"
  }
  elseif($MagicExitCode -eq $MagickExitCodeError)
  {
    $ExitCodeString = "EXECUTION ERROR"
  }
  return $ExitCodeString
}

function MakeMagickCompareStringForDSSIM
{
  param ([double]$MagickCompareValue)

  $WarningLimitForDSSIM = 0.006
  $MagickCompareString = $GeneralOkString
  if($MagickCompareValue -gt $MagickMetricValueGood -And $MagickCompareValue -le $WarningLimitForDSSIM)
  {
    $MagickCompareString = $GeneralWarningString
  }
  elseif($MagickCompareValue -gt $WarningLimitForDSSIM)
  {
    $MagickCompareString = $GeneralErrorString
  }
  return $MagickCompareString
}

function GetCompareTextColor
{
  param ([int]$MagicExitCode, [string]$MagickCompareString)
  
  if($MagicExitCode -eq $MagickExitCodeError)
  {
    $global:ImagesCorruptedCounter++
    return $ImageCorruptedColor
  }
  elseif($MagickCompareString -eq $GeneralErrorString)
  {
    $global:ImagesErrorCounter++
    return $ImageErrorColor
  }
  elseif($MagicExitCode -eq $MagickExitCodeDissimilar -Or $MagickCompareString -eq $GeneralWarningString)
  {
    $global:ImagesWarningCounter++
    return $ImageWarningColor
  }
  else
  {
    $global:ImagesOkCounter++
    return $ImageOkColor
  }
}

function ReportCompareResults
{
# Do one line report of comparison results: image-name, exit-code-string, metric-type, metric-value, ok/warning/error
# If given exit-code means error, then metric-value will be changed to error also.
# Function has three parameters:
# 1) $ImageFileName image file's name without path.
# 2) $MagicExitCode is integer value returned by magick executable doing compare (0=similar, 1=dissimilar, 2=error in execution).
# 3) $MagickCompareValue is double value which magick compare returned using wanted metric to do the comparison.
  param ([string]$ImageFileName, [int]$MagicExitCode, [double]$MagickCompareValue)

  $ExitCodeString = MakeMagickExitCodeString $MagicExitCode
  $MagickCompareString = MakeMagickCompareStringForDSSIM $MagickCompareValue
  $FontColor = GetCompareTextColor $MagicExitCode $MagickCompareString

  if($MagicExitCode -eq $MagickExitCodeError)
  {
    # This is when in magick execution an unexpected error has occurred
    $output = "Image: " + $ImageFileName + ", magick exit-code: " + $ExitCodeString + ", compare value (" + $MagickMetric + "): --- " + $GeneralErrorString
    Write-ColorOutput $FontColor $output
  }
  else
  {
    $output = "Image: " + $ImageFileName + ", magick exit-code: " + $ExitCodeString + ", compare value (" + $MagickMetric + "): " + $MagickCompareValue + " " + $MagickCompareString
    Write-ColorOutput $FontColor $output
  }
}

function TryConvertStringToDouble
{
  # Without this TryParse usage, if image-reading failed with magick, then 
  # then $PossibleValueString parameter contains error information, and then 
  # normal conversion throws exception, and that creates ugly looking error 
  # messages, which we are avoiding here..
  param ([string]$PossibleValueString, [double]$DefaultDoubleValue)

  $ParsedDouble = $null
  $success = [Double]::TryParse($PossibleValueString, [ref]$ParsedDouble)

  if ($success) 
  {
    return $ParsedDouble
  }
  else
  {
    return $DefaultDoubleValue
  }
}

function CompareImageFiles
{
# Both same name image files exist on both comparison directory trees and 
# now they need to be compared with magicks system.
# Function has three parameters:
# 1) $AbsoluteReferenceFilePath is absolute path of image file on the reference side of check tree.
# 2) $AbsoluteCurrentFilePath is absolute path of image file on the current side of check tree.
# 3) $ImageFileName is name of both checked files (on different paths), used only to output results.
  param ([string]$AbsoluteReferenceFilePath, [string]$AbsoluteCurrentFilePath, [string]$ImageFileName)
  
  $AbsoluteDifferenceFilePath = $AbsoluteReferenceFilePath.replace($global:UsedReferenceDirectoryRoot, $global:UsedDifferenceImagesDirectoryRoot)

  # Mysteries of Powershell: With magick executable you need to prevent magick's own output by adding "2>&1" thingy in the end
  $MagickResult = magick compare -metric $MagickMetric $AbsoluteReferenceFilePath $AbsoluteCurrentFilePath $AbsoluteDifferenceFilePath 2>&1
  [int]$MagicExitCode = $LASTEXITCODE
  [double]$MagickCompareValue = TryConvertStringToDouble $MagickResult.ToString() 1

  ReportCompareResults $ImageFileName $MagicExitCode $MagickCompareValue
}

function DifferenceImageDirectoryChecks
{
  # magick produces during image compare process a difference image files.
  # This function makes sure of following things:
  # 1) Make sure that given directory exists or crate it for these difference images, 
  #    magick won't lift a finger about creating directories.
  # 2) If directory exists, make sure it's empty, this way previous run's images won't be left there by accident.
  param ([string]$AbsoluteDifferenceImageDirectory)
  if(Test-Path -PathType Container -Path $AbsoluteDifferenceImageDirectory)
  {
    # Directory existed, now delete possible files in it.
	$AbsoluteDifferenceImageDirectoryWithIncludeParam = MakeGetChildItemPathWithIncludeParam $AbsoluteDifferenceImageDirectory
	Get-ChildItem -Path $AbsoluteDifferenceImageDirectoryWithIncludeParam -Include * -File | foreach { $_.Delete()}
  }
  else
  {
    # Directory didn't exist, create it.
	New-Item -ItemType Directory -Force -Path $AbsoluteDifferenceImageDirectory >$null
  }
}

function MissingFileErrorString
{
  param ([string]$ImageFileName, [string]$ExistSide)

  $ResultString = "Image: " + $ImageFileName + " was only in '" + $ExistSide + "' side " + $GeneralErrorString
  return $ResultString
}

function MissingDirectoryErrorString
{
  param ([string]$DirectoryName, [string]$ExistSide)

  $ResultString = "Directory: " + $DirectoryName + " was only in '" + $ExistSide + "' side " + $GeneralErrorString
  return $ResultString
}

function CompareDirectoryImages
{
# Check given directory's all image files in reference vs current directories.
# Check only this level, so no recursive options.
# Function has two parameters:
# 1) $AbsoluteDirectoryPath is absolute path from the reference side of check tree.
# 2) $RelativeDirectoryPath is relative path in relation to the reference and current root paths.
	
  param ([string]$AbsoluteDirectoryPath, [string]$RelativeDirectoryPath)

  OutputDescriptionFile $AbsoluteDirectoryPath $RelativeDirectoryPath

  # Let's get all the image files from the both testing side directories
  $FinalReferencePath = MakeGetChildItemPathWithIncludeParam $global:UsedReferenceDirectoryRoot $RelativeDirectoryPath
  $ReferenceImageFiles = Get-childitem -Path $FinalReferencePath -Include $ImageFileFilter -File
  $FinalCurrentPath = MakeGetChildItemPathWithIncludeParam $global:UsedCurrentDirectoryRoot $RelativeDirectoryPath
  $CurrentImageFiles = Get-childitem -Path $FinalCurrentPath -Include $ImageFileFilter -File

  if($ReferenceImageFiles -ne $null -And $CurrentImageFiles -ne $null)
  {
    $global:DirectoriesTotalCounter++
    $output =  "Doing image comparisons in directory: " + $RelativeDirectoryPath
    Write-Output $output
	
	$AbsoluteDifferenceImageDirectory = $global:UsedDifferenceImagesDirectoryRoot + "\" + $RelativeDirectoryPath
	DifferenceImageDirectoryChecks $AbsoluteDifferenceImageDirectory

    $ImageFileComparisons = Compare-Object -ReferenceObject $ReferenceImageFiles -DifferenceObject $CurrentImageFiles -IncludeEqual  -Property Name -PassThru | select Name, @{n="FullName";e={$_.fullname}}, SideIndicator

    ForEach($ImageFileCompare in $ImageFileComparisons) 
    {
      $global:ImagesTotalCounter++
      $AbsoluteReferenceFilePath = $ImageFileCompare.FullName
      $AbsoluteCurrentFilePath = $AbsoluteReferenceFilePath.replace($global:UsedReferenceDirectoryRoot, $global:UsedCurrentDirectoryRoot)
	  $ImageFileName = $ImageFileCompare.Name
      if ($ImageFileCompare.SideIndicator -eq "<=") 
      {
        $global:ImagesReferenceSideOnlyCounter++
        Write-ColorOutput $MissingFileOrDirectoryColor $(MissingFileErrorString $ImageFileName $ComparisonSideReference)
      }
      elseif ($ImageFileCompare.SideIndicator -eq "=>") 
      {
        $global:ImagesCurrentSideOnlyCounter++
        Write-ColorOutput $MissingFileOrDirectoryColor $(MissingFileErrorString $ImageFileName $ComparisonSideCurrent)
      }
      else 
      {
		CompareImageFiles $AbsoluteReferenceFilePath $AbsoluteCurrentFilePath $ImageFileName
      }
    }
	 
  }
}

function DoFinalCounterSummary
{
	Write-Output ""
    Write-ColorOutput "white" "--------------- Final summary -------------------------"
	$OkImagesSummary = "Images ok: " + $ImagesOkCounter + "/" + $ImagesTotalCounter
    Write-ColorOutput $ImageOkColor $OkImagesSummary
	if($ImagesWarningCounter -gt 0)
	{
	  $WarningImagesSummary = "Images warning: " + $ImagesWarningCounter + "/" + $ImagesTotalCounter
      Write-ColorOutput $ImageWarningColor $WarningImagesSummary
	}
	if($ImagesErrorCounter -gt 0)
	{
	  $ErrorImagesSummary = "Images error: " + $ImagesErrorCounter + "/" + $ImagesTotalCounter
      Write-ColorOutput $ImageErrorColor $ErrorImagesSummary
	}
	if($ImagesCorruptedCounter -gt 0)
	{
	  $CorruptedImagesSummary = "Images corrupted: " + $ImagesCorruptedCounter + "/" + $ImagesTotalCounter
      Write-ColorOutput $ImageCorruptedColor $CorruptedImagesSummary
	}
	if($ImagesReferenceSideOnlyCounter -gt 0)
	{
	  $ReferenceSideOnlyImagesSummary = "Images only on reference side: " + $ImagesReferenceSideOnlyCounter + "/" + $ImagesTotalCounter
      Write-ColorOutput $MissingFileOrDirectoryColor $ReferenceSideOnlyImagesSummary
	}
	if($ImagesCurrentSideOnlyCounter -gt 0)
	{
	  $CurrentSideOnlyImagesSummary = "Images only on current side: " + $ImagesCurrentSideOnlyCounter + "/" + $ImagesTotalCounter
      Write-ColorOutput $MissingFileOrDirectoryColor $CurrentSideOnlyImagesSummary
	}

	$CheckedDirectorySummary = "Directories checked: " + $DirectoriesTotalCounter
    Write-ColorOutput "white" $CheckedDirectorySummary
	if($DirectoriesReferenceSideOnlyCounter -gt 0)
	{
	  $ReferenceSideOnlyDirectorySummary = "Directories only on reference side: " + $DirectoriesReferenceSideOnlyCounter + "/" + $DirectoriesTotalCounter
      Write-ColorOutput $MissingFileOrDirectoryColor $ReferenceSideOnlyDirectorySummary
	}
	if($DirectoriesCurrentSideOnlyCounter -gt 0)
	{
	  $CurrentSideOnlyDirectorySummary = "Directories only on current side: " + $DirectoriesCurrentSideOnlyCounter + "/" + $DirectoriesTotalCounter
      Write-ColorOutput $MissingFileOrDirectoryColor $CurrentSideOnlyDirectorySummary
	}
}

function DoImageTestingForGivenDirectories
{
  # 1) ReferenceDir: Directory where reference images reside
  # 2) CurrentDir: Directory where now tested images reside
  # 3) DifferenceDir: Directory where the magick difference images are generated
  param ([string]$ReferenceDir, [string]$CurrentDir, [string]$DifferenceDir)

  $global:UsedReferenceDirectoryRoot = $ReferenceDir
  $global:UsedCurrentDirectoryRoot = $CurrentDir
  $global:UsedDifferenceImagesDirectoryRoot = $DifferenceDir
  
  # Let's get recursively all the directories from the both root directories
  $ReferenceDirectories = Get-childitem -Recurse -Directory $ReferenceDir
  $CurrentDirectories = Get-childitem -Recurse -Directory $CurrentDir

  if($ReferenceDirectories -eq $null -Or $CurrentDirectories -eq $null)
  {
    if($ReferenceDirectories -eq $null)
    {
    Write-Output "ReferenceDirectories was null, wrong root path given? Exiting..."
    }
    if($CurrentDirectories -eq $null)
    {
	  Write-Output "CurrentDirectories was null, wrong root path given? Exiting..."
    }
    return
  }

  $DirectoryComparisons = Compare-Object -ReferenceObject $ReferenceDirectories -DifferenceObject $CurrentDirectories -IncludeEqual -Property Name -PassThru | select Name, @{n="FullName";e={$_.fullname}}, SideIndicator
  ForEach($DirectoryCompare in $DirectoryComparisons) 
  {
    $FullDirName = $DirectoryCompare.FullName
    $RelativePath = GetRelativePath $global:UsedReferenceDirectoryRoot $FullDirName
    $RelativePath = RemoveDotSlashFromStart $RelativePath
    if ($DirectoryCompare.SideIndicator -eq "<=") 
    {
      $global:DirectoriesTotalCounter++
      $global:DirectoriesReferenceSideOnlyCounter++
      Write-ColorOutput $MissingFileOrDirectoryColor $(MissingDirectoryErrorString $RelativePath $ComparisonSideReference)
    }
    elseif ($DirectoryCompare.SideIndicator -eq "=>") 
    {
      $global:DirectoriesTotalCounter++
      $global:DirectoriesCurrentSideOnlyCounter++
      $RelativePath = GetRelativePath $global:UsedCurrentDirectoryRoot $FullDirName
      $RelativePath = RemoveDotSlashFromStart $RelativePath
      Write-ColorOutput $MissingFileOrDirectoryColor $(MissingDirectoryErrorString $RelativePath $ComparisonSideCurrent)
    }
    else 
    {
	  CompareDirectoryImages $FullDirName $RelativePath
    }
  }
}


DoImageTestingForGivenDirectories "D:\autotesting\baselinetest\reference" "D:\autotesting\baselinetest\current" "D:\autotesting\baselinetest\differenceimages"
DoImageTestingForGivenDirectories "D:\autotesting\reference" "D:\autotesting\current" "D:\autotesting\differenceimages"

DoFinalCounterSummary

#read-host “Press ENTER to continue...”
