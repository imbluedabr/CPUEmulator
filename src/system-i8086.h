#include "component.h"


class System8086 : public BaseSystem {
    
    public:
        //ram memory
        Memory ram;

        IOBusController ioBus;

        struct {
            uint16_t AX;
            uint16_t BX;
            uint16_t CX;
            uint16_t DX;
            uint16_t SP;
            uint16_t BP;
            uint16_t SI;
            uint16_t DI;
            uint16_t IP;
            union {
                struct {
                    bool carry : 1;
                    bool reserved1 : 1;
                    bool parity : 1;
                    bool reserved2 : 1;
                    bool auxCarry : 1;
                    bool reserved3 : 1;
                    bool zero : 1;
                    bool sign : 1;
                    bool trap : 1;
                    bool interrupt : 1;
                    bool direction : 1;
                    bool overflow : 1;
                };
                uint16_t value;
            } FLAGS;
            uint16_t CS;
            uint16_t DS;
            uint16_t SS;
            uint16_t ES;
        } reg;

        //if the computer is executing code or nah
        bool running;

        System8086();

        void pushW(uint16_t val);
        void pushB(uint8_t val);

        uint16_t popW();
        uint8_t popB();

        void interrupt(uint8_t irqVector) override;

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



