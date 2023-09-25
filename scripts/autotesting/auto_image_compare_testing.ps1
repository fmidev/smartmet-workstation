# This script does automation test for SmartMet generated images.
# There is *original* folder which contains proven correct images in it's subfolders.
# There is *current* folder which contains images which are just produced with Smartmet and that are wanted to be tested.
#

$OriginalDirectoryRoot = "D:\autotesting\original"
$CurrentDirectoryRoot = "D:\autotesting\current"
$DotSlash = ".\"
$DescriptionFileName = "description.txt"
$ImageFileFilter = "*.png"
$DifferenceImagesDirectoryRoot = "D:\autotesting\differenceimages"
$MagickMetric = "DSSIM"
$MagickExitCodeSimilar = 0
$MagickExitCodeDissimilar = 1
$MagickExitCodeError = 2
$MagickMetricValueGood = 0
$MagickMetricValueBad = 1

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
    $ExitCodeString = "execution error"
  }
  return $ExitCodeString
}

function MakeMagickCompareStringForDSSIM
{
  param ([double]$MagickCompareValue)

  $WarningLimitForDSSIM = 0.006
  $MagickCompareString = "[OK]"
  if($MagickCompareValue -gt $MagickMetricValueGood -And $MagickCompareValue -le $WarningLimitForDSSIM)
  {
    $MagickCompareString = "[Warning]"
  }
  elseif($MagickCompareValue -gt $WarningLimitForDSSIM)
  {
    $MagickCompareString = "[ERROR]"
  }
  return $MagickCompareString
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
  if($MagicExitCode -eq $MagickExitCodeError)
  {
    # Lat's make sure that in magick excution error the metric value is set to bad
    $MagickCompareValue = $MagickMetricValueBad
  }
  $MagickCompareString = MakeMagickCompareStringForDSSIM $MagickCompareValue
  $output = "Image: " + $ImageFileName + ", magick exit-code: " + $ExitCodeString + ", compare value (" + $MagickMetric + "): " + $MagickCompareValue + " " + $MagickCompareString
  Write-Output $output
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
# 1) $AbsoluteOriginalFilePath is absolute path of image file on the original side of check tree.
# 2) $AbsoluteCurrentFilePath is absolute path of image file on the current side of check tree.
# 3) $ImageFileName is name of both checked files (on different paths), used only to output results.
  param ([string]$AbsoluteOriginalFilePath, [string]$AbsoluteCurrentFilePath, [string]$ImageFileName)
  
  $AbsoluteDifferenceFilePath = $AbsoluteOriginalFilePath.replace($OriginalDirectoryRoot, $DifferenceImagesDirectoryRoot)

  # Mysteries of Powershell: With magick executable you need to prevent magick's own output by adding "2>&1" thingy in the end
  $MagickResult = magick compare -metric $MagickMetric $AbsoluteOriginalFilePath $AbsoluteCurrentFilePath $AbsoluteDifferenceFilePath 2>&1
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
	New-Item -ItemType Directory -Force -Path $AbsoluteDifferenceImageDirectory
  }
}

function MissingFileErrorString
{
  param ([string]$ImageFileName, [string]$ExistSide)

  $ErrorString = MakeMagickCompareStringForDSSIM $MagickMetricValueBad
  $ResultString = "Image: " + $ImageFileName + " was only in '" + $ExistSide + "' side " + $ErrorString
  return $ResultString
}

