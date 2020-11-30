import numpy as np
import json
import pickle
import argparse

from kafka import KafkaProducer, KafkaConsumer
from sklearn.ensemble import RandomForestRegressor

from ml.utils.logger import get_logger
from ml.utils.config import init_config

def prediction(params, history, alpha, mu, t):
    """
    Returns the expected total numbers of points for a set of time points
    
    params   -- parameter tuple (p,beta) of the Hawkes process
    history  -- (n,2) numpy array containing marked time points (t_i,m_i)  
    alpha    -- power parameter of the power-law mark distribution
    mu       -- min value parameter of the power-law mark distribution
    t        -- current time (i.e end of observation window)
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
    return Ntot

def init_parser():
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--broker-list", type=str, required=False, help="the broker list")
    parser.add_argument("--config", type=str, required=True, help="the broker list")
    return parser.parse_args()

def main():
    args = init_parser()
    config = init_config(args)
    consumer = KafkaConsumer(*config["consumer_topic"],bootstrap_servers=config["bootstrap_servers"], value_deserializer=lambda v: json.loads(v.decode('utf-8')))

    producer = KafkaProducer(bootstrap_servers=config["bootstrap_servers"], value_serializer=lambda v: json.dumps(v).encode('utf-8'))

    alpha = config["alpha"]
    mu = config["mu"]
    alert_limit = config["alert_limit"]

    regressors = {time: RandomForestRegressor() for time in config["times"]}

    n_true = {}
    n_tots = {}
    forest_inputs = {}

    time_to_id = {time:idx for idx, time in enumerate(config["times"])}

    logger = get_logger('predictor', broker_list=config["bootstrap_servers"], debug=True)

    for message in consumer:

        if message.value['type'] == 'model':
            regressors = pickle.loads(message.value['regressors'])
            logger.info("Updated models received")

        if message.value['type'] == 'size':
            # When we receive the final size of a cascade, we compute the stats and the samples

            tweet_id = message.value['cid']
            n_true[tweet_id] = message.value['n_tot']
            t = message.key

            n_tot = n_tots[tweet_id][time_to_id[t]]

            are = abs(n_tot - n_true[tweet_id]) / n_true[tweet_id]

            stat_message = {
                'type': 'stat',
                'cid': tweet_id,
                'T_obs': t,
                'ARE': are
            }

            producer.send('stats', stat_message)

            beta, n_star, G1, n_obs = forest_inputs[tweet_id][time_to_id[t]]

            W = (n_true[tweet_id] - n_obs) * (1 - n_star) / G1

            sample_message = {
                'type': 'sample',
                'cid': tweet_id,
                'X': (beta, n_star, G1),
                'W': W
            }

            producer.send('sample', key = t, value = sample_message)

            logger.info("Stats and sample produced for tweet {} at time {}".format(tweet_id, t))

        if message.value['type'] == "parameters":

            print(message.value)
            t = message.key
            G1 = message.value['G1']
            n_star = message.value['n_star']
            tweet_id = message.value['cid']
            p, beta = message.value['params']
            msg = message.value['msg']
            n_obs = message.value['n_obs']

            n_tot = regressors[t].predict(beta, n_star, G1)

            n_tots[tweet_id] = n_tots.get(tweet_id, [])+[n_tot]

            forest_inputs[tweet_id] = forest_inputs.get(tweet_id, []) + [(beta, n_star, G1, n_obs)]

            alert_message = {
                'type': 'alert',
                'cid': tweet_id,
                'msg': msg,
                'T_obs': t,
                'n_tot': n_tot,
            }

            producer.send('alerts', alert_message)

            logger.info("Alert produced for tweet {} at time {}".format(tweet_id, t))

            if n_tot > alert_limit:
                logger.warning("Tweet {} may create an important cascade with {} retweets predicted".format(tweet_id, n_tot))
            

if __name__ == '__main__':
    main()
