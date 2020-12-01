FROM ubuntu:18.04

RUN apt-get update && apt-get -y install python3-pip
COPY /build/dist/Tweetoscope_2020_06-1.0.tar.gz Tweetoscope_2020_06-1.0.tar.gz 
COPY /config/deployment /config

RUN pip3 install Tweetoscope_2020_06-1.0.tar.gz
RUN rm -r Tweetoscope_2020_06-1.0.tar.gz
