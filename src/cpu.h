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
    enum interupts { //interupt descriptor table
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
	struct {
		Byte REG_KSP;
		Byte REG_SR;
	} Enum;

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

    //utility functions
    void setBit(Byte index);
    bool getBit(Byte index);
    void clearBit(Byte index);
    Word parseFlags(Word value);
    void interupt(Byte intv);//cause an interupt with interupt vector intv
    Word pop(Byte n);//pop n amount of bytes from the stack
    void push(Word value, Byte n);//push n amount of bytes to the stack
    void incPC(Word i);
    Word getOp(Byte n);

    //these are implementation specific
    virtual Word read(Word adres, Byte n) = 0;
    virtual void write(Word aders, Word value, Byte n) = 0;
	virtual Word getReg(Byte reg) = 0;
    virtual void setReg(Byte reg, Word value) = 0;

    //main functions
    virtual void execute() = 0;
    virtual void reset() = 0;
    virtual void status() = 0;
};
