#!/bin/bash 2>nul

tools\split.exe --input ./engine/v4k.h --output-path ./engine/split/
tools\split.exe --input ./engine/v4k.c --output-path ./engine/split/
tools\split.exe --input ./engine/v4k   --output-path ./engine/split/
