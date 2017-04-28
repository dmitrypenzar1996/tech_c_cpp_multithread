import numpy as np
import os
from subprocess import Popen, PIPE
from timeit import default_timer as timer

TYPE = np.int32
MAX_VALUE = 2 ** 31
MIN_VALUE = - 2 ** 31
TEMP_SHUFFLED = "temp_shuffle.bin"
TEMP_SORTED = "temp_sorted.bin"
PATH_TO_EXT_SORT = "external_sort.exe"


def print_work_time(func):
    def wrapper(*args, **kwargs):
        start = timer()
        result = func(*args, **kwargs)
        end = timer()
        print("Execution take {}".format(end - start))
        return result
    return wrapper


def make_test(array_size,
              min_value=MIN_VALUE,
              max_value=MAX_VALUE,
              test_file_shuffle=TEMP_SHUFFLED):
    arr = np.random.randint(min_value, max_value, size=array_size, dtype=TYPE)
    arr.tofile(test_file_shuffle)


@print_work_time
def call_extern_sort(test_file_shuffle=TEMP_SHUFFLED,
                     test_file_sorted=TEMP_SORTED,
                     path_to_ext_sort=PATH_TO_EXT_SORT):
    proc = Popen(
        "./{} {} {}".format(path_to_ext_sort,
                            test_file_shuffle,
                            test_file_sorted),
        shell=True,
        stdout=PIPE,
        stderr=PIPE
    )
    proc.wait()
    proc.communicate()


def check(array_size,
          min_value=MIN_VALUE,
          max_value=MAX_VALUE,
          test_file_shuffle=TEMP_SHUFFLED,
          test_file_sorted=TEMP_SORTED,
          path_to_ext_sort=PATH_TO_EXT_SORT):

    make_test(array_size, min_value, max_value, test_file_shuffle)
    call_extern_sort(test_file_shuffle, test_file_sorted, path_to_ext_sort)
    arr = np.fromfile(test_file_sorted, dtype=TYPE)
    true_arr = np.sort(np.fromfile(test_file_shuffle, dtype=TYPE))

    os.remove(test_file_shuffle)
    os.remove(test_file_sorted)
    return np.equal(arr, true_arr).sum() == arr.shape[0]


for size in (2000, 4096, 10000, 16348, 25000, 100000, 1000000, 10000000):
    print "Testing on {} size".format(size)
    if check(size):
        print "Pass"
    else:
        print "Failed"
        break
