#!/bin/bash
cat 01main.s > out.s
cat 02lib.s  >> out.s

$AS out.s