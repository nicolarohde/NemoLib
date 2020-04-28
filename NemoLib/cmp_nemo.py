from ast import literal_eval
from sys import argv
from bisect import insort, bisect_left

def binary_search(a, x):
    'Locate the leftmost value exactly equal to x'
    i = bisect_left(a, x)
    if i != len(a) and a[i] == x:
        return i
    return -1


def get_line_no_comment(f):
    l = f.readline()
    while l and l.startswith('#'):
        l = f.readline()

    return l


def get_line_pair(f):
    while True:
        l1 = get_line_no_comment(f)
        l2 = get_line_no_comment(f)

        if not l2:
            break

        yield l1.rstrip('\n'), l2.rstrip('\n')


def get_sorted_motifs(f):
    motif_dict = {}

    for l1, l2 in get_line_pair(f):
        motif = literal_eval(l2)
        motif.sort()

        if l1 not in motif_dict:
            motif_dict[l1] = [motif]
        else:
            insort(motif_dict[l1], motif)

    return motif_dict


def print_mismatches(expected, actual):
    if len(expected) != len(actual):
        print(f"Mismatch in length: expected = {len(expected)} actual = {len(actual)}")
    
    if expected.keys() != actual.keys():
        print(f"Mismatch in keys: expected = {list(expected.keys())} actual = {list(actual.keys())}")

    wrong_motifs = {}

    for k in expected.keys():
        if k not in actual:
            print(f"The key {k} was NOT present in actual mapping but was expected to be")
            continue

        for expected_motif in expected[k]:
            if binary_search(actual[k], expected_motif) == -1:
                if k in wrong_motifs:
                    wrong_motifs[k] += 1
                else:
                    wrong_motifs[k] = 1

    for k in actual.keys():
        if k not in expected:
            print(f"The key {k} was present in actual mapping but was NOT expected to be")

    for k, v in wrong_motifs.items():
        print(f"The key {k} contained {v} unexpected motifs a {float(v) / float(len(actual[k])) * 100}% mismatch")


def main():
    expected_motifs = {}
    actual_motifs = {}

    with open(argv[1]) as correct_nemo_file:
        expected_motifs = get_sorted_motifs(correct_nemo_file)

    with open(argv[2]) as new_nemo_file:
        actual_motifs = get_sorted_motifs(new_nemo_file)

    print_mismatches(expected_motifs, actual_motifs)


if __name__ == "__main__":
    main()