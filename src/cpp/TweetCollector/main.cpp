#include <iostream>
#include "../Consumer.hpp"
#include "../Producer.hpp"
#include "Processor.hpp"
#include "TweetCollectorParams.hpp"

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

    const tweetoscope::params::collector params(param_path);
    kafka::TweetConsumer consumer(params);
    processor::MessageHandler handler(params);

    while(true)
    {
        auto msg = consumer.get_message();
        if( msg && ! msg.get_error() )
        {
            handler(std::move(msg));
        }
    }
    return 0;
}
