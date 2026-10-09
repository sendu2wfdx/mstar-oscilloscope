Add-Type -AssemblyName System.Drawing
$imgPath = Join-Path $PSScriptRoot "image\logo.png"
$icoPath = Join-Path $PSScriptRoot "image\logo.ico"
$img = [System.Drawing.Image]::FromFile($imgPath)
$iconSize = 64
$ms = New-Object System.IO.MemoryStream
$img.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
$pngBytes = $ms.ToArray()
$fs = [System.IO.File]::Create($icoPath)
$bw = New-Object System.IO.BinaryWriter($fs)
# ICONDIR
$bw.Write([Int16]0)
$bw.Write([Int16]1)
$bw.Write([Int16]1)
# ICONDIRENTRY
$bw.Write([byte]$iconSize)
$bw.Write([byte]$iconSize)
$bw.Write([byte]0)
$bw.Write([byte]0)
$bw.Write([Int16]1)
$bw.Write([Int16]32)
$bw.Write([Int32]$pngBytes.Length)
$bw.Write([Int32]22)
# Image data
$bw.Write($pngBytes)
$bw.Close()
$fs.Close()
$img.Dispose()
Write-Host "ICO created: $icoPath"
