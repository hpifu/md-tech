#!/usr/bin/env python3

import argparse
import pymysql
import sys
import json

config = {
    "mysql": {
        "host": "127.0.0.1",
        "port": 3306,
        "user": "hatlonely",
        "password": "keaiduo1",
        "db": "article"
    },
}

conn = pymysql.connect(
    host=config["mysql"]["host"],
    user=config["mysql"]["user"],
    port=config["mysql"]["port"],
    password=config["mysql"]["password"],
    db=config["mysql"]["db"],
    charset="utf8mb4",
    cursorclass=pymysql.cursors.DictCursor
)


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
        with conn.cursor() as cursor:
            cursor.execute("""
            INSERT INTO articles (title, author, tags, content, ctime) VALUES (%s, %s, %s, %s)
            ON DUPLICATE KEY UPDATE content=VALUES(content), tags=VALUES(tags), ctime=VALUES(ctime)""", (
                obj["title"],  obj["author"], obj["tags"], obj["content"], obj["date"]
            ))
        conn.commit()
        ofp.write(line)
        ofp.flush()
    ofp.close()


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="""Example:
    python3 mysql.py
""")
    parser.add_argument(
        "-i", "--input", default="stdin", help="input filename"
    )
    parser.add_argument(
        "-o", "--output", default="stdout", help="output filename"
    )
    args = parser.parse_args()
    insert(args.input, args.output)


if __name__ == "__main__":
    main()
