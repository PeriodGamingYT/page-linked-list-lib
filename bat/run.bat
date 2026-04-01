REM source-treks, grok and jump through C source code easily
REM Copyright (C) 2025  Nathan Phillips

REM This program is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 2 of the License, or
REM (at your option) any later version.

REM This program is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.

REM You should have received a copy of the GNU General Public License along
REM with this program; if not, write to the Free Software Foundation, Inc.,
REM 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

@ECHO OFF

PUSHD %~dp0\..
	CALL bat\vs-setup-caller-snippet.bat
POPD

REM Default parameters
REM ...

REM Flag parameters
SET NoCompile=0

CLS
SETLOCAL ENABLEDELAYEDEXPANSION
SET StartPath=%CD%
PUSHD %~dp0\..
	FOR %%x IN (%*) DO (
		IF "%%x" == "no-compile" (
			SET NoCompile=1
		)

		IF "%%x" == "help" (
			ECHO run[.bat] [no-compile]
			ENDLOCAL
			CD %StartPath%
			EXIT /B 0
		)
	)

	IF "!NoCompile!" == "0" (
		CALL bat\build.bat
	)

	IF %ERRORLEVEL% NEQ 0 (
		ECHO build.bat failed, shutting down with error...
		CD %StartPath%
		ENDLOCAL
		EXIT /B 1
	)

	IF EXIST obj\result\main.exe (
		START obj\result\main.exe
	)
POPD
ENDLOCAL
