#include <ostream>

#define BOOST_TEST_MODULE CollectorTest
#include <boost/test/unit_test.hpp>

#define private public
#include "TweetCollector/TweetCollectorParams.hpp"
#include "Producer.hpp"
#include "Consumer.hpp"
#include "TweetCollector/Processor.hpp"

void build_tweet(kafka::TweetProducer& producer, const std::string& topic,
        const std::string key, const std::string msg, const int time,
        const int magnitude, const int source_id)
{
    std::ostringstream os;
    os << "{"
       << "\"type\": \"tweet\""
       << ", \"msg\": \"" << msg << '"'
       << ", \"t\": " << time 
       << ", \"m\": " << magnitude 
       << ", \"source\": " << source_id
       << ", \"info\": \"\"" 
       << '}';

    producer.send_message(topic, os.str(), key);
}

std::string param_path{"config/collector_test.ini"};
tweetoscope::params::collector params(param_path, true);

kafka::TweetConsumer consumer(params);
kafka::TweetProducer producer(params);

BOOST_AUTO_TEST_CASE( connection_test )
{
    processor::MessageHandler handler(params);
    build_tweet(producer, params.topic.in, "0", "", 1, 10, 0); 
    int counter = 0;
    for(int i=0; i < 100; ++i)
    {
        auto msg = consumer.get_message();
        if(msg && !msg.get_error())
        {
            handler(std::move(msg));
            ++counter;
            break;
        }
    }
    if(!counter) BOOST_FAIL("Message not received");
}

BOOST_AUTO_TEST_CASE( sources_test )
{
    processor::MessageHandler handler(params);
    build_tweet(producer, params.topic.in, "0", "", 1, 10, 0); 
    build_tweet(producer, params.topic.in, "1", "", 1, 11, 1); 
    build_tweet(producer, params.topic.in, "2", "", 1, 12, 1); 
    build_tweet(producer, params.topic.in, "3", "", 1, 13, 2); 
    build_tweet(producer, params.topic.in, "4", "", 1, 14, 2); 
    build_tweet(producer, params.topic.in, "5", "", 1, 15, 2); 

    int counter = 0;
    for(int i=0; i < 150; ++i)
    {
        auto msg = consumer.get_message();
        if(msg && !msg.get_error())
        {
            handler(std::move(msg));
            if(counter++ == 6) break;
        }
    }
    if(counter < 5) BOOST_FAIL("Message not received");
    BOOST_ASSERT(handler.processor_map.size() == 3);
    for(int i = 0; i<3; ++i)
    {
        BOOST_ASSERT(handler.processor_map.find(i) != handler.processor_map.end());
        BOOST_ASSERT(handler.processor_map.at(i).symbols.size() == i+1);
    }
}

BOOST_AUTO_TEST_CASE( cascade_length_test )
{
    processor::MessageHandler handler(params);
    build_tweet(producer, params.topic.in, "0", "", 1, 10, 0); 
    build_tweet(producer, params.topic.in, "0", "", 1, 11, 0); 
    build_tweet(producer, params.topic.in, "0", "", 2, 12, 0); 
    build_tweet(producer, params.topic.in, "0", "", 3, 13, 0); 
    build_tweet(producer, params.topic.in, "0", "", 4, 14, 0); 
    build_tweet(producer, params.topic.in, "1", "", 1, 10, 0); 
    build_tweet(producer, params.topic.in, "1", "", 2, 10, 0); 
    build_tweet(producer, params.topic.in, "1", "", 3, 11, 0); 
    build_tweet(producer, params.topic.in, "1", "", 4, 12, 0); 
    build_tweet(producer, params.topic.in, "1", "", 5, 12, 0); 
    build_tweet(producer, params.topic.in, "1", "", 6, 12, 0); 

    int counter = 0;
    for(int i=0; i < 500; ++i)
    {
        auto msg = consumer.get_message();
        if(msg && !msg.get_error())
        {
            handler(std::move(msg));
            if(counter++ == 11) break;
        }
    }
    if(counter < 11) BOOST_FAIL("Message not received");
    BOOST_ASSERT(handler.processor_map.size() == 1);
    auto processor = handler.processor_map.find(0);
    BOOST_ASSERT(processor != handler.processor_map.end());
    BOOST_ASSERT(processor->second.symbols.size() == 2);
    auto first_key = processor->second.symbols.at(0); 
    auto second_key = processor->second.symbols.at(1); 
    BOOST_ASSERT(first_key.lock());
    BOOST_ASSERT(second_key.lock());
    BOOST_ASSERT(first_key.lock()->magnitudes.size() == 5);
    BOOST_ASSERT(second_key.lock()->magnitudes.size() == 6);
}

