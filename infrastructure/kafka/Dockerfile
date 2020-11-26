FROM mamar98/tweetoscope:master

ADD ./config/ ./config
COPY ./scripts/kafka.sh .

ENTRYPOINT ["sh", "-c", "./kafka.sh start && tail -f /dev/null"]
