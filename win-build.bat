@echo off
setlocal enabledelayedexpansion

REM ---------------------------------------------------------------------------
REM Packages a built LipidSpace.exe into Build\LipidSpace and zips it.
REM
REM   win-build.bat [path\to\LipidSpace.exe]
REM
REM With no argument the exe is looked for in the two locations that actually
REM occur: an in-tree qmake build (release\LipidSpace.exe, which is what CI
REM produces) and a Qt Creator shadow build (build\<kit>-Release\release\).
REM The kit name is discovered rather than hard-coded, so it survives a Qt
REM upgrade or a differently named kit.
REM
REM Qt itself is deployed with windeployqt, NOT from a hand-curated set of DLLs
REM checked into the repository. The hand-curated set was missing the
REM imageformats, iconengines, styles and tls plugins, so .ico and .svg
REM resources silently failed to load and HTTPS did not work at all.
REM
REM Every step is checked; the script fails loudly rather than producing a
REM half-populated zip.
REM ---------------------------------------------------------------------------

set "EXE=%~1"

if "%EXE%"=="" (
    if exist "release\LipidSpace.exe" set "EXE=release\LipidSpace.exe"
)
if "%EXE%"=="" (
    for /f "delims=" %%d in ('dir /b /ad "build\*Release*" 2^>nul') do (
        if exist "build\%%d\release\LipidSpace.exe" set "EXE=build\%%d\release\LipidSpace.exe"
    )
)
if "%EXE%"=="" (
    echo ERROR: no LipidSpace.exe found.
    echo        Build the project first, or pass the path as an argument:
    echo            win-build.bat path\to\LipidSpace.exe
    exit /b 1
)
if not exist "%EXE%" (
    echo ERROR: "%EXE%" does not exist.
    exit /b 1
)

REM windeployqt must come from the same Qt installation that built the exe, so
REM resolve it next to qmake rather than taking whichever one PATH offers first.
REM On the CI runner C:\mingw64\bin is ahead of Qt on PATH and ships its own
REM windeployqt6.exe belonging to a different Qt; that one looks for plugins
REM under its own prefix and fails with "Unable to find the platform plugin".
REM Whichever qmake generated the Makefile is by definition the right Qt.
set "QMAKE_EXE="
for /f "delims=" %%q in ('where qmake6 2^>nul') do (
    if "!QMAKE_EXE!"=="" set "QMAKE_EXE=%%q"
)
if "%QMAKE_EXE%"=="" (
    for /f "delims=" %%q in ('where qmake 2^>nul') do (
        if "!QMAKE_EXE!"=="" set "QMAKE_EXE=%%q"
    )
)
if "%QMAKE_EXE%"=="" (
    echo ERROR: neither qmake6 nor qmake is on PATH.
    echo        Run this from a Qt command prompt, or add ^<qt^>\bin to PATH.
    exit /b 1
)
for %%p in ("%QMAKE_EXE%") do set "QT_BIN=%%~dpp"

set "WINDEPLOYQT="
if exist "%QT_BIN%windeployqt6.exe" set "WINDEPLOYQT=%QT_BIN%windeployqt6.exe"
if "%WINDEPLOYQT%"=="" if exist "%QT_BIN%windeployqt.exe" set "WINDEPLOYQT=%QT_BIN%windeployqt.exe"
if "%WINDEPLOYQT%"=="" (
    echo ERROR: no windeployqt next to %QMAKE_EXE%
    exit /b 1
)

REM libgomp is not part of windeployqt's compiler-runtime set, so take it from
REM the same MinGW toolchain that compiled the exe. Do not search PATH for it:
REM that picks up whichever GCC happens to come first and can ship a libgomp
REM built by a different compiler than the rest of the application.
set "MINGW_BIN="
for /f "delims=" %%g in ('where g++ 2^>nul') do (
    if "!MINGW_BIN!"=="" for %%p in ("%%g") do set "MINGW_BIN=%%~dpp"
)
if "%MINGW_BIN%"=="" (
    echo ERROR: g++ is not on PATH, cannot locate libgomp-1.dll.
    exit /b 1
)
if not exist "%MINGW_BIN%libgomp-1.dll" (
    echo ERROR: libgomp-1.dll not found in %MINGW_BIN%
    exit /b 1
)

echo Packaging %EXE%
echo Deploying Qt with %WINDEPLOYQT%
echo MinGW runtime from %MINGW_BIN%

if exist "Build\LipidSpace" rmdir /s /q "Build\LipidSpace" || goto :fail
mkdir "Build\LipidSpace"          || goto :fail
mkdir "Build\LipidSpace\data"     || goto :fail
mkdir "Build\LipidSpace\examples" || goto :fail

copy /y "%EXE%" "Build\LipidSpace\"                                  || goto :fail
copy /y "libraries\cppgoslin\bin\win64\*.dll" "Build\LipidSpace\"     || goto :fail
copy /y "libraries\OpenBLAS\bin\win64\*.dll" "Build\LipidSpace\"      || goto :fail
REM OpenXLSX is linked statically - nothing to copy.

copy /y "%MINGW_BIN%libgomp-1.dll" "Build\LipidSpace\"               || goto :fail

REM No --release: windeployqt detects release/debug from the binary itself (it
REM reports "64 bit, release executable"). Passing --release explicitly makes it
REM classify the MinGW-built Qt plugins as non-matching and drop every one of
REM them, so it then reports "Unable to find the platform plugin" even though
REM plugins\platforms\qwindows.dll is right there. Known MinGW-only behaviour,
REM see msys2/MINGW-packages#6272. Locally on Qt 6.11.0 both forms select the
REM same 12 plugins, so dropping the flag costs nothing.
"%WINDEPLOYQT%" --no-translations --compiler-runtime "Build\LipidSpace\LipidSpace.exe" || goto :deployfail

copy /y "data\classes-matrix.csv" "Build\LipidSpace\data\"           || goto :fail
powershell -NoProfile -Command "Copy-Item 'data/images' -Destination 'Build/LipidSpace/data' -Recurse -Force" || goto :fail
copy /y "examples\Example-Dataset.xlsx" "Build\LipidSpace\examples\" || goto :fail
copy /y "examples\ThreeStudies.xlsx" "Build\LipidSpace\examples\"    || goto :fail
copy /y "LICENSE*" "Build\LipidSpace\"                               || goto :fail
copy /y "dist\README.txt" "Build\LipidSpace\"                        || goto :fail

REM Sanity check: the Qt platform plugin is what decides whether the packaged
REM application starts at all, so refuse to ship a bundle without it.
if not exist "Build\LipidSpace\platforms\qwindows.dll" (
    echo ERROR: platforms\qwindows.dll missing - windeployqt did not run correctly.
    exit /b 1
)

if exist "Build\LipidSpace.zip" del /q "Build\LipidSpace.zip"
powershell -NoProfile -Command "Compress-Archive -Path 'Build/LipidSpace' -DestinationPath 'Build/LipidSpace.zip' -Force" || goto :fail

echo.
echo Packaged Build\LipidSpace.zip
exit /b 0

:deployfail
echo.
echo ERROR: windeployqt failed.
echo   windeployqt: %WINDEPLOYQT%
echo   qmake:       %QMAKE_EXE%
echo   platform plugins next to that Qt:
dir /b "%QT_BIN%..\plugins\platforms" 2>nul || echo     ^(no plugins\platforms directory - wrong Qt installation^)
exit /b 1

:fail
echo.
echo ERROR: packaging failed at the step above.
exit /b 1
