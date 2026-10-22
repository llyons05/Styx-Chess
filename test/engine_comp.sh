#!/bin/bash

if [ "$#" -ne 5 ]; then
    echo "Error: Invalid number of arguments." >&2
    echo "Usage: $0 <engine1> <engine2> <sec/move> <rounds> <estimated elo gain>" >&2
    exit 1
fi

set -e
cd "$(dirname "$0")/.."

./fastchess-linux-x86-64/fastchess \
    -engine name=$1 cmd=$1 \
    -engine name=$2 cmd=$2 \
    -each st=$3 timemargin=50 \
    -rounds $4 -openings file=./resources/8moves_v3.pgn format=pgn order=random -srand $(date +%s) \
    -sprt elo0=0 elo1=$5 alpha=0.05 beta=0.05 \
    -concurrency 8