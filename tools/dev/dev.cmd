@echo off
rem Build hg-rc, copy it into the test copy of the game, and start the game.
rem Double-click this file (or run it from any prompt). Each step writes its own
rem log into buildlogs\ in the repo root so the output can be read afterwards.
rem
rem   dev.cmd            shows a menu and asks what to run
rem   dev.cmd hgvk       build, deploy, run with the Vulkan plugin (HGVK)
rem   dev.cmd dx5        build, deploy, run with the original Direct3D plugin
rem   dev.cmd dx5noconv  as dx5, but with all converted code off (noconv)
rem   dev.cmd ref12      the untouched 1.2 game (C:\Coding\HeavyGear\Ref12: 1.2's own DLLs and plugins, our
rem                      launcher with noconv and 1.2's own "no CD" setting), mission M01_SCN1, Direct3D
rem   dev.cmd build      build and deploy only, do not start the game
rem   dev.cmd direct     Vulkan plugin, skipping the menus and the Windows 256-colour
rem                      compatibility layer: starts mission M01_SCN1 straight away
rem   dev.cmd directnoz  as direct, but with the Vulkan depth buffer off (nozbuf)
rem   dev.cmd directorig as direct, but with the original detail and view distance
rem   dev.cmd directnolod      as direct, but without forcing the finest model LOD
rem   dev.cmd directnosubpixel as direct, but with whole-pixel polygon corners
rem   dev.cmd directsdl        as direct, but SDL3 runs input and sound (sdlinput sdlaudio)
rem   dev.cmd directnoconv     as direct, but with the original (unconverted) memory cache code
rem   dev.cmd menusdl          through the menus, with SDL3 for the menu screen, input and sound
rem   dev.cmd directnohud      as direct, but with the original 640x480 HUD (nohudhi)
rem   dev.cmd watch09          as direct, but mission M09_SCN1 (the Behemoth starts next to you) with the stack
rem                            watchers on (uninittrace); watch06 does the same with M06_SCN1
rem   dev.cmd watch09noconv    as watch09, but with the original (unconverted) code (uninittrace noconv);
rem                            watch06noconv does the same with M06_SCN1
rem A second argument picks the mission of the direct modes, e.g. "dev.cmd direct M06_SCN1" (default M01_SCN1).

setlocal
set "REPO=%~dp0..\.."
set "GAME=C:\Coding\HeavyGear\Project"
set "REF12=C:\Coding\HeavyGear\Ref12"
set "MODE=%~1"
set "MISSION=%~2"
if "%MISSION%"=="" set "MISSION=M01_SCN1"
if "%MODE%"=="" (
    echo.
    echo   1  Vulkan, through the menus
    echo   2  Vulkan, mission M01_SCN1 straight away - no menus, no 256-colour layer
    echo   3  Original Direct3D renderer
    echo   4  Build only, do not start the game
    echo   5  Like 2, but with the Vulkan depth buffer off, to compare
    echo   6  Like 2, but with the original detail and view distance, to compare
    echo   7  Like 2, but without forcing the most detailed models
    echo   8  Like 2, but without the sub-pixel corner fix
    echo   9  Like 2, but with the original unconverted memory cache code
    echo   A  Like 2, but SDL3 runs the keyboard, mouse and sound
    echo   B  Like 1, through the menus, with SDL3 for the menu screen too - no 256-colour layer
    echo   C  Like 2, but with the original 640x480 HUD instead of the full-resolution one
    echo   D  M09 watcher run, converted: mission M09_SCN1, the Behemoth starts next to you, stack watchers on
    echo   E  M09 watcher run, original: like D, but with the original unconverted code
    echo   F  M06 watcher run, converted: mission M06_SCN1, stack watchers on
    echo   G  M06 watcher run, original: like F, but with the original unconverted code
    echo   H  Untouched 1.2 game, mission %MISSION%, Direct3D (the reference, in C:\Coding\HeavyGear\Ref12)
    echo.
    choice /c 123456789ABCDEFGH /n /m "Pick 1-9 or A-H: "
    if errorlevel 17 (
        set "MODE=ref12"
    ) else if errorlevel 16 (
        set "MODE=watch06noconv"
    ) else if errorlevel 15 (
        set "MODE=watch06"
    ) else if errorlevel 14 (
        set "MODE=watch09noconv"
    ) else if errorlevel 13 (
        set "MODE=watch09"
    ) else if errorlevel 12 (
        set "MODE=directnohud"
    ) else if errorlevel 11 (
        set "MODE=menusdl"
    ) else if errorlevel 10 (
        set "MODE=directsdl"
    ) else if errorlevel 9 (
        set "MODE=directnoconv"
    ) else if errorlevel 8 (
        set "MODE=directnosubpixel"
    ) else if errorlevel 7 (
        set "MODE=directnolod"
    ) else if errorlevel 6 (
        set "MODE=directorig"
    ) else if errorlevel 5 (
        set "MODE=directnoz"
    ) else if errorlevel 4 (
        set "MODE=build"
    ) else if errorlevel 3 (
        set "MODE=dx5"
    ) else if errorlevel 2 (
        set "MODE=direct"
    ) else (
        set "MODE=hgvk"
    )
)

pushd "%REPO%"
set "LOGS=%CD%\buildlogs"
if not exist "%LOGS%" mkdir "%LOGS%"
del /q "%LOGS%\*.log" >nul 2>&1

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
    echo Visual Studio Build Tools not found.
    goto :fail
)

