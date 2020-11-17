#pragma once

#include <cppkafka/configuration.h>
#include <cppkafka/cppkafka.h>
#include <cppkafka/message_builder.h>

#include "Logger.hpp"
#include "TweetCollector/TweetCollectorParams.hpp"
#include "TweetCollector/TweetStreams.hpp"

namespace kafka
{
    struct TweetProducer
    {
        TweetProducer(tweetoscope::params::collector params): producer(get_config(params))
        {
            BOOST_LOG_TRIVIAL(info) << "Created consumer on brokers" << params.kafka.brokers;
        }

        void send_message(const std::string& topic, const std::string payload, const std::string& key)
        {
            BOOST_LOG_TRIVIAL(trace) << "Sent message on topic: " << topic << " with key: " << key << " and payload: " << payload;
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
