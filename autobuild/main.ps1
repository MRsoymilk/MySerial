param (
    [string]$Note = "无更新说明"
)

./auto_build/MySerial/svn-update.ps1
./auto_build/MySerial/build.ps1

# 1. 从 version.h 提取版本号
$versionHeader = "D:\work\project\my-serial\build\Desktop_Qt_6_9_1_MSVC2022_64bit-Release\version.h"
$major = Select-String -Path $versionHeader -Pattern 'APP_VERSION_MAJOR\s+(\d+)' | ForEach-Object { $_.Matches[0].Groups[1].Value }
$minor = Select-String -Path $versionHeader -Pattern 'APP_VERSION_MINOR\s+(\d+)' | ForEach-Object { $_.Matches[0].Groups[1].Value }
$patch = Select-String -Path $versionHeader -Pattern 'APP_VERSION_PATCH\s+(\d+)' | ForEach-Object { $_.Matches[0].Groups[1].Value }
$version = "$major.$minor.$patch"

# 时间戳
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# 路径配置
$exeSource = "D:\work\project\my-serial\build\Desktop_Qt_6_9_1_MSVC2022_64bit-Release\MySerial.exe"
$latestDir = "D:\work\output\MySerial\latest"
$outputDir = "D:\work\output\MySerial"
$updateFile = Join-Path $outputDir "update.txt"
$updateJsonFile = Join-Path $outputDir "update.json"
$zipFileName = "MySerial_${timestamp}.zip"
$zipFilePath = Join-Path $outputDir $zipFileName
$downloadUrl = "http://192.168.123.233:8000"

# 创建 latest 目录
if (-Not (Test-Path $latestDir)) {
    New-Item -Path $latestDir -ItemType Directory | Out-Null
}

# 复制 EXE
Write-Host ">> 正在复制 MySerial.exe 到 latest 文件夹..."
Copy-Item -Path $exeSource -Destination $latestDir -Force

# 压缩 latest
# Write-Host ">> 正在压缩为 $zipFileName..."
# Compress-Archive -Path "$latestDir\*" -DestinationPath $zipFilePath -Force

# NSIS
$makensis = "D:\software\NSIS\makensis.exe"
$nsisScript = "D:\work\output\auto_build\MySerial\nsis_MySerial.nsi"

if (Test-Path $makensis) {
    # 原版
    Write-Host ">> 正在打包安装程序 MySerial_Setup_${timestamp}.exe"
    & $makensis `
    /DAPP_NAME=MySerial `
    /DAPP_ARGS="" `
    /DVERSION=$version `
    /DTIMESTAMP=$timestamp `
    /DOUTDIR=$outputDir `
    $nsisScript

    # C版
    Write-Host ">> 正在打包安装程序 C_MySerial_Setup_${timestamp}.exe"
    & $makensis `
    /DAPP_NAME=C_MySerial `
    /DAPP_ARGS="--cli --mode easy --algorithm F30_Curves" `
    /DVERSION=$version `
    /DTIMESTAMP=$timestamp `
    /DOUTDIR=$outputDir `
    $nsisScript

    # P版
    Write-Host ">> 正在打包安装程序 P_MySerial_Setup_${timestamp}.exe"
    & $makensis `
    /DAPP_NAME=P_MySerial `
    /DAPP_ARGS="--cli --mode produce --algorithm F30_Curves" `
    /DVERSION=$version `
    /DTIMESTAMP=$timestamp `
    /DOUTDIR=$outputDir `
    $nsisScript
} else {
    Write-Warning "⚠️ 找不到 makensis.exe，请确认 NSIS 已正确安装。"
}
$installerName = "MySerial_${version}_${timestamp}.exe"

# 写入 update.txt
Write-Host ">> 写入更新说明到 update.txt..."
Add-Content -Path $updateFile -Value "`n时间戳：$timestamp"
Add-Content -Path $updateFile -Value "版本号：$version"
Add-Content -Path $updateFile -Value "更新：$Note"

# 写入 update.json
Write-Host ">> 写入更新信息到 update.json..."
$jsonObj = @{
    version     = $version
    timestamp   = $timestamp
    description = $Note
    file        = $installerName
    url         = $downloadUrl
}

$jsonText = $jsonObj | ConvertTo-Json -Depth 2
Set-Content -Path $updateJsonFile -Value $jsonText -Encoding UTF8

Write-Host "✅ 打包完成：   $installerName"
Write-Host "📝 更新说明：   $updateFile"
Write-Host "📦 更新 JSON：  $updateJsonFile"