function CompareDirectoryImages
{
# Check given directory's all image files in original vs current directories.
# Check only this level, so no recursive options.
# Function has two parameters:
# 1) $AbsoluteDirectoryPath is absolute path from the original side of check tree.
# 2) $RelativeDirectoryPath is relative path in relation to the original and current root paths.
	
  param ([string]$AbsoluteDirectoryPath, [string]$RelativeDirectoryPath)

  OutputDescriptionFile $AbsoluteDirectoryPath $RelativeDirectoryPath

  # Let's get all the image files from the both testing side directories
  $FinalOriginalPath = MakeGetChildItemPathWithIncludeParam $OriginalDirectoryRoot $RelativeDirectoryPath
  $OriginalImageFiles = Get-childitem -Path $FinalOriginalPath -Include $ImageFileFilter -File
  $FinalCurrentPath = MakeGetChildItemPathWithIncludeParam $CurrentDirectoryRoot $RelativeDirectoryPath
  $CurrentImageFiles = Get-childitem -Path $FinalCurrentPath -Include $ImageFileFilter -File

  if($OriginalImageFiles -ne $null -And $CurrentImageFiles -ne $null)
  {
    $output =  "Doing image comparisons in directory: " + $RelativeDirectoryPath
    Write-Output $output
	
	$AbsoluteDifferenceImageDirectory = $DifferenceImagesDirectoryRoot + "\" + $RelativeDirectoryPath
	DifferenceImageDirectoryChecks $AbsoluteDifferenceImageDirectory

    $ImageFileComparisons = Compare-Object -ReferenceObject $OriginalImageFiles -DifferenceObject $CurrentImageFiles -IncludeEqual  -Property Name -PassThru | select Name, @{n="FullName";e={$_.fullname}}, SideIndicator

    ForEach($ImageFileCompare in $ImageFileComparisons) 
    {
      $AbsoluteOriginalFilePath = $ImageFileCompare.FullName
      $AbsoluteCurrentFilePath = $AbsoluteOriginalFilePath.replace($OriginalDirectoryRoot, $CurrentDirectoryRoot)
	  $ImageFileName = $ImageFileCompare.Name
      if ($ImageFileCompare.SideIndicator -eq "<=") 
      {
	    # Write-Output puts new-line after each separate output, you have to make one variable that contain the whole message first
        $output = MissingFileErrorString $ImageFileName "original"
        Write-Output $output
      }
      elseif ($ImageFileCompare.SideIndicator -eq "=>") 
      {
        # Write-Output puts new-line after each separate output, you have to make one variable that contain the whole message first
        $output = MissingFileErrorString $ImageFileName "current"
        Write-Output $output
      }
      else 
      {
		CompareImageFiles $AbsoluteOriginalFilePath $AbsoluteCurrentFilePath $ImageFileName
      }
    }
	 
  }
}


# Let's get recursively all the directories from the both root directories
$OriginalDirectories = Get-childitem -Recurse -Directory $OriginalDirectoryRoot
$CurrentDirectories = Get-childitem -Recurse -Directory $CurrentDirectoryRoot

if($OriginalDirectories -eq $null -Or $CurrentDirectories -eq $null)
{
  if($OriginalDirectories -eq $null)
  {
	Write-Output "OriginalDirectories on null, wrong root path given? Exiting..."
  }
  if($CurrentDirectories -eq $null)
  {
	Write-Output "CurrentDirectories on null, wrong root path given? Exiting..."
  }
  exit
}

$DirectoryComparisons = Compare-Object -ReferenceObject $OriginalDirectories -DifferenceObject $CurrentDirectories -IncludeEqual -Property Name -PassThru | select Name, @{n="FullName";e={$_.fullname}}, SideIndicator
ForEach($DirectoryCompare in $DirectoryComparisons) 
{
  $FullDirName = $DirectoryCompare.FullName
  $RelativePath = GetRelativePath $OriginalDirectoryRoot $FullDirName
  $RelativePath = RemoveDotSlashFromStart $RelativePath
  if ($DirectoryCompare.SideIndicator -eq "<=") 
  {
	# Write-Output puts new-line after each separate output, you have to make one variable that contain the whole message first
    $output = "Error: Directory " + $RelativePath + " was only in 'original' side"
    Write-Output $output
  }
  elseif ($DirectoryCompare.SideIndicator -eq "=>") 
  {
    $RelativePath = GetRelativePath $CurrentDirectoryRoot $FullDirName
    $RelativePath = RemoveDotSlashFromStart $RelativePath
	# Write-Output puts new-line after each separate output, you have to make one variable that contain the whole message first
    $output = "Error: Directory " + $RelativePath + " was only in 'current' side"
    Write-Output $output
  }
  else 
  {
	CompareDirectoryImages $FullDirName $RelativePath
  }
}

#read-host “Press ENTER to continue...”
