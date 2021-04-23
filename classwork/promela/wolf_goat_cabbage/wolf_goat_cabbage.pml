bit man = 0
bit wolf = 0
bit goat = 0
bit cabbage = 0

/*active proctype main(){
    do
    :: wolf != goat && goat != cabbage -> printf("man\n"); man = 1 - man
    :: goat != cabbage && man == wolf -> printf("wolf\n"); man = 1 - man; wolf = 1 - wolf
    :: man == goat -> printf("goat\n"); man = 1 - man; goat = 1 - goat
    :: wolf != goat && man == cabbage -> printf("cabbage\n"); man = 1 - man; cabbage = 1 - cabbage
    od
}

ltl goalNeverReached {!<>(man && wolf && goat && cabbage)}
*/

inline unsafe(){
    (wolf == goat || goat == cabbage) && man != goat
}

active proctype main(){
    do
    :: true -> atomic { printf("man\n"); man = 1 - man}
    :: man == wolf -> atomic {printf("wolf\n"); man = 1 - man; wolf = 1 - wolf}
    :: man == goat -> atomic {man = 1 - man; goat = 1 - goat}
    :: man == cabbage -> atomic {printf("cabbage\n"); man = 1 - man; cabbage = 1 - cabbage}
    od
}

ltl goalNeverReached {! (!( (wolf == goat || goat == cabbage) && man != goat) U (man && wolf && goat && cabbage)) }
