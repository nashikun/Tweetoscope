"""
Random forest learner
"""

import numpy as np
import pickle
import argparse
import json
from collections import defaultdict

from sklearn.ensemble import RandomForestRegressor

from kafka import KafkaProducer, KafkaConsumer, TopicPartition

from ml.utils.logger import get_logger
from ml.utils.config import init_config


def init_parser():
    """!
    Initialises parser
    """

    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--broker-list",
                        type=str,
                        required=False,
                        help="the broker list")
    parser.add_argument("--config",
                        type=str,
                        required=True,
                        help="the path of the config file")
    return parser.parse_args()


def main():
    """
    Main predictor function
    """

    args = init_parser()
    config = init_config(args)
    consumer = KafkaConsumer(config["consumer_topic"],
                             bootstrap_servers=config["bootstrap_servers"])
    producer = KafkaProducer(
        bootstrap_servers=config["bootstrap_servers"])

    regressors = defaultdict(RandomForestRegressor)
    train_X = defaultdict(list)
    train_y = defaultdict(list)

    # Set the frequence of trainings of each random forest
    update_size = config["update_size"]

    logger = get_logger('learner',
                        broker_list=config["bootstrap_servers"],
                        debug=True)

    for message in consumer:

        t = message.key

        value = message.value.decode().replace("'", '"').replace('(', '[').replace(')',']')

        value = eval(value)
        inputs = value['X']  # (beta, n_star, G1)
        W = value['W']

        train_X[t].append(inputs)
        train_y[t].append(W)

        if not len(train_X[t]) % update_size:

            regressors[t].fit(train_X[t], train_y[t])

            regressor_message = pickle.dumps({"type": "model", "regressor": regressors[t]})

            producer.send('models',
                          key=t,
                          value=regressor_message,
                          partition=message.partition)

            logger.info("Model {}s updated and sent".format(t))


if __name__ == '__main__':
    main()
