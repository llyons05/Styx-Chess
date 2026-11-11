#!/bin/sh
set -eu

envsubst '$LICHESS_TOKEN $VERSION_TAG $ENGINE_CONCURRENCY' \
    < /lichess-bot/config/config.yml.template \
    > /lichess-bot/config/config.yml

chmod 600 /lichess-bot/config/config.yml

exec "$@"