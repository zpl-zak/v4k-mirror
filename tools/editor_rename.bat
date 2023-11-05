@echo off
setlocal enabledelayedexpansion
cd %~dp0\..

pushd tools\editor
    for /r %%f in (.\*.c) do (
        set "filename=%%~nxf"
        if /i not "!filename!"=="rename.bat" (
            echo Processing: %%f
            ..\fwkren.exe %%f to
        ) else (
            echo Skipping %%f
        )
    )
    for /r %%f in (.\*.h) do (
        set "filename=%%~nxf"
        if /i not "!filename!"=="rename.bat" (
            echo Processing: %%f
            ..\fwkren.exe %%f to
        ) else (
            echo Skipping %%f
        )
    )
    for %%f in (".\fwk*") do (
        set "filename=%%~nf"
        set "newname=v4k!filename:fwk=!%%~xf"
        echo Renaming "%%f" to "!newname!"
        move "%%f" "!newname!"
    )
    echo All done.
    endlocal
popd