#include <iostream>
#include <fstream>
#include <string>
#include <memory>

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

//forward decleration of class CPU so we can have parent refrences
class CPU;

class Component {
    public:
    CPU* parent;
    Component(CPU* parent);
};

template <typename T, typename ADR> class RamMemory : public Component {
    public:
    DynamicArray<T> ram;
    static constexpr size_t wordSize = sizeof(T);
    //constructor
    RamMemory(CPU* parent, ADR size);
    //reset the memory
    void reset();
    //load an array of size "size" and at ädres in ram at "adres"
    void load(T* data, ADR size, ADR adres);
    //load a binery file into ram
    void loadBios();
    //dump a certain chunk of ram into the console
    void dump(ADR start, ADR end);
    //read a word from ram
    T read(ADR adres, unsigned char n);
    //write a word to ram
    void write(ADR adres, T value, unsigned char n);
};

class FlashDevice : public Component {
    public:
    DynamicArray<unsigned char> flash;
    enum IORegs {
        IO_DISKADR = 0x0A, //sector adres on the disk
        IO_RAMADR  = 0x0B, //adres in ram
        IO_BLOCKS  = 0x0C, //amount of sectors to read / write
        IO_CR      = 0x0D  //control register bit0 = read / write, bit1 = request dma
    };
    //constructor
    FlashDevice(CPU* parent, unsigned int size);
    //load the Flash.bin file into the flash
    void loadFlash();
    //store the flash into Flash.bin
    void storeFlash();
    //dump a portion of the flash into the console
    void dump(unsigned int start, unsigned int end);
    //update the FlashDevice, contains all the io and dma logic
    void update();
};

class SerialIODevice : public Component {
    public:
    //constructor
    SerialIODevice(CPU* parent);
    //update the SerialIODevice, contains all io logic
    void update();
};

template <typename T, typename ADR> class DMAChannel {
    public:
    DynamicArray<T>* source;
    ADR size; //the amount of words to transfer
    unsigned int sourceAdres; //the adres in the hardware buffer to read / write to
    ADR destAdres; //the adres in ram
    bool RW; //if the dma operation is to read or write to ram
    unsigned char irqVec;
    DMAChannel();
};

template <typename T, typename ADR> class DMAControllerDevice : public Component {
    public:
    static constexpr size_t channels = 16;
    //the dma channels
    DMAChannel<T, ADR> DMAChannelArray[channels];
    //constructor
    DMAControllerDevice(CPU* parent);
    //dma read request
    bool DMAOperation(int channel, bool RW, DynamicArray<T>* dest, ADR size, unsigned int destAdres, ADR sourceAdres, unsigned char irqVec);
    //updates the dma transfer, contains all dma transfer logic
    void update();
};
