@echo off
setlocal enabledelayedexpansion

:: KeyCast Build Script
:: Usage: build.bat [--clean] [--debug] [--release]

set BUILD_TYPE=Release
set DO_CLEAN=0

:: Parse arguments
:parse_args
if "%~1"=="" goto done_parsing
if /i "%~1"=="--clean" set DO_CLEAN=1
if /i "%~1"=="--debug" set BUILD_TYPE=Debug
if /i "%~1"=="--release" set BUILD_TYPE=Release
shift
goto parse_args
:done_parsing

:: Setup MinGW environment
set PATH=C:\msys64\mingw64\bin;%PATH%
set CMAKE_PREFIX_PATH=C:\msys64\mingw64

echo ========================================
echo KeyCast Build Script
echo Build Type: %BUILD_TYPE%
echo ========================================

:: Clean if requested
if %DO_CLEAN%==1 (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
    if exist dist\KeyCast.exe del /q dist\KeyCast.exe
    echo Clean complete.
)

:: Create build directory
if not exist build mkdir build
cd build

:: Run CMake configure
echo.
echo Configuring with CMake...
cmake -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH=C:\msys64\mingw64 ^
    ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

:: Build
echo.
echo Building KeyCast...
cmake --build . --config %BUILD_TYPE% -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    cd ..
    exit /b 1
)

cd ..

:: Copy Qt DLLs to dist
echo.
echo Copying runtime DLLs...

:: Qt DLLs
copy /y "C:\msys64\mingw64\bin\Qt6Core.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\Qt6Gui.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\Qt6Widgets.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\Qt6Network.dll" dist\ >nul 2>&1

:: MinGW runtime DLLs
copy /y "C:\msys64\mingw64\bin\libgcc_s_seh-1.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libstdc++-6.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libwinpthread-1.dll" dist\ >nul 2>&1

:: Additional Qt dependencies
copy /y "C:\msys64\mingw64\bin\libicuin*.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libicuuc*.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libicudt*.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libpcre2-16-0.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libpng16-16.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libharfbuzz-0.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libfreetype-6.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\zlib1.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libbz2-1.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libbrotlidec.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libbrotlicommon.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libglib-2.0-0.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libintl-8.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libiconv-2.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libgraphite2.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libdouble-conversion.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libzstd.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libmd4c.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libpcre2-8-0.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libb2-1.dll" dist\ >nul 2>&1

:: SSL libraries for network
copy /y "C:\msys64\mingw64\bin\libssl-3-x64.dll" dist\ >nul 2>&1
copy /y "C:\msys64\mingw64\bin\libcrypto-3-x64.dll" dist\ >nul 2>&1

:: Qt plugins
if not exist dist\platforms mkdir dist\platforms
copy /y "C:\msys64\mingw64\share\qt6\plugins\platforms\qwindows.dll" dist\platforms\ >nul 2>&1

if not exist dist\styles mkdir dist\styles
copy /y "C:\msys64\mingw64\share\qt6\plugins\styles\qwindowsvistastyle.dll" dist\styles\ >nul 2>&1

if not exist dist\tls mkdir dist\tls
copy /y "C:\msys64\mingw64\share\qt6\plugins\tls\*.dll" dist\tls\ >nul 2>&1

echo.
echo ========================================
echo Build complete!
echo Output: dist\KeyCast.exe
echo ========================================
