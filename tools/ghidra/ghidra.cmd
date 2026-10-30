@echo off
rem Decompiles the game DLLs with Ghidra (headless) so the C can be read from
rem build\ghidra\<module>.c. Run with no arguments for a menu, or pass the
rem choice: 1 / sim, 2 / shell, 3 / all, 4 / resim, 5 / reshell.
rem The output is original game code: build\ is not committed.
setlocal
set "GHIDRA=C:\Coding\ghidra-12.1.4"
set "JAVA_HOME=C:\Coding\jdk-27"
rem Ghidra refuses project paths with "." or ".." parts, so resolve them.
for %%I in ("%~dp0..\..") do set "ROOT=%%~fI"
for %%I in ("%ROOT%\..\Project") do set "GAME=%%~fI"
for %%I in ("%~dp0.") do set "HERE=%%~fI"
set "OUT=%ROOT%\build\ghidra"

set "CHOICE=%~1"
if not defined CHOICE (
    echo.
    echo   Ghidra decompile
    echo   ----------------
    echo   1  HGSIM.DLL    mission code: analyse and decompile ^(about 35 minutes^)
    echo   2  HGSHELL.DLL  menus and campaign: analyse and decompile ^(much longer^)
    echo   3  Both, one after the other
    echo   4  HGSIM.DLL    export the C again from the saved analysis ^(quick^)
    echo   5  HGSHELL.DLL  export the C again from the saved analysis ^(quick^)
    echo.
    set /p "CHOICE=  Pick 1-5: "
)

set "MODULES="
set "MODE=import"
if "%CHOICE%"=="1" set "MODULES=HGSIM.DLL"
if /i "%CHOICE%"=="sim" set "MODULES=HGSIM.DLL"
if "%CHOICE%"=="2" set "MODULES=HGSHELL.DLL"
if /i "%CHOICE%"=="shell" set "MODULES=HGSHELL.DLL"
if "%CHOICE%"=="3" set "MODULES=HGSIM.DLL HGSHELL.DLL"
if /i "%CHOICE%"=="all" set "MODULES=HGSIM.DLL HGSHELL.DLL"
if "%CHOICE%"=="4" (set "MODULES=HGSIM.DLL" & set "MODE=process")
if /i "%CHOICE%"=="resim" (set "MODULES=HGSIM.DLL" & set "MODE=process")
if "%CHOICE%"=="5" (set "MODULES=HGSHELL.DLL" & set "MODE=process")
if /i "%CHOICE%"=="reshell" (set "MODULES=HGSHELL.DLL" & set "MODE=process")
if not defined MODULES (
    echo Not a choice: %CHOICE%
    pause
    exit /b 1
)

if not exist "%GHIDRA%\support\analyzeHeadless.bat" (
    echo Ghidra not found in %GHIDRA%
    pause
    exit /b 1
)
if not exist "%OUT%\proj" mkdir "%OUT%\proj"

for %%M in (%MODULES%) do (
    echo.
    echo === %%M: working, this takes a while ===
    if "%MODE%"=="process" (
        call "%GHIDRA%\support\analyzeHeadless.bat" "%OUT%\proj" hg -process %%M -noanalysis -scriptPath "%HERE%" -postScript ExportDecomp.java "%OUT%" < nul > "%OUT%\%%M.log" 2>&1
    ) else (
        call "%GHIDRA%\support\analyzeHeadless.bat" "%OUT%\proj" hg -import "%GAME%\%%M" -overwrite -scriptPath "%HERE%" -postScript ExportDecomp.java "%OUT%" < nul > "%OUT%\%%M.log" 2>&1
    )
    if errorlevel 1 (
        echo %%M failed, see build\ghidra\%%M.log
    ) else (
        echo %%M done: build\ghidra\%%~nM.c
    )
)
echo.
echo Finished. Tell Claude.
pause
