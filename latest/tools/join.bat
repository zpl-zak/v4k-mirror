#!/bin/bash 2>nul

tools\join.exe --template engine/split/v4k.h.inl --path ./engine/split/ --output ./engine/v4k.h
tools\join.exe --template engine/split/v4k.c.inl --path ./engine/split/ --output ./engine/v4k.c
tools\join.exe --template engine/split/v4k.x.inl --path ./engine/split/ --output ./engine/v4k
