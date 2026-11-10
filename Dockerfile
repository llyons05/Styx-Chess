FROM docker.io/lichessbotdevs/lichess-bot:latest

ARG VERSION_TAG=latest
ENV VERSION_TAG=$VERSION_TAG

COPY ./engines/styx_${VERSION_TAG}-linux ./engines/
COPY ./config.yml ./config/config.yml.template
COPY ./resources/PerfectX.bin ./books/
COPY ./test/entrypoint.sh ./entrypoint.sh

RUN apt-get update && apt-get install -y gettext-base \
    && rm -rf /var/lib/apt/lists/*

entrypoint ["./entrypoint.sh"]

CMD ["/bin/sh", "-c", "docker/copy_files.sh && python3 lichess-bot.py ${OPTIONS} --disable_auto_logging"]