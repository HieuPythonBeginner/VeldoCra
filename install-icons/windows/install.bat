@echo off
REM ============================================
REM   VeldoCra File Association Installer
REM   For Windows
REM ============================================

setlocal EnableDelayedExpansion

echo ============================================
echo   VeldoCra File Association Installer
echo   For Windows
echo ============================================
echo.

REM Check for admin privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] This script may require Administrator privileges.
    echo Press any key to continue anyway...
    pause >nul
)

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM Go to project root
cd /d "%SCRIPT_DIR\.."

REM Create VeldoCra app data directory
if not exist "%LOCALAPPDATA%\VeldoCra" mkdir "%LOCALAPPDATA%\VeldoCra"
if not exist "%LOCALAPPDATA%\VeldoCra\icons" mkdir "%LOCALAPPDATA%\VeldoCra\icons"

echo [1/4] Installing VeldoCra icon...
REM Copy icon file if exists (try .ico first, then .png)
if exist "icons\veldora.ico" (
    copy /Y "icons\veldora.ico" "%LOCALAPPDATA%\VeldoCra\icons\" >nul
    echo       - ICO icon installed to %LOCALAPPDATA%\VeldoCra\icons\
) else if exist "VeldAni.png" (
    copy /Y "VeldAni.png" "%LOCALAPPDATA%\VeldoCra\icons\" >nul
    echo       - PNG icon installed to %LOCALAPPDATA%\VeldoCra\icons\
) else (
    echo       - Warning: No icon found!
)

echo [2/4] Installing VeldoCra compiler...
REM Copy compiler if exists
if exist "build\bin\velc.exe" (
    copy /Y "build\bin\velc.exe" "%LOCALAPPDATA%\VeldoCra\" >nul
    echo       - Compiler installed
) else (
    echo       - Warning: velc.exe not found. Please build first with: cmake .. && make
)

echo [3/4] Registering .vel file association...
REM Create registry file
(
    echo Windows Registry Editor Version 5.00
    echo.
    echo [HKEY_CLASSES_ROOT\.vel]
    echo @="VeldoCraSourceFile"
    echo "Content Type"="text/x-vel"
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile]
    echo @="VeldoCra Source File"
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile\DefaultIcon]
    echo @="%LOCALAPPDATA%\\VeldoCra\\icons\\veldora.ico"
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell]
    echo @="open"
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell\open]
    echo @="Open with VeldoCra"
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell\open\command]
    echo @="\"%LOCALAPPDATA%\\VeldoCra\\velc.exe\" \"%%1\""
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell\run]
    echo @="Run with VeldoCra VM"
    echo.
    echo [HKEY_CLASSES_ROOT\VeldoCraSourceFile\shell\run\command]
    echo @="\"%LOCALAPPDATA%\\VeldoCra\\velc.exe\" run \"%%1\""
) > "%TEMP%\veldocra.reg"

REM Import registry
reg import "%TEMP%\veldocra.reg" >nul 2>&1
del "%TEMP%\veldocra.reg"
echo       - Registry updated

echo [4/4] Refreshing Windows shell...
REM Refresh shell
ie4uinit.exe -show >nul 2>&1

echo.
echo ============================================
echo   Installation Complete!
echo ============================================
echo.
echo [Note] Please restart Explorer or log out/in to see changes.
echo.
echo To test, create a test.vel file and double-click it.
echo.
pause

