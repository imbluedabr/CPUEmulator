#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <memory>
#include <cstdint>



//ew dude what the fuck
//TODO: port it to ncurses so i can compile on linux

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#else
//assume windows
#include <conio.h>

#endif

template <typename T> class DynamicArray {

    public:
        //size of the array in number of elements (not bytes!!)
        size_t size;

        //pointer to the array
        T* array;

        //pointer to the last element in the array
        T* lastElement;
        
        DynamicArray(size_t size);

        T& operator[](size_t index);

        ~DynamicArray();
};

//may also contain MMU logic since the read and write functions can be overwritten
class Memory {

    public:

        //constructor
        Memory(uint32_t size);
        
        //reset the memory
        void reset();
        
        //load an array of size "size" and at adres in ram at "adres"
        void load(uint32_t* data, uint32_t size, uint32_t adres);
        
        //load a binery file into ram
        void loadFile(const char* filen, size_t programSize, uint32_t adres);

        //dump a certain chunk of ram into the console
        void dump(uint32_t start, uint32_t end);

        //read a word from ram
        virtual uint32_t read(uint32_t adres, uint8_t n);

        //write a word to ram
        virtual void write(uint32_t adres, uint32_t value, uint8_t n);
    
    protected:
        //dynamic array
        DynamicArray<uint32_t> ram;

};

class BaseSystem {
    public:
        BaseSystem(Memory& mem);

        //sometimes necisairy for io devices and for DMA
        virtual void interrupt(uint8_t irqVector) = 0;
        
        Memory& _Memory;
};


//brah id rather rewrite the entire project in c then continue this torture
struct DMAChannel {
    DynamicArray<uint8_t>* source;
    uint32_t size; //the amount of words to transfer
    uint32_t sourceAdres; //the adres in the hardware buffer to read / write to
    uint32_t destAdres; //the adres in ram
    bool RW; //if the dma operation is to read or write to ram
    uint8_t irqVec;
};

class DMAController {
    
    public:
        //constructor
        DMAController(BaseSystem& baseSystem);
        
        //dma read request
        bool DMAOperation(int channel, bool RW, DynamicArray<uint8_t>* dest, uint32_t size, uint32_t destAdres, uint32_t sourceAdres, uint8_t irqVec);
        
        //updates the dma transfer, contains all dma transfer logic
        void update();

    protected:

        BaseSystem& _BaseSystem;
        
        //amount of DMA channels available
        static constexpr size_t channels = 16;
        
        //the dma channels
        DMAChannel DMAChannelArray[channels];

};


//interface for IO devices
class IODevice {
    
    public:
        //read callback, adres is relative to the io device
        virtual uint32_t read(uint8_t adres) = 0; //pure virtual functions
        virtual void write(uint8_t adres, uint32_t value) = 0;
};

class IOBusController {
    
    public:
        //constructor
        IOBusController();

        //add a device to the io bus at a certain index
        void addDevice(uint8_t index, IODevice* device);

        //read from the iobus(called by the cpu)
        uint32_t read(uint8_t adres);

        //write to the iobus(also called by the cpu)
        void write(uint8_t adres, uint32_t value);

    protected:
        //list of device pointers
        IODevice *devices[16];
};

class FlashDevice : public IODevice {
    
    public:
        enum IORegs {
            IO_DISKADR = 0x00, //sector adres on the disk
            IO_RAMADR  = 0x01, //adres in ram
            IO_BLOCKS  = 0x02, //amount of sectors to read / write
            IO_CR      = 0x03  //control register bit0 = read / write, bit1 = request dma
        };
        
        //holds the channel the flash memory should use
        uint8_t DMAChannel = 0;

        //holds the interupt that should fire when the dma is finished
        uint8_t DMAInterrupt = 0;
        
        //constructor
        FlashDevice(DMAController& dmacontroller, const char* imageName, uint32_t size);
        
        //load the Flash.bin file into the flash
        void loadFlash(const char* name);
        
        //store the flash into Flash.bin
        void storeFlash();
        
        //dump a portion of the flash into the console
        void dump(uint32_t start, uint32_t end);
        
        //update the FlashDevice, contains all the io and dma logic
        void update();

        //io read
        uint32_t read(uint8_t adres) override;

        //io write
        void write(uint8_t adres, uint32_t value) override;
    
    protected:
        //flash memory
        DynamicArray<uint8_t> flash;
        
        //Acces to DmaController class
        DMAController& _DMAController;

        //io registers
        uint32_t Registers[16];

        char* imageName;
};

class BasicStorageDevice : public IODevice {

    public:
        enum IORegs {
            IO_ADRES = 0,
            IO_DATA = 1
        };

        BasicStorageDevice(const char* filename, uint32_t size);

        uint32_t read(uint8_t adres) override;
        void write(uint8_t adres, uint32_t value) override;

        void update();

    protected:
        //storage
        DynamicArray<uint16_t> storage;

        uint16_t adres;
};


class SerialIODevice : public IODevice {
    
    public:
        enum IORegs {
            IO_MODE = 0x00, //0, 1: baud rate factor
            IO_CTRL = 0x01, //0: en TX, 1: en term, 2: en RX, 4: rst error
            IO_STATUS = 0x02, //contains error flags and status flags
            IO_DATA = 0x03  //writing to this port writes to TX, reading reads from RX
        };

        //i finna kill my self
        //constructor
        SerialIODevice();
        ~SerialIODevice();

        //update the SerialIODevice, contains all io logic
        void update();

        //io read
        uint32_t read(uint8_t adres) override;

        //io write
        void write(uint8_t adres, uint32_t value) override;

    protected:
        //transmiter buffer, this holds the characters being send out
        char TXBuffer[16];
        
        //reciever buffer, this holds the characters recieved before they are read out by the cpu
        char RXBuffer[16];

        uint16_t TXSize;
        uint16_t RXSize;
        
        //io registers
        uint32_t Registers[16];
};