BOOST_AUTO_TEST_CASE( termination_test )
{
    processor::MessageHandler handler(params);
    build_tweet(producer, params.topic.in, "0", "", 1, 10, 0); 
    build_tweet(producer, params.topic.in, "0", "", 2, 12, 0); 
    build_tweet(producer, params.topic.in, "0", "", 3, 13, 0); 
    build_tweet(producer, params.topic.in, "0", "", 4, 14, 0); 
    build_tweet(producer, params.topic.in, "1", "", 30, 10, 0); 
    build_tweet(producer, params.topic.in, "1", "", 31, 10, 0); 
    build_tweet(producer, params.topic.in, "1", "", 32, 11, 0); 
    build_tweet(producer, params.topic.in, "1", "", 34, 12, 0); 

    int counter = 0;
    for(int i=0; i < 500; ++i)
    {
        auto msg = consumer.get_message();
        if(msg && !msg.get_error())
        {
            handler(std::move(msg));
            if(counter++ == 8) break;
        }
    }
    if(counter < 8) BOOST_FAIL("Message not received");
    BOOST_ASSERT(handler.processor_map.size() == 1);
    auto processor = handler.processor_map.find(0);
    BOOST_ASSERT(processor != handler.processor_map.end());
    BOOST_ASSERT(processor->second.symbols.size() == 2);
    auto first_key = processor->second.symbols.at(0); 
    auto second_key = processor->second.symbols.at(1); 
    BOOST_ASSERT(!first_key.lock());
    BOOST_ASSERT(second_key.lock());
}

BOOST_AUTO_TEST_CASE( partial_test )
{
    processor::MessageHandler handler(params);
    build_tweet(producer, params.topic.in, "0", "", 1, 10, 0); 
    build_tweet(producer, params.topic.in, "0", "", 2, 12, 0); 
    build_tweet(producer, params.topic.in, "0", "", 3, 13, 0); 
    build_tweet(producer, params.topic.in, "1", "", 7, 10, 0); 
    build_tweet(producer, params.topic.in, "0", "", 8, 10, 0); 
    build_tweet(producer, params.topic.in, "1", "", 9, 11, 0); 
    build_tweet(producer, params.topic.in, "1", "", 12, 12, 0); 

    int counter = 0;
    for(int i=0; i < 500; ++i)
    {
        auto msg = consumer.get_message();
        if(msg && !msg.get_error())
        {
            handler(std::move(msg));
            if(counter++ == 7) break;
        }
    }
    if(counter < 7) BOOST_FAIL("Message not received");
    BOOST_ASSERT(handler.processor_map.size() == 1);
    auto processor = handler.processor_map.find(0);
    BOOST_ASSERT(processor != handler.processor_map.end());
    BOOST_ASSERT(processor->second.symbols.size() == 2);
    auto first_key = processor->second.symbols.at(0); 
    auto second_key = processor->second.symbols.at(1); 
    BOOST_ASSERT(processor->second.symbols.at(0).lock());
    BOOST_ASSERT(processor->second.symbols.at(1).lock());
    auto partial = processor->second.partial_cascades;
    BOOST_ASSERT(partial.size() == 2);
    BOOST_ASSERT(partial.at(6).size() == 1);
    BOOST_ASSERT(partial.at(12).size() == 2);
}
