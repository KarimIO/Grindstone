@echo off

setlocal enableextensions enabledelayedexpansion

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
	>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
	>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"

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
	if exist "C:\vcpkg" (
		set "VCPKG_PATH=C:\vcpkg"
		echo Vcpkg is already installed.
	) else (
		set /P "VCPKG_PATH=Enter a path to install vcpkg to (Cannot have special characters. If empty, uses C:\vcpkg): " || set "VCPKG_PATH=C:\vcpkg"

		if not exist "!VCPKG_PATH!" (
			md "!VCPKG_PATH!"
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
