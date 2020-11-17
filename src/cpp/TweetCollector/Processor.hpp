#pragma once

#include <c++/7/bits/c++config.h>
#include <cppkafka/message.h>
#include <memory>
#include <map>
#include <iomanip>
#include <cppkafka/cppkafka.h>
#include <new>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/heap/binomial_heap.hpp>

#include "../Logger.hpp"
#include "TweetStreams.hpp"
#include "TweetCollectorParams.hpp"
#include "../Producer.hpp"

namespace processor
{
    class Cascade;
    class Processor;
    class MessageHandler;

    using ref = std::shared_ptr<Cascade>;

    struct Cascade_Comparator
    {
        bool operator()(ref op1, ref op2) const noexcept;
    };
    
    using cascade_queue = boost::heap::binomial_heap<ref, boost::heap::compare<Cascade_Comparator>>;

    class Cascade
    {
        public:
            Cascade(const tweetoscope::cascade::idf key, const tweetoscope::tweet& twt,
                    const tweetoscope::observations* observation, kafka::TweetProducer* producer)
                : msg(twt.msg), info(twt.info), key(std::to_string(key)), producer(producer), observation(observation)
            {}

            void add_tweet(const tweetoscope::tweet& twt)
            {
                if(!first_ts) first_ts = twt.time;
                last_ts = twt.time;
                magnitudes.push_back({twt.time, twt.magnitude});
            }

            const tweetoscope::timestamp& first_time() const noexcept {return first_ts;}

            const tweetoscope::timestamp& last_time() const noexcept {return last_ts;}

            void partial(tweetoscope::timestamp ts) const noexcept
            {
                std::ostringstream os;

                os << "{\'type\' : "  << "\'serie\'"   << " , "
                   << "\'cid\' : "    << key         << " , "
                   << "\'msg\' : "    << msg         << " , "
                   << "\'T_obs\' : "  << ts << " , "
                   << "\'tweets\' : [";
                
                for(auto ptr = magnitudes.begin(); ptr != magnitudes.end(); ++ptr)
                {
                    os << "(" << ptr->first << ", " << ptr->second << ")";
                    if(ptr != magnitudes.end() - 1) os << ", "; 
                }
                os << "]}";
                producer->send_message(partial_topic, os.str(), key);
                BOOST_LOG_TRIVIAL(debug) << "Cascade " << std::setw(5) << key << " has been observed for window: " << ts;
            }

            void terminate() const noexcept
            {
                std::ostringstream os;
                os << "{\'type\' : "  << "\'size\'"    << " , "
                    << "\'cid\' : "    << key         << " , "
                    << "\'n_tot\' : "  << magnitudes.size() << " , "
                    << "\'t_end\' : "  << last_ts << "}";
                
                for(auto& obs : *observation) producer->send_message(terminated_topic, os.str(), key);
                BOOST_LOG_TRIVIAL(debug) << "Cascade " << std::setw(5) << key << " has been terminated";
            }

        public:
            cascade_queue::handle_type location;
            const std::string key;
            inline static std::string partial_topic{};
            inline static std::string terminated_topic{};

        private:
            const std::string msg;
            const std::string info;
            const tweetoscope::observations* observation;
            kafka::TweetProducer* producer;
            bool terminated{false};
            tweetoscope::timestamp last_ts{0};
            tweetoscope::timestamp first_ts{0};
            std::vector<std::pair<tweetoscope::timestamp, double>> magnitudes;
    };

    bool Cascade_Comparator:: operator()(ref op1, ref op2) const noexcept
    {
        return op1->last_time() < op2->last_time();
    }

    struct Processor
    {

        Processor(const int termination, const tweetoscope::observations* observation, kafka::TweetProducer* producer): termination(termination), 
            observation(observation), producer(producer)
        {
            for(const auto& obs: *observation)
            {
                partial_cascades.insert({obs, {}});
            }
        }

        void operator()(const tweetoscope::cascade::idf key, tweetoscope::tweet&& twt)
        {
            ref r = std::make_shared<Cascade>(key, twt, observation, producer);
            auto [it, inserted] = symbols.insert(std::make_pair(key, r));
            if(inserted) BOOST_LOG_TRIVIAL(debug) << "Created new cascade with key: " << key;
            for(auto& [ts, cascades]: partial_cascades)
            {
                while(cascades.size())
                {
                    if(auto sp = cascades.front().lock())
                    {
                        if(sp->first_time() + ts < twt.time) 
                        {
                            sp->partial(ts);
                            cascades.pop();
                        }
                        else break;
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(debug) << "Removed stale shared pointer from partial cascades of obs: " << ts;
                        cascades.pop();
                    }
                }
                if(inserted) cascades.push(r);
            }

            while(!queue.empty() && queue.top()->last_time() + termination < twt.time)
            {
                auto c = queue.top();
                queue.pop();
                c->terminate();
            } 
            if(inserted) r->location = queue.push(r);

            if(auto sp = it->second.lock()) 
            {
                sp->add_tweet(twt);
                queue.update(sp->location);
                BOOST_LOG_TRIVIAL(debug) << "Added tweet to cascade of id: " << key;
            }
        }

        private:
            int termination;
            kafka::TweetProducer* producer;
            const tweetoscope::observations* observation;
            cascade_queue queue;
            std::unordered_map<tweetoscope::cascade::idf, std::weak_ptr<Cascade>> symbols;
            std::unordered_map<tweetoscope::timestamp, std::queue<std::weak_ptr<Cascade>>> partial_cascades;
    };

    struct MessageHandler
    {

        MessageHandler(const tweetoscope::params::collector& params): producer(params), termination(params.times.terminated), 
            observation(params.times.observation)
        {
            Cascade::partial_topic = params.topic.out_series;
            Cascade::terminated_topic = params.topic.out_properties;
        }

        void operator()(cppkafka::Message&& msg)
        {
            tweetoscope::tweet twt;
            int key = tweetoscope::cascade::idf(std::stoi(msg.get_key()));
            auto istr = std::istringstream(std::string(msg.get_payload()));
            istr >> twt;
            auto [it, inserted] = processor_map.try_emplace(twt.source, termination, &observation, &producer);
            BOOST_LOG_TRIVIAL(trace) << "Received tweet with key: " << key << " and timestamp: " << twt.time;
            if(inserted) BOOST_LOG_TRIVIAL(info) << "Created new processor for the source: " << twt.source;
            it->second(key, std::move(twt));
        }

        private:
            const tweetoscope::observations observation;
            int termination;
            kafka::TweetProducer producer;
            std::map<int, Processor> processor_map;
    };
}
