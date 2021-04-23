chan c = [1] of {byte}

active [2] proctype P(){
    do
    :: c!0
    :: c?_
    od
}