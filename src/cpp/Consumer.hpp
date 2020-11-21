#pragma once

#include <cppkafka/configuration.h>
#include <cppkafka/cppkafka.h>
#include <chrono>

#include "Logger.hpp"
#include "TweetCollector/TweetCollectorParams.hpp"
#include "TweetCollector/TweetStreams.hpp"

namespace kafka
{
    /*!
     *  Reads messages from a kafka topic
     * 
     *  @param params The collctor configuration object
     */
    struct TweetConsumer
    {
        /*!
         * Constructor
         */
        TweetConsumer(tweetoscope::params::collector params): consumer(get_config(params))
        {
            std::cout << std::endl
                << "Consumer parameters : " << std::endl
                << "----------"    << std::endl
                << std::endl
                << params << std::endl
                << std::endl;
            
            if(params.topic.timeout != "") 
            {
                std::chrono::milliseconds timeout{std::stoi(params.topic.timeout)};
                consumer.set_timeout(timeout);
            }
            consumer.subscribe({params.topic.in});
            BOOST_LOG_TRIVIAL(info) << "Consumer subscribed to topic " << params.topic.in;
        }

        /*!
         * Returns a message from the subscribed topic. 
         *
         * Message may be empty or contain an error
         */
        cppkafka::Message get_message()
        {
            return consumer.poll();
        }
        
        private:
            static cppkafka::Configuration get_config(tweetoscope::params::collector params)
            {
                return {
                 { "bootstrap.servers", params.kafka.brokers},
                     { "auto.offset.reset", params.kafka.offset_reset},
                     { "group.id", params.kafka.group}
                };
            }

        private:
            cppkafka::Consumer consumer; 
    };
}
