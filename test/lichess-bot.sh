set -e
cd "$(dirname "$0")/.."

docker run --rm -t -i --name styx-chess ghcr.io/llyons05/styx-chess:latest