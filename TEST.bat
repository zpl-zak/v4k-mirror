@echo off

cd /d "%~dp0\."

call make cook

mkdir tests > nul 2> nul
mkdir tests\out > nul 2> nul
mkdir tests\ref > nul 2> nul
mkdir tests\diff > nul 2> nul

where gm > nul 2> nul
if errorlevel 1 (
    echo "gm" not found. Please install GraphicsMagick and add it to the PATH.
    pause
    exit /b 1
)

for %%x in (*.exe) do (
    echo Running %%x...
    start /wait "" "%%x" --cook-jobs=0 --capture=50 --mute

    if not exist "tests\ref\%%~nx.png" (
        echo [pass] reference image not found. Copying %%~nx.png ...
        copy "tests\out\%%~nx.png" "tests\ref\%%~nx.png"
    ) else (
        call gm compare -metric MSE -maximum-error 0.0065 "tests\ref\%%~nx.png" "tests\out\%%~nx.png" -file "tests\diff\%%~nx.png"
        if errorlevel 1 (
            echo [fail] %%~nx.exe! Check "tests\diff\%%~nx.png"
        ) else (
            echo [pass] %%~nx.exe
            del "tests\diff\%%~nx.png"
        )
    )
)
