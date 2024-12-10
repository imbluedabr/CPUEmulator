#include "component.h"
#include "cpu.h"

class CPU_8086 : public CPU {
    public:
    enum MMU {
        MMU_BASEADR = 0,
        MMU_TOPADR = 1
    };

    void write(Word virtAdres, Word value, Byte n) override;
    Word read(Word virtAdres, Byte n) override;

    void execute() override;
    void reset() override;
	void status() override;

    CPU_8086();
};



