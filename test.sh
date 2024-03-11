#!/bin/bash

# === Set up path to point to UE2 utils ===
export PATH="$(pwd)/bin:$PATH"
export PREFIX=ue2-
export AS=${PREFIX}as
export NM=${PREFIX}nm
export SIZE=${PREFIX}size
export REL=${PREFIX}rel
export LD=${PREFIX}ld

# === Set up test location ===
if [[ -d tmp ]]; then 
    #rm -rf tmp;
    find tmp -type f -delete
else
    mkdir tmp
fi

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
    pushd $dir 1>/dev/null || exit
    cecho "as:"
    if [[ -f 'as.sh' ]]; then
        bash as.sh
    else
        for f in *.s; do
            cecho $(pwd)/$f
            $AS -o "${f%.*}.out" $f
        done
    fi
    # cecho "nm:"
    # $NM *.out
    # cecho "size:"
    # $SIZE *.out
    # cecho "rel:"
    # for f in *.out; do
    #     echo $f
    #     $REL $f
    # done
    cecho "ld => obj:"
    $LD -rso out.obj *.out
    cecho "ld => bin:"
    $LD -o out.bin *.out
    popd 1>/dev/null
done