echo [1/5] Setting up the 32-bit compiler
call "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" x86 > "%LOGS%\1-vcvars.log" 2>&1
if errorlevel 1 ( set "STEP=1-vcvars" & goto :fail )

taskkill /f /im HG.EXE >nul 2>&1

echo [2/5] Configuring
rem Configure only the first time; the build below reconfigures by itself when a CMakeLists changes.
if exist build\CMakeCache.txt (
    echo Already configured > "%LOGS%\2-configure.log"
    rem echo and taskkill leave a stale errorlevel behind; clear it before the check below.
    ver > nul
) else (
    cmake -S . -B build -A Win32 > "%LOGS%\2-configure.log" 2>&1
)
if errorlevel 1 ( set "STEP=2-configure" & goto :fail )

echo [3/5] Compiling
cmake --build build --config Release > "%LOGS%\3-compile.log" 2>&1
if errorlevel 1 ( set "STEP=3-compile" & goto :fail )

echo [4/5] Copying into %GAME%
if not exist "%GAME%\plugin" mkdir "%GAME%\plugin"
copy /y build\src\launcher\Release\HG.exe "%GAME%\HG.EXE" > "%LOGS%\4-copy.log" 2>&1
if errorlevel 1 ( set "STEP=4-copy" & goto :fail )

echo Build and deploy OK.
if /i "%MODE%"=="build" goto :done

set "CFGWORDS="
if /i "%MODE%"=="directnoz" (
    set "MODE=direct"
    set "CFGWORDS= nozbuf"
)
if /i "%MODE%"=="directorig" (
    set "MODE=direct"
    set "CFGWORDS= nodetail"
)
if /i "%MODE%"=="directnolod" (
    set "MODE=direct"
    set "CFGWORDS= nolod"
)
if /i "%MODE%"=="directnosubpixel" (
    set "MODE=direct"
    set "CFGWORDS= nosubpixel"
)
if /i "%MODE%"=="directsdl" (
    set "MODE=direct"
    set "CFGWORDS= sdlinput sdlaudio"
)
if /i "%MODE%"=="directnoconv" (
    set "MODE=direct"
    set "CFGWORDS= noconv"
)
if /i "%MODE%"=="directnohud" (
    set "MODE=direct"
    set "CFGWORDS= nohudhi"
)
if /i "%MODE%"=="watch06" (
    set "MODE=direct"
    set "MISSION=M06_SCN1"
    set "CFGWORDS= uninittrace"
)
if /i "%MODE%"=="watch06noconv" (
    set "MODE=direct"
    set "MISSION=M06_SCN1"
    set "CFGWORDS= uninittrace noconv"
)
if /i "%MODE%"=="watch09" (
    set "MODE=direct"
    set "MISSION=M09_SCN1"
    set "CFGWORDS= uninittrace"
)
if /i "%MODE%"=="watch09noconv" (
    set "MODE=direct"
    set "MISSION=M09_SCN1"
    set "CFGWORDS= uninittrace noconv"
)
if /i "%MODE%"=="ref12" (
    copy /y build\src\launcher\Release\HG.exe "%REF12%\HG.EXE" > nul
    >"%REF12%\DLLFILE.INI" echo DX5DRAW.DLL
    >"%REF12%\hgrc.cfg" echo d3d noconv
    set "HG_DEBUG=4"
    echo [5/5] Starting the untouched 1.2 game, mission %MISSION%
    pushd "%REF12%"
    HG.EXE %MISSION% /HIGH /D3D
    popd
    goto :done
)
if /i "%MODE%"=="direct" (
    >"%GAME%\DLLFILE.INI" echo HGVK.DLL
    >"%GAME%\hgrc.cfg" echo hgvk nocompat%CFGWORDS%
    set "HG_DEBUG=4"
    echo [5/5] Starting mission %MISSION% directly
    pushd "%GAME%"
    HG.EXE %MISSION% /HIGH /3DFX
    popd
    goto :done
)
if /i "%MODE%"=="menusdl" (
    >"%GAME%\DLLFILE.INI" echo HGVK.DLL
    >"%GAME%\hgrc.cfg" echo hgvk sdldisplay sdlinput sdlaudio
) else if /i "%MODE%"=="dx5noconv" (
    >"%GAME%\DLLFILE.INI" echo DX5DRAW.DLL
    >"%GAME%\hgrc.cfg" echo d3d noconv
) else if /i "%MODE%"=="dx5" (
    >"%GAME%\DLLFILE.INI" echo DX5DRAW.DLL
    >"%GAME%\hgrc.cfg" echo d3d
) else (
    >"%GAME%\DLLFILE.INI" echo HGVK.DLL
    >"%GAME%\hgrc.cfg" echo hgvk
)
set "HG_DEBUG=4"
echo [5/5] Starting the game (%MODE%)
pushd "%GAME%"
HG.EXE
popd

:done
popd
endlocal
exit /b 0

:fail
echo.
echo FAILED at step %STEP%. Last lines of buildlogs\%STEP%.log:
if defined STEP powershell -NoProfile -Command "Get-Content -Tail 15 '%LOGS%\%STEP%.log'"
popd
pause
endlocal
exit /b 1
