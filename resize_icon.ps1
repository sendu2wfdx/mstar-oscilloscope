Add-Type -AssemblyName System.Drawing
$imgPath = Join-Path $PSScriptRoot "image\MScollect.png"
$iconPath = Join-Path $PSScriptRoot "image\MScollect_icon.png"
$img = [System.Drawing.Image]::FromFile($imgPath)
$iconSize = 64
$bmp = New-Object System.Drawing.Bitmap($iconSize, $iconSize)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$g.DrawImage($img, 0, 0, $iconSize, $iconSize)
$bmp.Save($iconPath, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose()
$bmp.Dispose()
$img.Dispose()
Write-Host "Icon created: $iconPath (${iconSize}x${iconSize})"
