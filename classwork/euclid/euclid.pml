proctype euclid(int a, b){
    int x = a;
    int y = b;

    do
    :: (x > y) -> x = x - y
    :: (x < y) -> y = y - x
    :: else -> break
    od;

    printf("gcd (%d, %d) = %d\n", a, b, x)
}

init{
    run euclid(120, 20)
}