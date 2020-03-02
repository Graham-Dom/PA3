#!/bin/bash
for i in {1..100}; do ./multi-lookup 2>error; done | sort | uniq -c