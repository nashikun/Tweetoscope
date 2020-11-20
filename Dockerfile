FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y python3-pip cmake pkg-config git zookeeperd tar wget librdkafka-dev libboost-all-dev libssl-dev
RUN apt update
RUN apt install curl
RUN git clone https://github.com/HerveFrezza-Buet/gaml
RUN git clone https://github.com/mfontanini/cppkafka
RUN wget https://downloads.apache.org/kafka/2.6.0/kafka_2.13-2.6.0.tgz
RUN wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz

RUN tar -xvzf kafka_2.13-2.6.0.tgz
ENV KAFKA_PATH /kafka_2.13-2.6.0
RUN tar -zxvf boost_1_74_0.tar.gz
RUN cd boost_1_74_0 && ./bootstrap.sh --with-libraries=all && ./b2 --prefix=/usr/lib
RUN cd gaml; mkdir -p gaml/build; cd gaml/build; cmake .. -DCMAKE_INSTALL_PREFIX=/usr; make -j; make install
RUN rm boost_1_74_0.tar.gz kafka_2.13-2.6.0.tgz
RUN cd cppkafka; mkdir build; cd build; cmake ..; make; make install
RUN ldconfig
COPY requirements.txt .
RUN pip3 install -r requirements.txt

