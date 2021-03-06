import unittest
import numpy as np
import os
import sys

myPath = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, myPath + '/../../src')

from ml import hawkes

class HawkesTest(unittest.TestCase):
    def test_cascade(self):

        cascade = np.load("data/cascade_ex.npy")

        alpha = 2.4
        mu = 10

        t = cascade[-1, 0]
        _, MLE = hawkes.compute_MLE(cascade, t, alpha, mu)

        p_est, beta_est = MLE

        self.assertEqual((round(100 * p_est, 3), round(10000 * beta_est, 3)),
                         (2.725, 2.747))

if __name__ == "__main__":
    unittest.main()
