import argparse, chess, pathlib, subprocess

parser = argparse.ArgumentParser(description="Test two engines against each other.")
parser.add_argument("engine1", type=pathlib.Path, help="The path to the first engine")
parser.add_argument("engine2", type=pathlib.Path, help="The path to the second engine")
parser.add_argument("-n", default=100, type=int, help="The number of positions for the engines to play against each other")

def main():
    args = parser.parse_args()

if __name__ == "__main__":
    main()
