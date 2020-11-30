import unittest
import numpy as np
import sys

sys.path.append('src/python/')

import predictor

class PredictorTest(unittest.TestCase):

    def test_predictor(self):

        cascade = np.load("data/cascade_ex.npy")
        
        alpha = 2.4
        mu = 10

        p, beta = 0.02, 1./3600

        self.assertEqual(
            [round(predictor.prediction([p, beta], cascade, alpha, mu, t),4) for t in [300,600,1200,1800]],
            [290.7797, 285.6802, 285.1896, 281.0748]
            )

unittest.main()