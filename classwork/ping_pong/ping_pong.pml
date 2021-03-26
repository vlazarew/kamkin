#define N 4

chan c = [1] of {int}

active [2] proctype P() {
    int x;

    if
    :: (_pid == 0) -> atomic{ c!0; printf("c!0") }
    :: else  
    fi

    S:  atomic{ c?x; printf("c?x\n")} ;
        atomic{c!(x+1)%N; printf("c!(x+1)\n")};
        goto S  
}

active proctype waychdog(){
    do
    :: timeout -> assert(false)
    od
}