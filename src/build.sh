#!/usr/bin/env bash
curPath=$(pwd)
outPath="$(pwd)/outos"
 # mkdir $outPath
srcPath="$curPath/src/os"
pushd $srcPath

KOUT="kernel.s"
KFILES="sys/entry.asm \
    sys/blk.asm     \
    sys/fs.asm      \
    sys/mem.asm     \
    sys/proc.asm    \
    sys/math.asm    \
    sys/sysent.asm  \
    sys/trap.asm    \
    sys/exit.asm    \
    "
    
cat $KFILES > $KOUT

MSGLEN=24
for program in *.s; do
    p=$(basename -- "$program")
    p="${p%.*}"
    msg="Compiling $p "
    msglen=${#msg}
    echo -n $msg
    printf %$((MSGLEN-msglen))s
    $FASM $p.s ${outPath}/$p.bin
done

rm $KOUT

echo
echo "=== Making file system ==="
rm ${outPath}/mkimg*
$CC mkimg.c -funsigned-char -o ${outPath}/mkimg
popd
pushd $outPath
FILES="kernel.bin mon.bin"
./mkimg -o hwboot.img -b swboot.bin $FILES
./mkimg -r hwboot.img
# LABELS=$(echo $FILES | sed s/\.bin/\.labels/g)
# cat $LABELS -u > ${outPath}/hwboot.labels
echo "DONE"
popd