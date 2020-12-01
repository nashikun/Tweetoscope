import argparse
import json

from kafka import KafkaProducer, KafkaConsumer

from ml.utils.config import init_config

def init_parser():
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--broker-list", type=str, required=False, help="the broker list")
    parser.add_argument("--config", type=str, required=True, help="the broker list")
    return parser.parse_args()

def average(lst): 
    return sum(lst) / len(lst)

def main():

    args = init_parser()
    config = init_config(args)

    consumer = KafkaConsumer(
        config["consumer_topic"],
        bootstrap_servers=config["bootstrap_servers"],
        value_deserializer=lambda v: json.loads(v).decode('utf-8')
        )

    for message in consumer:

        n_tot = message.value["n_tot"]
        tweet_id = message.value["cid"]
        T_obs = message.value["T_obs"]

        if n_tot > config.retweet_limit:
            print("Tweet {} may reach an important size, {: .3f} retweets predicted with {}s of observation".format(tweet_id, n_tot, T_obs))

if __name__ == '__main__':
    main()