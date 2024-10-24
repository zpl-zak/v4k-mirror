#!/bin/bash 2>nul

@REM tools\join.exe --template engine/split/v4k.h.inl --path ./engine/split/ --output ./engine/v4k.h
@REM tools\join.exe --template engine/split/v4k.c.inl --path ./engine/split/ --output ./engine/v4k.c
tools\join.exe --template engine/split/v4k.x.inl --path ./engine/split/ --output ./engine/v4k
