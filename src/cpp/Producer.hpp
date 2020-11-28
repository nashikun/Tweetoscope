#pragma once

#include <cppkafka/configuration.h>
#include <cppkafka/cppkafka.h>
#include <cppkafka/message_builder.h>

#include "Logger.hpp"
#include "TweetCollector/TweetCollectorParams.hpp"
#include "TweetCollector/TweetStreams.hpp"

namespace kafka
{
    /*!
    *  Writes messages to kafka topics
    *
    *  @param params The collector configuration objet
    */
    struct TweetProducer
    {
        /*!
         * Constructor
         */
        TweetProducer(tweetoscope::params::collector params): producer(get_config(params))
        {
            LOG_INFO("Created consumer on brokers " + params.kafka.brokers);
        }

        /*!
         * Sends a message to a topic
         *
         * @param topic The topic to send the message to
         * @param key The message key
         * @param payload The message body
         */
        void send_message(const std::string& topic, const std::string payload, const std::string& key)
        {
            LOG_TRACE("Sent message on topic: " + topic + " with key: " + key + " and payload: " + payload);
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
