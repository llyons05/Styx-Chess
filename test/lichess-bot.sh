set -e
cd "$(dirname "$0")/.."

if [ "$#" -le 0 ]; then
    echo "Error: Invalid number of arguments." >&2
    echo "Usage: $0 <lichess-token> [num concurrent games]" >&2
    exit 1
fi

EC="${2:-2}"

docker run --rm -t -i -e LICHESS_TOKEN=$1 -e ENGINE_CONCURRENCY=$EC --name styx-chess ghcr.io/llyons05/styx-chess:latest