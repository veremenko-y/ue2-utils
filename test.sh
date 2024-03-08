#!/bin/bash

# === Set up path to point to UE2 utils ===
PATH="$(pwd)/bin:$PATH"
# Hackery. Extracting PREFIX variable from makefile
source <(cat Makefile | grep ^PREFIX)

AS=${PREFIX}as
NM=${PREFIX}nm
SIZE=${PREFIX}size
REL=${PREFIX}rel
LD=${PREFIX}ld

# === Set up test location ===
if [[ -d tmp ]]; then 
    #rm -rf tmp;
    find tmp -type f -delete
fi

mkdir tmp
cp -r tests/* tmp
cd tmp

# === Running Test Itself ===
function cecho {
    tput setaf 2; echo "$1"; tput sgr0
}
path='t'
if [[ $1 != "" ]]; then
    path=t$1
fi
for dir in "$path"*; do
    pushd $dir || exit
    echo $(pwd)
    cecho "as:"
    for f in *.s; do
        $AS -o "${f%.*}.out" $f
    done
    # cecho "nm:"
    # $NM *.out
    # cecho "size:"
    # $SIZE *.out
    # cecho "rel:"
    # for f in *.out; do
    #     echo $f
    #     $REL $f
    # done
    cecho "ld:"
    $LD -r -o out.obj 01main.out 02lib.out
    #$LD -o out.bin 01main.out 02lib.out
    popd 1>/dev/null
done


