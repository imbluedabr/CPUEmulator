#include "components.h"
//Word is the wordsize of the cpu
typedef Word = short unsigned int;
typedef Byte = unsigned char;

//should probally make this an interface using pure virtual functions and then
//inherit from CPU and overwrite all virtual functions so i can have multiple
//different cpu's and systems and still make every component compatible

class CPU {
    public:
    static constexpr Word register_count = 12;
    //core cpu data
    Word registers[register_count]; //a, b, c, d, e, f, usp, ssp, pc, sr
    enum regs {
        REG_A,
        REG_B,
        REG_C,
        REG_D,
        REG_E,
        REG_F,
        REG_PC,
        REG_SR,
        REG_KSP,
        REG_USP
    };
    enum flags {
        FLAG_ZERO,
        FLAG_NEG,
        FLAG_CARRY,
        FLAG_USERMODE,
        FLAG_INTERUPT
    };
    Word IOBus[256];

    //core devices
    RamMemory<Word, Word> memory;
    DMAControllerDevice<Byte, Word> DMAController;

    //io devices
    SerialIODevice serialIO;
    FlashDevice flash;

    //constructor
    CPU();

    //inline utility functions
    void setBit(Byte index);
    bool getBit(Byte index);
    void clearBit(Byte index);
    Word getReg(Byte reg);
    void setReg(Byte reg, Word value);
    Word parseFlags(Word value);
    void interupt(Byte intv);//cause an interupt with interupt vector "intv"
    Word pop(Byte n);//pop n amount of bytes from the stack
    void push(Word value, Byte n);//push n amount of bytes to the stack

    //main functions
    void execute();
    void reset();
};
