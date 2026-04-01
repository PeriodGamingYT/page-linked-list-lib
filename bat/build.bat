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
SET DebugCompilerFlags=/Zi /MTd /D DEBUG_MODE /DEBUG
SET ReleaseCompilerFlags=/O2

SET DefaultUseDebug=1

REM Flag Parameters
SET UseDebug=%DefaultUseDebug%

REM This variable is here for consistency with other
REM Batch files and for extra build options in the future
SET IsInvalidCommandLine=0

CLS
SETLOCAL ENABLEDELAYEDEXPANSION
SET StartPath=%CD%
PUSHD %~dp0\..
	FOR %%x IN (%*) DO (
		IF "%%x" == "debug" (
			SET UseDebug=1
		)

		IF "%%x" == "release" (
			SET UseDebug=0
		)

		IF "%%x" == "help" (
			ECHO build[.bat] [debug] [release] [help]
			ENDLOCAL
			POPD
			EXIT /B 0
		)
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, shutting down...
		ENDLOCAL
		CD %StartPath%
		EXIT /B 1
	)

	DEL /F /S /Q obj >NUL 2>&1

	SET FilesToCompile=
	FOR /R src %%x IN (*.c) DO (
		SET FilesToCompile=!FilesToCompile! %%x
	)

	MKDIR obj
	PUSHD obj
		SET CompilerFlags=%ReleaseCompilerFlags%
		IF "!UseDebug!" == "1" (
			SET CompilerFlags=%DebugCompilerFlags%
		)

		CL ^
			/I..\include ^
				!CompilerFlags! ^
				%FilesToCompile% ^
			/link /OUT:main.exe

		IF %ERRORLEVEL% NEQ 0 (
			ECHO Compiling failed, shutting down with error...
			POPD
			POPD

			REM I don't know exactly why a third one is needed,
			REM but it does ensure the user is taken to where
			REM they were before when the program crashes.
			POPD
			ENDLOCAL
			EXIT /B 1
		)

		MKDIR result
		PUSHD result
			COPY ..\main.exe .
			MKDIR assets
			XCOPY ..\..\assets .
		POPD
	POPD
POPD
ENDLOCAL
