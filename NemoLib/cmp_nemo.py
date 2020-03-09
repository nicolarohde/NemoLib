from ast import literal_eval
from sys import argv
from bisect import insort


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

        yield l1, l2


def get_sorted_motifs(f, motif_dict):
    for l1, l2 in get_line_pair(f):
        motif = literal_eval(l2)
        motif.sort()

        if l1 not in motif_dict:
            motif_dict[l1] = [motif]
        else:
            insort(motif_dict[l1], motif)


def print_mismatches(expected, actual):
    if len(expected) != len(actual):
        print(f"Mismatch in length: expected = {len(expected)} actual = {len(actual)}")
    
    if expected.keys() != actual.keys():
        print(f"Mismatch in keys: expected = {expected.keys()} actual = {actual.keys()}")

    for expct, actl in zip(expected, actual):
        for e, a in zip(expct, actl):
            if e != a:
                print(f"Mismatch in motif: expected = {e} actual = {a}")


def main():
    expected_motifs = {}
    actual_motifs = {}

    with open(argv[1]) as correct_nemo_file:
        get_sorted_motifs(correct_nemo_file, expected_motifs)

    with open(argv[2]) as new_nemo_file:
        get_sorted_motifs(new_nemo_file, actual_motifs)

    print_mismatches(expected_motifs, actual_motifs)


if __name__ == "__main__":
    main()