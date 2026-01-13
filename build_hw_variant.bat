@echo off
REM Build script for AT-USB LoRa Dongle - Hardware Variant Builder
REM Usage: build_hw_variant.bat [variant] [build_type]
REM   variant: 868_XTAL, 868_TCXO, 915_XTAL, 915_TCXO (default: 915_TCXO)
REM   build_type: Debug, Release (default: Debug)

setlocal

REM Set defaults
set HW_VARIANT=%1
set BUILD_TYPE=%2

if "%HW_VARIANT%"=="" set HW_VARIANT=915_TCXO
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Debug

echo ========================================
echo Building AT-USB LoRa Dongle
echo Hardware Variant: %HW_VARIANT%
echo Build Type: %BUILD_TYPE%
echo ========================================
echo.

REM Convert to lowercase preset name
set PRESET_NAME=%HW_VARIANT%_%BUILD_TYPE%
set PRESET_NAME=%PRESET_NAME:_XTAL=-xtal%
set PRESET_NAME=%PRESET_NAME:_TCXO=-tcxo%
set PRESET_NAME=%PRESET_NAME:_Debug=-debug%
set PRESET_NAME=%PRESET_NAME:_Release=-release%

for %%L IN (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) DO SET PRESET_NAME=!PRESET_NAME:%%L=%%L!
setlocal EnableDelayedExpansion
for %%i in ("A=a" "B=b" "C=c" "D=d" "E=e" "F=f" "G=g" "H=h" "I=i" "J=j" "K=k" "L=l" "M=m" "N=n" "O=o" "P=p" "Q=q" "R=r" "S=s" "T=t" "U=u" "V=v" "W=w" "X=x" "Y=y" "Z=z") do (
    set "PRESET_NAME=!PRESET_NAME:%%~i!"
)

echo Using CMake preset: !PRESET_NAME!
echo.

REM Configure
echo Configuring...
cmake --preset !PRESET_NAME!
if errorlevel 1 (
    echo.
    echo ERROR: Configuration failed!
    exit /b 1
)

echo.
echo Building...
cmake --build build/!PRESET_NAME! -j
if errorlevel 1 (
    echo.
    echo ERROR: Build failed!
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo Output: build/!PRESET_NAME!/CmakeTest.elf
echo ========================================

endlocal
