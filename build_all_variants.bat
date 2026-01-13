@echo off
REM Build all hardware variants for AT-USB LoRa Dongle
REM This script builds all 8 variants (4 HW configs x 2 build types)

setlocal

echo ========================================
echo Building ALL Hardware Variants
echo ========================================
echo.

set VARIANTS=868_XTAL 868_TCXO 915_XTAL 915_TCXO
set BUILD_TYPES=Debug Release

set SUCCESS_COUNT=0
set FAIL_COUNT=0
set FAILED_BUILDS=

for %%V in (%VARIANTS%) do (
    for %%B in (%BUILD_TYPES%) do (
        echo.
        echo ========================================
        echo Building: %%V - %%B
        echo ========================================
        call build_hw_variant.bat %%V %%B
        if errorlevel 1 (
            set /a FAIL_COUNT+=1
            set "FAILED_BUILDS=!FAILED_BUILDS! %%V-%%B"
        ) else (
            set /a SUCCESS_COUNT+=1
        )
    )
)

echo.
echo ========================================
echo Build Summary
echo ========================================
echo Successful builds: %SUCCESS_COUNT%
echo Failed builds: %FAIL_COUNT%
if not "%FAILED_BUILDS%"=="" (
    echo Failed: %FAILED_BUILDS%
)
echo ========================================

if %FAIL_COUNT% GTR 0 (
    exit /b 1
)

endlocal
