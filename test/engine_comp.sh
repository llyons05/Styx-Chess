#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Error: Invalid number of arguments." >&2
    echo "Usage: $0 <engine1> <engine2> <sec/move> <rounds>" >&2
    exit 1
fi

set -e
cd "$(dirname "$0")/.."

./fastchess-linux-x86-64/fastchess \
    -engine name=$1 cmd=$1 \
    -engine name=$2 cmd=$2 \
    -each st=$3 timemargin=50 \
    -rounds $4 -openings file=./resources/8moves_v3 format=pgn -srand $(date +%s) \
    -sprt elo0=0 elo1=5 alpha=0.1 beta=0.1 \
    -concurrency 10