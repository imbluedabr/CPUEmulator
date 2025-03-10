#include "component.h"
#include "cpu.h"

class CPU_X16 : public CPU {
    public:
	    void write(Word adres, Word value, Byte n) override;
	    Word read(Word adres, Byte n) override;

	    void execute() override;
	    void reset() override;
	    void status() override;

	    CPU_X16();
}
