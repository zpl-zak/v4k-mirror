#!/bin/bash 2>nul

python tools/join.py --template engine/split/v4k.h.inl --path ./engine/split/ --output ./engine/v4k.h
python tools/join.py --template engine/split/v4k.c.inl --path ./engine/split/ --output ./engine/v4k.c
python tools/join.py --template engine/split/v4k.x.inl --path ./engine/split/ --output ./engine/v4k
