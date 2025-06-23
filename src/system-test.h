#include "component.h"

class SystemClock : public IODevice {
    public:

        SystemClock();
        
        uint32_t read(uint8_t adres) override;
        void write(uint8_t adres, uint32_t value) override;
        
        bool running;
};

class SystemTest : public BaseSystem {

    public:
        //ram memory
        Memory ram;

        //io bus interface
        IOBusController ioBus;

        //system clock
        SystemClock clock;

        //storage
        BasicStorageDevice storage;
        
        //serial port
        SerialIODevice serialIODevice;
        
        //constants useful for converting between bit width's
        static const uint16_t dataBusMax = 65535;

        static const uint16_t adresBusMax = 2047;

        static const uint16_t to11bit = 0x07FF;

        struct {
            uint16_t A;
            uint16_t DP;
            uint16_t BP;
            uint16_t SP;
            uint16_t PC;
        } reg;
        
        struct {
            bool zero;
            bool positive;
            bool negative;
        } flag;

        SystemTest();

        //does nothing
        void interrupt(uint8_t irqVector) override;
        
        //check the accumulator and update the flags
        void setFlags(uint16_t value);

        //jump to a certain 11 bit adres in memory
        void jump(uint16_t adres);
        
        void push(uint16_t val);

        uint16_t pop();

        //reset the computer
        void reset();

        //execute a single instruction
        void step();

        //shows all the registers
        void status();

        //start the computer
        void run();

        //run the computer in debug mode
        void runDebug();
};






