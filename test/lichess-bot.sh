set -e
cd "$(dirname "$0")/.."

docker run --rm -t -i -v ./engines/:/lichess-bot/config --name StyxChess lichessbotdevs/lichess-bot