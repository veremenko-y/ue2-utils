#!/bin/env bash

PATH="../bin:$PATH"

declare -i org
detailed=false

function byte() {
    org+=1
    echo -e -n "\x$1" >> expect.bin
}

function offset() {
    printf "%x" $((org + $1))
}

function bz() {
    byte "00"
    byte "$1"
}
function bl() {
    byte "10"
    byte "$1"
}
function lda() {
    byte "20"
    byte "$1"
}
function ldl() {
    byte "30"
    byte "$1"
}
function stl() {
    byte "40"
    byte "$1"
}
function jsr() {
    byte "50"
    byte "$1"
}
function strh() {
    byte "60"
    byte "$1"
}
function strl() {
    byte "70"
    byte "$1"
}
function rsr() {
    byte "80"
    byte "$1"
}
function scf() {
    byte "90"
    byte "$1"
}
function adc() {
    byte "A0"
    byte "$1"
}
function cmp() {
    byte "B0"
    byte "$1"
}
function ror() {
    byte "C0"
    byte "$1"
}
function nand() {
    byte "D0"
    byte "$1"
}
function ori() {
    byte "E0"
    byte "$1"
}
function ore() {
    byte "F0"
    byte "$1"
}

files=()

function newfile() {
    files+=("$1")
    src $1
}

function src() {
    while IFS= read -r line; do
        IFS=';'; t=($line); unset IFS
        echo ${t[0]} >> $1.s
        if [ "$line" != "" ] && [ ${#t[@]} -eq 2 ]; then
            eval ${t[1]}
        fi
    done 
    if ! ue2-as $1.s -o $1.o; then
        echo ue2-as $1.s -o $1.o
    fi
}

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

function expect() {
    /usr/bin/cmp -s expect.bin built.bin
    status=$?
    if [[ $status = 0 ]]; then
        echo -e "${GREEN}[PASS]${NC}"
    else
        echo -e "${RED}[FAIL]${NC}"
        if $detailed ; then
            if [ -f "expect.bin" ] && [ -f "built.bin" ]; then
                /usr/bin/cmp expect.bin built.bin
                #bindiff expect.bin built.bin
            fi
            [[ -f expect.bin ]] && echo "expect.bin:" && hexdump -C expect.bin
            [[ -f built.bin ]] && echo "built.bin:" && hexdump -C built.bin
            [[ -f *.o ]] && ue2-nm *.o
            #ue2-dis expect.bin built.bin
            exit 1
        fi
    fi
}

function link() {
    ld="ue2-ld -b -o built.bin"
    while [ "$1" != "" ]; do
        ld="$ld $1.o"
        shift
    done
    if ! $ld ; then
        echo $ld
    fi
}
function cleanup() {
    files=()
    rm -f *.bin *.o *.s
}

function runtest() {
    #cleanup
    org=0
    f=$1
    name="${f%.*}"
    src $name < $f
    link $name $files
    printf "Test: %-16s" $name
    expect
}

cleanup

if [ "$1" == "--detailed" ] || [ "$1" == "-d" ]; then
    detailed=true
    shift
elif [ "$1" == "--help" ] || [ "$1" == "-h" ] ; then
    echo "Usage: test.sh [--detailed] [test_name]"
    echo
    echo "Options:"
    echo -e "\t-d, --detailed       fail at the first test and show binary diff"
    echo -e "\t-h, --help           this help"
    exit 0
fi
if [ "$1" == "" ]; then
    for f in *.test; do
    runtest $f
    cleanup
    done
else
    runtest "$1.test"
    cleanup
fi
