#pragma once

#include <cppkafka/message.h>
#include <memory>
#include <map>
#include <cppkafka/cppkafka.h>
#include <unordered_map>
#include <vector>
#include <boost/heap/binomial_heap.hpp>

#include "TweetStreams.hpp"

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
            Cascade(tweetoscope::cascade::idf key, const tweetoscope::tweet& twt) : msg(twt.msg), info(twt.info), key(key) 
            {
            }

            void add_tweet(const tweetoscope::tweet& twt)
            {
                if(!first_ts) first_ts = twt.time;
                last_ts = twt.time;
                magnitudes.push_back({twt.time, twt.magnitude});
            }

            const tweetoscope::timestamp& first_time() const noexcept
            {
                return first_ts;
            }

            const tweetoscope::timestamp& last_time() const noexcept
            {
                return last_ts;
            }

            void terminate() const noexcept
            {
                std::cout << "key :" << key << " termination" << std::endl; 
            }

            void partial(tweetoscope::timestamp ts) const noexcept
            {
                std::cout << "key :" << key << " partial: " << ts << std::endl; 
            }

        public:
            cascade_queue::handle_type location;
            tweetoscope::cascade::idf key;

        private:
            std::string msg;
            std::string info;
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

        Processor(int termination, tweetoscope::observations observation): termination(termination), observation(observation)
        {
            for(auto obs: observation)
            {
                partial_cascades.insert({obs, {}});
            }
        }

        void operator()(const tweetoscope::cascade::idf key, tweetoscope::tweet&& tweet)
        {
            auto twt = std::move(tweet);
            while(!queue.empty() && queue.top()->last_time() < twt.time - termination)
            {
                ref r = queue.top();
                queue.pop();
                r->terminate();
            } 

            ref r = std::make_shared<Cascade>(key, twt);
            auto [it, inserted] = symbols.insert(std::make_pair(key, r));
            if(inserted)
            {
                r->location = queue.push(r);
            }

            for(auto& [ts, cascades]: partial_cascades)
            {
                while(cascades.size() && cascades.front()->first_time() < twt.time - ts)
                {
                    ref r = cascades.front();
                    cascades.pop();
                    r->partial(ts);
                }
                if(inserted)
                {
                    cascades.push(r);
                    std::cout << "inserted: " << key << " size: " << cascades.size() << std::endl;
                }
            }

            if(auto sp = it->second.lock()) 
            {
                sp->add_tweet(twt);
                queue.update(sp->location);
            }

            std::cout << "===================================================" << std::endl;
        }

        int termination;
        tweetoscope::observations observation;
        cascade_queue queue;
        std::unordered_map<tweetoscope::cascade::idf, std::weak_ptr<Cascade>> symbols;
        std::unordered_map<tweetoscope::timestamp, std::queue<ref>> partial_cascades;
    };

    struct MessageHandler
    {

        MessageHandler(int termination, tweetoscope::observations observation): termination(termination), observation(observation)
        {}

        void operator()(cppkafka::Message&& msg)
        {
            tweetoscope::tweet twt;
            int key = tweetoscope::cascade::idf(std::stoi(msg.get_key()));
            auto istr = std::istringstream(std::string(msg.get_payload()));
            istr >> twt;
            auto [it, changed] = processor_map.try_emplace(twt.source, termination, observation);
            it->second(key, std::move(twt));
        }

        private:
            tweetoscope::observations observation;
            int termination;
            std::map<int, Processor> processor_map;
    };
}
