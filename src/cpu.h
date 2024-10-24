
#include "components.h"
//Word is the wordsize of the cpu
using Word = short unsigned int;
using Byte = unsigned char;

//should probally make this an interface using pure virtual functions and then
//inherit from CPU and overwrite all virtual functions so i can have multiple
//different cpu's and systems and still make every component compatible

class CPU {
    public:
    static constexpr Word register_count = 10;
    static constexpr Word intvTable = 0x0000; //location where the interupt vector table is stored in ram
    //core cpu data
    Word registers[register_count]; //a, b, c, d, e, f, pc, sr, ksp, usp
    enum regs {
        REG_A,
        REG_B,
        REG_C,
        REG_D,
        REG_E,
        REG_F,
        REG_PC,
        REG_SR,
        REG_SP = 8,
        REG_KSP = 8, //KSP and USP are for internal use only, the program can only see one SP
        REG_USP = 9
    };
    enum flags {
        FLAG_ZERO,
        FLAG_NEG,
        FLAG_CARRY,
        FLAG_USERMODE, //if the cpu is in usermode or not
        FLAG_INTERUPT, //if the cpu is handeling an interupt request
        FLAG_HALT,     //if the cpu is halted or not
        FLAG_ENABLEINT //if interupts are enabled or not
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
    void interupt(Byte intv);//cause an interupt with interupt vector intv
    Word pop(Byte n);//pop n amount of bytes from the stack
    void push(Word value, Byte n);//push n amount of bytes to the stack
    void incPC(Word i);
    Word getOpc(Byte n);

    //main functions
    void execute();
    void reset();
    void status();
};
