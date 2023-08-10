#!/bin/bash 2>nul

python tools/split.py --input ./engine/v4k.h --output-path ./engine/split/
python tools/split.py --input ./engine/v4k.c --output-path ./engine/split/
python tools/split.py --input ./engine/v4k   --output-path ./engine/split/
