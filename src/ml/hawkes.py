"""
Hawkes Estimator
"""
import numpy as np
import json                       # To parse and dump JSON
import argparse
import matplotlib.pyplot as plt
import scipy.optimize as optim

import re

from kafka import KafkaProducer, KafkaConsumer, TopicPartition

from ml.utils.logger import get_logger
from ml.utils.config import init_config

def loglikelihood(params, history, t):
    """!
    Returns the loglikelihood of a Hawkes process with exponential kernel
    computed with a linear time complexity
        
    @param params   parameter tuple (p,beta) of the Hawkes process
    @param history  (n,2) numpy array containing marked time points (t_i,m_i)  
    @param t        current time (i.e end of observation window)
    """
    
    p,beta = params    
    
    if p <= 0 or p >= 1 or beta <= 0.: return -np.inf

    n = len(history)
    tis = history[:,0]
    mis = history[:,1]
    
    LL = (n-1) * np.log(p * beta)
    logA = -np.inf
    prev_ti, prev_mi = history[0]
    
    i = 0
    for ti,mi in history[1:]:
        if(prev_mi + np.exp(logA) <= 0):
            print("Bad value", prev_mi + np.exp(logA))
        
        logA = np.log(prev_mi + np.exp(logA)) - beta * (ti - prev_ti)
        LL += logA
        prev_ti,prev_mi = ti,mi
        i += 1
        
    logA = np.log(prev_mi + np.exp(logA)) - beta * (t - prev_ti)
    LL -= p * (np.sum(mis) - np.exp(logA))

    return LL

def compute_MLE(history, t, alpha, mu,
                init_params=np.array([0.0001, 1./60]), 
                max_n_star = 1., display=False):
    """!
    Returns the pair of the estimated loglikelihood and parameters (as a numpy array)

    @param history     (n,2) numpy array containing marked time points (t_i,m_i)  
    @param t           current time (i.e end of observation window)
    @param alpha       power parameter of the power-law mark distribution
    @param mu          min value parameter of the power-law mark distribution
    @param init_params initial values for the parameters (p,beta)
    @param max_n_star  maximum authorized value of the branching factor (defines the upper bound of p)
    @param display     verbose flag to display optimization iterations (see 'disp' options of optim.optimize)
    """
    
    # Define the target function to minimize as minus the loglikelihood
    target = lambda params : -loglikelihood(params, history, t)
    
    EM = mu * (alpha - 1) / (alpha - 2)
    eps = 1.E-8

    # Set realistic bounds on p and beta
    p_min, p_max       = eps, max_n_star/EM - eps
    beta_min, beta_max = 1/(3600. * 24 * 10), 1/(60. * 1)
    
    
    # Define the bounds on p (first column) and beta (second column)
    bounds = optim.Bounds(
        np.array([p_min, beta_min]),
        np.array([p_max, beta_max])
    )
    
    # Run the optimization
    res = optim.minimize(
        target, init_params,
        method='Powell',
        bounds=bounds,
        options={'xtol': 1e-8, 'disp': display}
    )
    
    # Returns the loglikelihood and found parameters
    return(-res.fun, res.x)

def prediction(params, history, alpha, mu, t):
    """!
    Returns the expected total numbers of points for a set of time points
    
    @param params   parameter tuple (p,beta) of the Hawkes process
    @param history  (n,2) numpy array containing marked time points (t_i,m_i)  
    @param alpha    power parameter of the power-law mark distribution
    @param mu       min value parameter of the power-law mark distribution
    @param t        current time (i.e end of observation window)
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
    parser.add_argument("--partition", type=int, required=True, help="the broker list")
    return parser.parse_args()


def main():
    """
    Main predictor function
    """

    args = init_parser()
    config = init_config(args)
    logger = get_logger(f'hawkes-{config["partition"]}', broker_list=config["bootstrap_servers"], debug=True)
    consumer = KafkaConsumer(bootstrap_servers=config["bootstrap_servers"])
    consumer.assign([TopicPartition(config["consumer_topic"], config["partition"])])
    producer = KafkaProducer(bootstrap_servers=config["bootstrap_servers"], value_serializer=lambda v: json.dumps(v).encode("utf-8"), key_serializer=lambda v: json.dumps(v).encode("utf-8"))

    alpha = config["alpha"]
    mu = config["mu"]

    for message in consumer:
        mess = message.value.decode().replace("'", '"').replace('(', '[').replace(')',']')

        mess = eval(mess)
                
        cascade = np.array(mess["tweets"])
        tweet_id = mess["cid"]
        text = mess["msg"]
        T_obs = mess["T_obs"]
        p, beta = 0.02, 1/3600
        t = cascade[-1,0]
        LL = loglikelihood((p, beta), cascade, t)
        LL_MLE,MLE = compute_MLE(cascade, t, alpha, mu)
        p_est, beta_est = MLE
        N, G1, n_star = prediction([p_est, beta_est], cascade, alpha, mu, t)

        messfinal = {
            "type": "parameters",
            "cid": tweet_id,
            "msg":text,
            "n_obs": len(cascade),
            "n_supp": N,
            "params": list(MLE),
            "G1": G1,
            "n_star": n_star
            }
        
        producer.send(config["producer_topic"], key=T_obs, value=messfinal, partition=config["partition"])

        logger.info("Predicted params p = {: .3f} and beta = {: .3f} for tweet {} at time {} on partition: {}".format(p_est, beta_est, tweet_id, T_obs, config["partition"]))

if __name__ == '__main__':
    main()
