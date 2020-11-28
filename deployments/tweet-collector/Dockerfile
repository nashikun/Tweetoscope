FROM ubuntu:18.04

RUN apt-get update && apt-get install -y librdkafka-dev

ADD ./build/config/deployment ./config
COPY ./build/TweetCollector .

ENTRYPOINT ["./TweetCollector"]
