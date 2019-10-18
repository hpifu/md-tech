#!/usr/bin/env python3

import argparse
import sys
import os
import json


def process(inputdir, author, author_id, output="stdout"):
    if output == "stdout":
        ofp = sys.stdout
    else:
        ofp = open(output, "w")
    for fn in os.listdir(inputdir):
        path = os.path.join(inputdir, fn)
        if os.path.isdir(path):
            continue
        if not fn.endswith("md"):
            continue

        article = {}
        fp = open(path)
        lines = fp.read().split("\n")
        if lines[0] == "---":
            for i in range(1, len(lines)):
                if lines[i] == "---":
                    article["content"] = "\n".join(lines[i+1:]).strip()
                    break
                kv = lines[i].split(":")
                article[kv[0].strip()] = ":".join(kv[1:]).strip()
        if "author" not in article:
            article["author"] = author
        if "tags" not in article:
            article["tags"] = ""
        if article["tags"].startswith("["):
            article["tags"] = ",".join([
                i.strip() for i in article["tags"].strip("[]").split(",")
            ])
        article["authorID"] = author_id
        print(json.dumps(article))
        fp.close()


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="""Example:
    python3 article.py -i ../hatlonely/article -a hatlonely
""")
    parser.add_argument(
        "-i", "--inputdir", required=True, type=str, help="input directory"
    )
    parser.add_argument(
        "-o", "--output", default="stdout", help="output filename"
    )
    parser.add_argument(
        "-a", "--author", default="hatlonely", help="author name"
    )
    parser.add_argument(
        "-u", "--author-id", type=int, default=0, help="author id"
    )
    args = parser.parse_args()
    process(args.inputdir, args.author, args.author_id, args.output)


if __name__ == "__main__":
    main()
