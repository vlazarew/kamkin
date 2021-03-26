bit flag[2] = 0;
bit turn = 0;

active [2] proctype P() {
    int i = _pid;
    NCS: printf("NCS%d\n", i);
    SET: flag[i] = 1; turn = i;
    TST: !(flag[1-i] == 1 && turn == i)
    CRS: printf("CRS%d\n", i)
    RST: flag[i] = 0; goto NCS
}

// G{~(P0@CRS & P1@CRS)}
ltl safety {[](!(P[0]@CRS && P[1]@CRS))}

// G{P0@SET -> F(P0@CRS)}
ltl liveness {[](P[0]@SET -> <>P[0]@CRS)}