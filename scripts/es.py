#!/usr/bin/env python3

import argparse
import sys
import json
from elasticsearch import Elasticsearch

config = {
    "host": "127.0.0.1:9200",
}

es = Elasticsearch([config["host"]])


def insert(input="stdin", output="stdout"):
    if input == "stdin":
        ifp = sys.stdin
    else:
        ifp = open(input)
    if output == "stdout":
        ofp = sys.stdout
    else:
        ofp = open(output, "w")

    for line in ifp:
        obj = json.loads(line[:-1])
        res = es.index(index="article", doc_type='_doc', id=obj["id"], body={
            "id": obj["id"],
            "title": obj["title"],
            "tags": obj["tags"],
            "author": obj["author"],
            "content": obj["content"],
        })
        ofp.write(line)
        ofp.flush()
    ofp.close()


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="""Example:
    python3 es.py
""")
    parser.add_argument("-i", "--input", default="stdin", help="input filename")
    parser.add_argument("-o", "--output", default="stdout", help="output filename")
    args = parser.parse_args()
    insert(args.input, args.output)


if __name__ == "__main__":
    main()
