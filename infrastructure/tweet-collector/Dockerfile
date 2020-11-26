FROM mamar98/tweetoscope:master

ADD ./config/ ./config
COPY ./build/TweetCollector .

ENTRYPOINT ["./TweetCollector"]
