import json
import argparse



def main():
    print 'hello world'
    status = 0

def create_parser():
    parser = argparse.ArgumentParser(description='Create experiments file.')
    parser.add_argument("alg_type",
            help="Which algorithm 2k_spanner | 3_spanner")
    return parser

if __name__ == "__main__":
    parser = create_parser()
    args = parser.parse_args()
    if args.alg_type == "3_spanner":
        print "3_spanner_file"
    else:
        print "2k_spanner_file"

    main()

