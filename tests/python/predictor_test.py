import unittest
import numpy as np
import os
import sys

myPath = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, myPath + '/../../src')

from ml import predictor

class PredictorTest(unittest.TestCase):

    def test_predictor(self):

        cascade = np.load("data/cascade_ex.npy")
        
        alpha = 2.4
        mu = 10

        p, beta = 0.02, 1./3600

        self.assertEqual(
            [round(predictor.prediction([p, beta], cascade, alpha, mu, t)[0], 4) for t in [300,600,1200,1800]],
            [290.7797, 285.6802, 285.1896, 281.0748]
            )
if __name__ == "__main__":
    unittest.main()
