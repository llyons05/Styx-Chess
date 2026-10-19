#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Error: Invalid number of arguments." >&2
    echo "Usage: $0 <engine1> <engine2> <sec/move> <rounds>" >&2
    exit 1
fi

set -e
cd "$(dirname "$0")/.."

./fastchess-linux-x86-64/fastchess \
    -engine name=lukechess1 cmd=$1 \
    -engine name=lukechess2 cmd=$2 \
    -each st=$3 timemargin=50 \
    -rounds $4 -openings file=./resources/UHO_4060_v4.epd format=epd