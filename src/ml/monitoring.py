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
        value_deserializer=lambda v: json.loads(v.decode('utf-8'))
        )

    are_list = []

    for message in consumer:

        are_list.append(message.value['ARE'])

        if len(are_list) % config["update_period"] == 0:
            print("Mean ARE: {}\nMear ARE on the {} last tweets: {}".format(
                average(are_list), 
                config["update_period"], 
                average(are_list[-config["update_period"]:]))
                )

if __name__ == '__main__':
    main()
