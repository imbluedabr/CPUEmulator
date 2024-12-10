#pragma once
#include <iostream>
#include <memory>
#include <cstdint>

using Word = uint16_t;
using Byte = uint8_t;

//should probally make this an interface using pure virtual functions and then
//inherit from CPU and overwrite all virtual functions so i can have multiple
//different cpu's and systems and still make every component compatible

struct IOEntry {
    void (*write)(CPU*, Word);
    Word (*read)(CPU*);
    Word val;
    IOEntry();
};

class IOBusHandeler : public Component {
    private:
    IOEntry IOList[256];
    public:
    IOBusHandeler(CPU* parent);
    void atachDev(Byte adres, void (*write)(CPU*, Word), Word (*read)(CPU*));
    Word readIO(Byte adres);
    void writeIO(Byte adres, Word value);
    Word& operator[](Byte adres);
};


class CPU {
    public:
    static constexpr Word register_count = 10;
    static constexpr Word intvTable = 0x0000; //location where the interupt vector table is stored in ram
    enum interupts {//just use the x86 interupt table since im trying to make it somewhat x86 compatable
	INT_DIVZERO,
	INT_RESERVED, //should be single step interupt but idk might use it for something else
	INT_NMI, //worst case scenario interupt caused by very bad hardware failures
	INT_BREAKPNT, //breakpoint interupt
	INT_OVERFLOW, //caused when INTO instruction is executed and the FLAG_CARRY is set
	INT_BOUND, //caused when the BOUND instruction is executed and it detected out of bounds indexing
	INT_INVOPC, //invalid opcode
	INT_DEVNOTAVAIL, //device not available, gets called everytime an fpu instruction is called since im not adding an fpu(yet)
	INT_DBLFAULT, //double fault, gets caused when there is another exception during a GPF exception
	INT_RESERVED1, //legacy interupt
	INT_INVTSS, //invalid TSS something about multi tasking, i dont rly understand much about this so...
	INT_SEGNPRES, //segment not present
	INT_STACKSEGFAULT, //stack segmentation fault
	INT_GPF, //general protection fault
	INT_PAGEFAULT, //page fault caused by accesing non existent memory or acces memory without the right privilege
	INT_RESERVED2, //another reserved one (i dont understand anything about its purpose)
	INT_FPUERROR, //x86 fpu error
	INT_ALIGNCHK, //alignment check
	INT_MACHCHECK, //machine check
	INT_SIMDFPERR, //SIMD floating point exception
        INT_FLASHDMA //flash DMA finished
    };
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
        FLAG_MASKINT //if interupts are enabled or not
    };

    IOBusHandeler IOBus; //handles IO

    //core devices
    RamMemory<Word, Word> memory;
    DMAControllerDevice<Byte, Word> DMAController;
    enum DMA_channels {//dma chanels
        DMA_FLASH
    };

    //io devices
    SerialIODevice<Word> serialIO;
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
    Word getOp(Byte n);

    //these are implementation specific
    virtual Word read(Word adres, Byte n) = 0;
    virtual void write(Word aders, Word value, Byte n) = 0;

    //main functions
    virtual void execute() = 0;
    virtual void reset() = 0;
    virtual void status() = 0;
};
