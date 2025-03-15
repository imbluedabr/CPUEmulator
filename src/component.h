#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdint>



//ew dude what the fuck
//TODO: port it to ncurses so i can compile on linux
#include <conio.h>

template <typename T> class DynamicArray {
    public:
    T* array;
    size_t size;
    T* lastElement;
    DynamicArray(size_t size);
    T& operator[](size_t index);
    ~DynamicArray();
};


class Memory {
    public:
    DynamicArray<uint32_t> ram;
    uint32_t biosBaseAdres;

    //constructor
    Memory(uint32_t size);
    //reset the memory
    void reset();
    //load an array of size "size" and at adres in ram at "adres"
    void load(uint32_t* data, uint32_t size, uint32_t adres);
    //load a binery file into ram
    void loadBios();
    //dump a certain chunk of ram into the console
    void dump(uint32_t start, uint32_t end);
    //read a word from ram
    uint32_t read(uint32_t adres, uint8_t n) virtual;
    //write a word to ram
    void write(uint32_t adres, uint32_t value, uint8_t n) virtual;
};


//brah id rather rewrite the entire project in c then continue this torture
struct DMAChannel {
    DynamicArray<uint8_t>* source;
    uint32_t size; //the amount of words to transfer
    uint32_t sourceAdres; //the adres in the hardware buffer to read / write to
    uint32_t destAdres; //the adres in ram
    bool RW; //if the dma operation is to read or write to ram
    uint8_t irqVec;
    DMAChannel();
};

class DMAControllerDevice : virtual public Memory {
    public:
    static constexpr size_t channels = 16;
    //the dma channels
    DMAChannel DMAChannelArray[channels];
    //constructor
    DMAControllerDevice();
    //dma read request
    bool DMAOperation(int channel, bool RW, DynamicArray<uint8_t>* dest, uint32_t size, uint32_t destAdres, uint32_t sourceAdres, uint8_t irqVec);
    //updates the dma transfer, contains all dma transfer logic
    void update();
};



class FlashDevice : virtual public DMAControllerDevice {
    public:
    DynamicArray<uint8_t> flash;
    enum IORegs {
        IO_DISKADR = 0x0A, //sector adres on the disk
        IO_RAMADR  = 0x0B, //adres in ram
        IO_BLOCKS  = 0x0C, //amount of sectors to read / write
        IO_CR      = 0x0D  //control register bit0 = read / write, bit1 = request dma
    };
    //constructor
    FlashDevice(uint32_t size);
    //load the Flash.bin file into the flash
    void loadFlash();
    //store the flash into Flash.bin
    void storeFlash();
    //dump a portion of the flash into the console
    void dump(uint32_t start, uint32_t end);
    //update the FlashDevice, contains all the io and dma logic
    void update();
};


class SerialIODevice {
    public:
    enum IORegs {
        IO_TX     = 0x1A,
        IO_TXSIZE = 0x1B, //amount of bytes currently in the buffer
        IO_RX     = 0x1C,
        IO_RXSIZE = 0x1D
    };
    char TXBuffer[16];
    char RXBuffer[16];
    //callbacks
    //i finna kill my self
    //static void TXWrite(CPU* parent, T value);
    //static T RXRead(CPU* parent);
    //constructor
    SerialIODevice();//my life's biggest bruh moment

    //update the SerialIODevice, contains all io logic
    void update();
};



