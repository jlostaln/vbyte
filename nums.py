import numpy as np
import sys

def output_help():
    print("""
Generates benchmark data for problem sets 3 and 4 of programming for perfomance.

Outputs either unsorted or unsorted data with or without index queries

Do not pipe the output of this script directly to a program you are timing!s

Usage:
    python3 nums.py [options]

Options:
    -h           Print this message and terminate.
    -n <int>     Number of insertions and queries to generate. Defaults to 10^6.
    -m <int>     Number size limit. Defaults to 10^6.
    -s           Output sorted values, and do not ouptput index queries.
    -q <int>     Output an additional q index queries.
    -r <number>  Set the seed for the random number generator.

Example:
    python3 nums.py -r 1337 > data
        Generates a file called "data" using default values.
    """)
    exit()

def main(n, limit, sort, queries):
    a = np.random.randint(0, limit, size=n + 1, dtype=np.uint64)
    if sort:
        a.sort()
    a[0] = n
    sys.stdout.buffer.write(a)
    if queries > 0:
        a = np.random.randint(0, n, size=queries + 1, dtype=np.uint64)
        a[0] = queries
        sys.stdout.buffer.write(a)
    

if __name__ == "__main__":
    if "-h" in sys.argv or "help" in sys.argv:
        output_help()
    n = 1000000
    if "-n" in sys.argv:
        n = int(sys.argv[sys.argv.index("-n") + 1])
    limit = 1000000
    if "-m" in sys.argv:
        limit = int(sys.argv[sys.argv.index("-m") + 1])
    if "-r" in sys.argv:
        np.random.seed(int(sys.argv[sys.argv.index("-r") + 1]))
    sort = "-s" in sys.argv
    queries = 0
    if "-q" in sys.argv:
        queries = int(sys.argv[sys.argv.index("-q") + 1])
    main(n, limit, sort, queries)
