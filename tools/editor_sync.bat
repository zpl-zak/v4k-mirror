@echo off
cd %~dp0\..
setlocal enabledelayedexpansion
pushd tools\
	rd/q/s editor
	xcopy/e/y ..\..\fwk-mirror\tools\editor editor\
popd
call tools\editor_rename.bat