import numpy as np
import pickle
import argparse

from sklearn.ensemble import RandomForestRegressor

from kafka import KafkaProducer, KafkaConsumer

from ml.utils.logger import get_logger
from ml.utils.config import init_config


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--broker-list",
                        type=str,
                        required=False,
                        help="the broker list")
    parser.add_argument("--config",
                        type=str,
                        required=True,
                        help="the broker list")
    return parser.parse_args()


def main():
    args = init_parser()
    config = init_config(args)

    consumer = KafkaConsumer("samples",
                             bootstrap_servers=config["bootstrap_servers"])
    producer = KafkaProducer(bootstrap_servers=config["bootstrap_servers"])

    regressors = {time: RandomForestRegressor() for time in config["times"]}
    train_X = {time: RandomForestRegressor() for time in config["times"]}
    train_y = {time: RandomForestRegressor() for time in config["times"]}

    # Set the frequence of trainings of each random forest
    update_size = config["update_size"]

    logger = get_logger('learner',
                        broker_list=config["bootstrap_servers"],
                        debug=True)

    for message in consumer:

        t = message.key

        inputs = message.value['X']  # (beta, n_star, G1)
        W = message.value['W']

        train_X[t].append(inputs)
        train_y[t].append(W)

        if len(train_X[t]) % update_size == 0:

            regressors[t].fit(train_X[t], train_y[t])

            regressor_message = pickle.dumps(regressors)

            producer.send('models', regressor_message)

            logger.info("Model {}s updated and sent".format(t))


if __name__ == '__main__':
    main()
