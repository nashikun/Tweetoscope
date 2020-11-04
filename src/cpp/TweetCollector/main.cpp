#include "TweetCollectorParams.hpp"
#include "TweetStreams.hpp"
#include "Processor.hpp"

#include <cppkafka/configuration.h>
#include <cppkafka/cppkafka.h>

int main(int argc, char* argv[])
{

    if(argc > 2)
    {
        std::cout << "Usage : " << argv[0] << " <config-filename>" << std::endl;
        return 0;
    }

    std::string param_path;
    if(argc == 2) {
        param_path = argv[1];
    }
    else {
        std::cout << "Using default config path: config/collector.ini" <<std::endl;
        param_path = "config/collector.ini";
    }
    tweetoscope::params::collector params(param_path);

    cppkafka::Configuration config = {
        { "bootstrap.servers", params.kafka.brokers},
        { "auto.offset.reset", params.kafka.offset_reset},
        { "group.id", params.kafka.group}
    };

    std::cout << std::endl
        << "Parameters : " << std::endl
        << "----------"    << std::endl
        << std::endl
        << params << std::endl
        << std::endl;
    
    cppkafka::Consumer consumer(config); 
    processor::MessageHandler handler(params.times.terminated, params.times.observation);

    consumer.subscribe({params.topic.in});

    while(true)
    {
        auto msg = consumer.poll();
        if( msg && ! msg.get_error() )
        {
            handler(std::move(msg));
        }
    }
    return 0;
}
