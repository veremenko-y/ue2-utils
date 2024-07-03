.set C, 1
.set Z, 2
.set TX, 0xffc

.export C
.export Z
.export TX

echo:
    stl     TX
    rsr
    
.export echo
