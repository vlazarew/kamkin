#define INVALID 255
#define BUFFER_SIZE 3

chan m = [1] of {
    byte,   // номер сообщения (SEQ): 0 или 1
    bool,   // признак ошибки (CRC не моделируется)
    byte    // передаваемые данные
};


chan a = [1] of {
    byte,   // номер подтверждения (SEQ): 0 или 1
    bool    // признак ошибки
};


byte sendSEQ = 0;           // текущий номер сообщения
byte sendACK = INVALID;     // текущий номер подтверждения

byte sendMSG = 0;           // переданное сообщение
byte recvMSG = INVALID;     // полученное сообщение

active proctype P() {
    byte ack; 
    bool err;

    do
    :: a?ack,err ->
        if
        :: !err ->  if
                    :: (ack == sendSEQ) ->
                        atomic {
                        sendSEQ = 1 - sendSEQ; // следующий SEQ
                        sendMSG = (sendMSG + 1) % BUFFER_SIZE
                        }
                    :: else             // игнорируем: ошибка некорректный номер
                    fi
        :: else                         // игнорируем: ошибка передачи
        fi
    :: nfull(m) -> m!sendSEQ,0,sendMSG // повторная передача
    od
}

active proctype Q(){
    byte seq;
    bool err;
    byte msg;

    do
    :: m?seq, err, msg -> 
        if
        :: !err -> seq = atomic{
            sendACK = seq;
            recvMSG = msg;
        :: else     // игнорируем: ошибка передачи 
        fi
    :: nfull(a) && (sendACK != INVALID) -> a!sendACK, 0
    od
}

active proctype msgMedia() {
    byte seq, msg;

    do
    :: true -> Loss: atomic {
                m?seq,_,msg;
                printf("Loss of SEQ=%d, MSG=%d\n", seq, msg)
                }
    :: true -> Corruption: atomic {
                m?seq,_,msg; m!seq,1,INVALID;
                printf("Corruption of SEQ=%d, MSG=%d\n", seq, msg)
                }
    :: true -> Normal: atomic {
                m?<seq,0,msg>; empty(m);
                printf("Transmission of SEQ=%d, MSG=%d\n", seq, msg)
                }
    od
}

active proctype ackMedia() {
    byte seq;

    do
    :: true -> Loss: atomic {
                a?seq,_;
                printf("Loss of ACK=%d\n", seq)
                }
    :: true -> Corruption: atomic {
                a?seq,_; a!seq,1;
                printf("Corruption of SEACKQ=%d\n", seq)
                }
    :: true -> Normal: atomic {
                a?<seq,0>; empty(a);
                printf("Transmission of ACK=%d\n", seq)
                }
    od
}

// GF{Normal}
#define LINK_ALIVE(link) []<>(link@Normal)
#define LINKS_ALIVE (LINK_ALIVE(msgMedia) && LINK_ALIVE(ackMedia))

// GF{sendMAS == recvMSG}
#define DATA_TRANSMITTED []<>(sendMSG == recvMSG)

ltl ReliableTransmission{
    LINKS_ALIVE -> DATA_TRANSMITTED
}