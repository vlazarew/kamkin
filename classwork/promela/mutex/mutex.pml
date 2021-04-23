int count = 1;

active [2] proctype P(){
    byte i = _pid;
    
    NCS: printf("NCS%d\n", i);
    SET: atomic {count > 0 -> count--}
    CRS: printf("CRS%d\n", i);
    RST: count++; goto NCS
}

ltl mutex {[](!(P[0]@CRS && P[1]@CRS))}