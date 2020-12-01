"""
Tweet predictor
"""
import numpy as np
import json
import pickle
import argparse
import sklearn
from collections import defaultdict

from kafka import KafkaProducer, KafkaConsumer, TopicPartition
from sklearn.ensemble import RandomForestRegressor

from ml.utils.logger import get_logger
from ml.utils.config import init_config

def prediction(params, history, alpha, mu, t):
    """!
    Returns the expected total numbers of points for a set of time points
    
    @param params    parameter tuple (p,beta) of the Hawkes process
    @param history   (n,2) numpy array containing marked time points (t_i,m_i)  
    @param alpha     power parameter of the power-law mark distribution
    @param mu        min value parameter of the power-law mark distribution
    @param t         current time (i.e end of observation window)
    """

    p,beta = params
    
    tis = history[:,0]
   
    EM = mu * (alpha - 1) / (alpha - 2)
    n_star = p * EM
    if n_star >= 1:
        raise Exception(f"Branching factor {n_star:.2f} greater than one")
    n = len(history)

    I = history[:,0] < t
    tis = history[I,0]
    mis = history[I,1]
    G1 = p * np.sum(mis * np.exp(-beta * (t - tis)))
    Ntot = n + G1 / (1. - n_star)
    return Ntot, G1, n_star

def init_parser():
    """!
    Initialises parser
    """

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--broker-list", type=str, required=False, help="the broker list")
    parser.add_argument("--config", type=str, required=True, help="the path of the config file")
    parser.add_argument('--obs-window', type=str, help="the observation window", required=True)
    return parser.parse_args()

def main():
    """
    Main predictor function
    """

    args = init_parser()
    config = init_config(args)

    partition = config["obs_map"][config["obs_window"]]
    consumer = KafkaConsumer(
        bootstrap_servers=config["bootstrap_servers"],
        key_deserializer= lambda v: v.decode(),
    )
    consumer.assign([TopicPartition(topic, partition) for topic in config["consumer_topic"]])

    producer_samples = KafkaProducer(
            bootstrap_servers=config["bootstrap_servers"],
            value_serializer=lambda v: json.dumps(v).encode('utf-8'),
            key_serializer=str.encode)

    producer_alerts = KafkaProducer(
            bootstrap_servers = config["bootstrap_servers"],
            value_serializer=lambda v: json.dumps(v).encode('utf-8'))

    alpha = config["alpha"]
    mu = config["mu"]
    alert_limit = config["alert_limit"]

    regressor = RandomForestRegressor()

    sizes = defaultdict(dict)
    forest_inputs = {}

    #time_to_id = {time:idx for idx, time in enumerate(config["times"])}

    logger = get_logger('predictor', broker_list=config["bootstrap_servers"], debug=True)

    for message in consumer:

        try:
            mess = message.value.decode().replace("'", '"')
            mess = json.loads(mess)
        except:
            mess = pickle.loads(message.value)

        ###################   MODEL
        if mess['type'] == 'model':
            regressor = mess['regressor']
            logger.info("Updated model received")

        ###################   SIZE
        t = message.key
        if mess['type'] == 'size':
            # When we receive the final size of a cascade, we store it
            tweet_id = mess['cid']
            sizes[tweet_id]["real"] = mess['n_tot']

        if mess['type'] == "parameters":

            G1 = mess['G1']
            n_star = mess['n_star']
            tweet_id = mess['cid']
            p, beta = mess['params']
            msg = mess['msg']
            n_obs = mess['n_obs']

            try:
                sklearn.utils.validation.check_is_fitted(regressor)
                n_tot = regressor.predict((beta, n_star, G1))
            except:
                n_tot = n_obs + G1 / (1 - n_star)

            sizes[tweet_id]["prediction"] = n_tot

            forest_inputs[tweet_id] = [beta, n_star, G1, n_obs]

            alert_message = {
                'type': 'alert',
                'cid': tweet_id,
                'msg': msg,
                'T_obs': t,
                'n_tot': n_tot,
            }

            producer_alerts.send('alerts', key=None, value=alert_message)
            producer_alerts.flush()
            logger.info("Alert produced for tweet {} at time {}".format(tweet_id, t))

            if n_tot > alert_limit:
                logger.warning("Tweet {} may create an important cascade with {} retweets predicted".format(tweet_id, n_tot))

        if len(sizes[tweet_id].keys()) == 2:
            true_size = sizes[tweet_id]["real"]
            pred_size = sizes[tweet_id]["prediction"]
            are = abs(pred_size - true_size) / true_size

            stat_message = {
                    'type': 'stat',
                    'cid': tweet_id,
                    'T_obs': t,
                    'ARE': are
                    }

            producer_alerts.send('stats', key=None, value=stat_message)
            producer_alerts.flush()
            beta, n_star, G1, n_obs = forest_inputs[tweet_id]

            W = (true_size- n_obs) * (1 - n_star) / G1

            sample_message = {
                    'type': 'sample',
                    'cid': tweet_id,
                    'X': (beta, n_star, G1),
                    'W': W
                    }

            producer_samples.send('samples', key=args.obs_window, value=sample_message)
            producer_samples.flush()
            logger.info("Stats and sample produced for tweet {} at time {}".format(tweet_id, t))

if __name__ == '__main__':
    main()
