@echo off
for %%x in (*.exe) do (
    echo Running %%x...
    start /wait "" "%%x"
)
