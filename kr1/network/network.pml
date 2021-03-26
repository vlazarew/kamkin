proctype source(chan out){
    do
    :: out ! 1 -> SEND:
    od
}

proctype sink(chan in_){
    do
    :: in_ ? _ -> RECV:
    od
}

proctype fork(chan in_, out1, out2){
    do
    :: in_ ? _ -> atomic{out1 ! 1; out2 ! 1}
    od
}

proctype join(chan in1, in2, out){
    do
    :: in1 ? _ -> in2 ? _; atomic{out ! 1}
    od
}

proctype switch(chan in_, out1, out2){
    do
    :: in_ ? _ -> atomic{out1 ! 1};
    :: in_ ? _ -> atomic{out2 ! 1};
    od
}

proctype merge(chan in1, in2, out){
    do
    :: in1 ? _ -> atomic{out ! 1};
    :: in2 ? _ -> atomic{out ! 1};
    od
}

proctype queue_in(chan in_, buf){
    do
    :: in_ ? _ -> atomic{buf ! 1}
    od
}

proctype queue_out(chan out, buf){
    do
    :: buf ? _ -> atomic{out ! 1}
    od
}

chan a = [0] of {bit};
chan b = [0] of {bit};
chan c = [0] of {bit};
chan d = [0] of {bit};
chan e = [0] of {bit};
chan f = [0] of {bit};
chan g = [0] of {bit};
chan h = [0] of {bit};


chan buf2 = [2] of {bit};
chan buf3 = [3] of {bit};


active proctype watchdog(){
	do
	:: timeout -> assert(false)
	od
}

init {
    atomic{
        run source(a);
        run source(b);
        run merge(a, e, c);
        run queue_in(c, buf3);
        run queue_out(d, buf3);
        run switch(d, e, f);
        run queue_in(b, buf2);
        run queue_out(g, buf2);
        run join(f, g, h);
        run sink(h);
    }
}