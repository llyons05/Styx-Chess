#!/bin/bash

set -e
cd "$(dirname "$0")/../engines"

ENGINE="${1:-styx_latest-linux}"

./"$ENGINE" -t 5 4865609 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
./"$ENGINE" -t 4 4085603 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
./"$ENGINE" -t 6 11030083 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
./"$ENGINE" -t 5 15833292 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -"
./"$ENGINE" -t 5 89941194 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -"
./"$ENGINE" -t 4 3894594 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -"