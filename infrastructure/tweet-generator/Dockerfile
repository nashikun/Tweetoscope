FROM mamar98/tweetoscope:master

ADD ./config/ ./config
ADD ./data/ ./data
COPY ./build/TweetGenerator .

ENTRYPOINT ["./TweetGenerator"]
