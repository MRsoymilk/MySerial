@echo off
setlocal

tasklist /FI "IMAGENAME eq MySerial.exe" /NH | find /I "MySerial.exe" >nul
if errorlevel 1 (
    rem MySerial.exe not running
    exit /b 0
) else (
    rem Try to kill
    taskkill /F /IM MySerial.exe >nul 2>&1
    timeout /t 1 /nobreak >nul
    tasklist /FI "IMAGENAME eq MySerial.exe" /NH | find /I "MySerial.exe" >nul
    if errorlevel 1 (
        rem Killed successfully
        exit /b 0
    ) else (
        rem Failed to kill
        exit /b 1
    )
)