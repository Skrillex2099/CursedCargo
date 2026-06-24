@echo off
setlocal

set "PROJECT=%~dp0CursedCargo.uproject"
set "BUILD_TOOL=C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"

if not exist "%BUILD_TOOL%" (
    echo Unreal Engine 5.8 build tool was not found.
    echo Expected: %BUILD_TOOL%
    pause
    exit /b 1
)

call "%BUILD_TOOL%" CursedCargoEditor Win64 Development "%PROJECT%" -WaitMutex -NoHotReloadFromIDE
set "RESULT=%ERRORLEVEL%"

if not "%RESULT%"=="0" (
    echo.
    echo Build failed with exit code %RESULT%.
    pause
    exit /b %RESULT%
)

echo.
echo CursedCargoEditor built successfully.
pause
exit /b 0

