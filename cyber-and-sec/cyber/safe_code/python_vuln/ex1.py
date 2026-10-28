# *********************************************************
#
# Description: Reads a config file path from CFG_FILE_NAME env var,
#              vulnerable to environment variable injection
# Author: Moshe Yifrah
#
# *********************************************************

import os


def read_configuration():
    cfg_file_name = os.getenv("CFG_FILE_NAME")

    color = ""
    prefix = ""

    with open(cfg_file_name, "r") as file:
        lines = file.readlines()
        lines = [line.rstrip() for line in lines]
        for line in lines:
            cfg = " ".join(line.split()).split(" ")
            if len(cfg) == 0 or len(cfg[0]) == 0 or cfg[0][0] == "#":
                continue
            elif cfg[0] == "color":
                color = cfg[1]
            elif cfg[0] == "prefix":
                prefix = cfg[1]
            else:
                print(f"\033[93mWrong config line: {cfg}\033[0m")

    return color, prefix


if __name__ == "__main__":
    color, prefix = read_configuration()
    print("ready!")
