#pragma once

#include <cppkafka/configuration.h>
#include <cppkafka/cppkafka.h>
#include <cppkafka/message_builder.h>

#include "TweetCollector/TweetCollectorParams.hpp"
#include "TweetCollector/TweetStreams.hpp"

namespace kafka
{
    struct TweetProducer
    {
        TweetProducer(tweetoscope::params::collector params): producer(get_config(params))
        {
        }

        void send_message(const std::string& topic, const std::string payload, const std::string& key)
        {
            producer.produce(cppkafka::MessageBuilder(topic).key(key).payload(payload));
        }

        private:
            static cppkafka::Configuration get_config(tweetoscope::params::collector params)
            {
                return {
                    { "bootstrap.servers", params.kafka.brokers},
                    { "log.connection.close", params.kafka.connection}
                };
            }

        private:
            cppkafka::Producer producer;
    };
}
