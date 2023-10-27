@echo off
cd %~dp0\..
pushd tools\
	rd/q/s editor
	xcopy/e/y ..\..\fwk-mirror\tools\editor editor\
popd
call tools\editor_rename.bat