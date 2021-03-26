active [10] proctype main() {
    byte i = _pid // This is comment
    printf("hello world:  %d\n", i) // This is also comment
}

init{
    run main()
}