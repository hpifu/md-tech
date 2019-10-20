#!/usr/bin/env bash

python3 article.py -i ../hatlonely/article -a hatlonely -u 1 | python3 mysql.py
