@echo off

setlocal enableextensions enabledelayedexpansion

net session >nul 2>&1
if %errorLevel% NEQ 0 (
	echo Failure: Current permissions inadequate - retry as administrator.
	exit
)

where git.exe /q
if %ERRORLEVEL% EQU 0 (
	echo Git is already installed.
) else (
	echo Git is not installed. You can install it here: https://git-scm.com/download/win
	exit
)

where cmake.exe /q
if %ERRORLEVEL% EQU 0 (
	echo Cmake is already installed.
) else (
	echo Cmake is not installed. You can install it here: https://cmake.org/
)

where vcpkg.exe /q
if %ERRORLEVEL% EQU 0 (
	for /f "tokens=*" %%i in ('where vcpkg.exe') do set VCPKG_PATH=%%i
	echo Vcpkg is already installed.
) else (
	if exist "%ProgramFiles(x86)%\vcpkg" (
		set "VCPKG_PATH=%ProgramFiles(x86)%\vcpkg"
		echo Vcpkg is already installed.
	) else (
		set /P "VCPKG_PATH=Enter a path to install vcpkg to (if empty, uses %ProgramFiles(x86)%\vcpkg\): " || set "VCPKG_PATH=%ProgramFiles(x86)%\vcpkg"
		if not exist "!VCPKG_PATH!" (
			git clone https://github.com/Microsoft/vcpkg.git "!VCPKG_PATH!"
		)

		call "!VCPKG_PATH!/bootstrap-vcpkg.bat"
		call "!VCPKG_PATH!/vcpkg.exe" integrate install
	)
)

pushd ..
echo call cmake.exe -DCMAKE_TOOLCHAIN_FILE="!VCPKG_PATH!/scripts/buildsystems/vcpkg.cmake" ./
call cmake.exe -DCMAKE_TOOLCHAIN_FILE="!VCPKG_PATH!/scripts/buildsystems/vcpkg.cmake" ./
popd
