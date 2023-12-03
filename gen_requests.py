import random
import bisect
import math
import numpy as np
from functools import reduce


class ZipfGenerator:

    def __init__(self, m, alpha):
        # Calculate Zeta values from 1 to n:
        tmp = [1. / (math.pow(float(i), alpha)) for i in range(1, m + 1)]
        zeta = reduce(lambda sums, x: sums + [sums[-1] + x], tmp, [0])

        # Store the translation map:
        self.distMap = [x / zeta[-1] for x in zeta]

    def next(self):
        # Take a uniform 0-1 pseudo-random value:
        u = random.random()

        # Translate the Zipf variable:
        return bisect.bisect(self.distMap, u) - 1


def gen_zipf(m: int, alpha: float, n: int, start: int = 0) -> np.ndarray:
    """generate zipf distributed workload

    Args:
        m (int): the number of objects
        alpha (float): the skewness
        n (int): the number of requests
        start (int, optional): start obj_id. Defaults to 0.

    Returns:
        requests that are zipf distributed 
    """

    np_tmp = np.power(np.arange(1, m + 1), -alpha)
    np_zeta = np.cumsum(np_tmp)
    dist_map = np_zeta / np_zeta[-1]
    r = np.random.uniform(0, 1, n)
    return np.searchsorted(dist_map, r) + start


def gen_uniform(m: int, n: int, start: int = 0) -> np.ndarray:
    """generate uniform distributed workload

    Args:
        m (int): the number of objects
        n (int): the number of requests
        start (int, optional): start obj_id. Defaults to 0.

    Returns:
        requests that are uniform distributed
    """

    return np.random.uniform(0, m, n).astype(int) + start


if __name__ == "__main__":
    from argparse import ArgumentParser
    ap = ArgumentParser()
    ap.add_argument("-m", type=int, default=1000000, help="Number of objects")
    ap.add_argument("-n",
                    type=int,
                    default=100000000,
                    help="Number of requests")
    ap.add_argument("--alpha", type=float, default=1.0, help="Zipf parameter")
    ap.add_argument("--output_file",
                    type=str,
                    default="./dataset.txt",
                    help="Output to a file")

    p = ap.parse_args()

    output_file = open(p.output_file, "w") if p.output_file != "" else None
    batch_size = 1000000
    i = 0
    for n_batch in range((p.n - 1) // batch_size + 1):
        for obj in gen_zipf(p.m, p.alpha, batch_size):
            output_file.write(f"{obj}\n")
    output_file.close()
