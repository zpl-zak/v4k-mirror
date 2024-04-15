@echo off

set WORKDIR=%~dp0
call make cook

mkdir %WORKDIR%\tests > nul 2> nul
mkdir %WORKDIR%\tests\out > nul 2> nul
mkdir %WORKDIR%\tests\ref > nul 2> nul
mkdir %WORKDIR%\tests\diff > nul 2> nul

where gm > nul 2> nul
if errorlevel 1 (
    echo "gm" not found. Please install GraphicsMagick and add it to the PATH.
    pause
    exit /b 1
)

for %%x in (*.exe) do (
    echo Running %%x...
    start /wait "" "%%x" --nocook --capture=50

    if not exist %WORKDIR%\tests\ref\%%~nx.exe.png (
        echo Reference image not found. Copying %%~nx.exe.png ...
        copy %WORKDIR%\tests\out\%%~nx.exe.png %WORKDIR%\tests\ref\%%~nx.exe.png
    ) else (
        call gm compare -metric MSE -maximum-error 0.02 %WORKDIR%\tests\ref\%%~nx.exe.png %WORKDIR%\tests\out\%%~nx.exe.png -file %WORKDIR%\tests\diff\%%~nx.exe.png > nul 2> nul
        if errorlevel 1 (
            echo Images differ too much. Check %WORKDIR%\tests\diff\%%~nx.exe.png
        )
    )
)
