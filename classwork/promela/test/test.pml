#define N 4 

//one queen per each row (queen[i] -> x=i)
//queen[i] (x,y) = (i, y[i])

byte y[N] = 0;
bool found = false;

inline checker() {
 int i;
 int j;
 bool bitExists = false;
 for (i : 0 .. N - 1){
  for (j : i .. N - 1){
   bitExists = bitExists || ((y[i] == y[j] || i + y[i] == j + y[j] || i - y[i] == j - y[j]) && i!=j); 
  }
 }
 found = !bitExists 
}

active [N] proctype mover (){
 byte newY;
 do
 :: atomic { newY = y[_pid] + 1; y[_pid] = ((newY < N) -> newY : 0) ; checker()}
 od
} 

ltl goalNeverReached {!(<>found)}