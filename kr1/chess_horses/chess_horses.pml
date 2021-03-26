#define N 8 
#define square N * N

bool found = false;
int visitNumbers[square];
int x = 1;
int y = 1; 
int counter = 0;

inline canMove(target_x, target_y) {
    (target_x >= 1 && target_x <= N && target_y >= 1 && target_y <= N && ((target_x * N) + target_y) >= 0 && ((target_x * N) + target_y) <= square && visitNumbers[target_x * N + target_y] == -1)
}

inline reset(){
    true
}

proctype checker() {
    do
        :: canMove(x - 1, y - 2) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x - 1), (y - 2), counter, visitNumbers[(x - 1) * N + (y - 2)]); x = (x - 1); y = (y - 2); counter = counter + 1; visitNumbers[(x - 1) * N + (y - 2)] = counter};
        :: canMove(x - 2, y - 1) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x - 2), (y - 1), counter, visitNumbers[(x - 2) * N + (y - 1)]); x = (x - 2); y = (y - 1); counter = counter + 1; visitNumbers[(x - 2) * N + (y - 1)] = counter};
        :: canMove(x + 1, y - 2) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x + 1), (y - 2), counter, visitNumbers[(x + 1) * N + (y - 2)]); x = (x + 1); y = (y - 2); counter = counter + 1; visitNumbers[(x + 1) * N + (y - 2)] = counter};
        :: canMove(x + 2, y - 1) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x + 2), (y - 1), counter, visitNumbers[(x + 2) * N + (y - 1)]); x = (x + 2); y = (y - 1); counter = counter + 1; visitNumbers[(x + 2) * N + (y - 1)] = counter};
        :: canMove(x + 2, y + 1) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x + 2), (y + 1), counter, visitNumbers[(x + 2) * N + (y + 1)]); x = (x + 2); y = (y + 1); counter = counter + 1; visitNumbers[(x + 2) * N + (y + 1)] = counter};
        :: canMove(x + 1, y + 2) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x + 1), (y + 2), counter, visitNumbers[(x + 1) * N + (y + 2)]); x = (x + 1); y = (y + 2); counter = counter + 1; visitNumbers[(x + 1) * N + (y + 2)] = counter};
        :: canMove(x - 1, y + 2) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x - 1), (y + 2), counter, visitNumbers[(x - 1) * N + (y + 2)]); x = (x - 1); y = (y + 2); counter = counter + 1; visitNumbers[(x - 1) * N + (y + 2)] = counter};
        :: canMove(x - 2, y + 1) -> atomic {printf("Moved from (x:%d, y:%d) to (x:%d, y:%d) steps: %d visited: %d", x, y, (x - 2), (y + 1), counter, visitNumbers[(x - 2) * N + (y + 1)]); x = (x - 2); y = (y + 1); counter = counter + 1; visitNumbers[(x - 2) * N + (y + 1)] = counter};
        :: counter >= square -> reset()
    od

    int i;
    bool allFound = true;
    for (i : 0 .. square - 1){
        allFound = allFound * (visitNumbers[i] != -1);
    }
    found = allFound;
}

init{
    int i;
    for (i : 0 .. square - 1){
        visitNumbers[i] = -1
    }  

    run checker()
}

ltl goalNeverReached {!(<>found)}