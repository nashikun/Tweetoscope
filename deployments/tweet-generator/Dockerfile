FROM ubuntu:18.04

RUN apt-get update && apt-get install -y librdkafka-dev 

ADD ./build/config/deployment ./config
ADD ./data/ ./data
COPY ./build/TweetGenerator .

ENTRYPOINT ["./TweetGenerator"]
