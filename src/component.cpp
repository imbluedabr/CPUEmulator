#include "component.h"
#include "cpu.h"

//contains all the logic for the components

//ik bad code but i just needed a way to allocate memory without the bloat of vector
template <typename T> DynamicArray<T>::DynamicArray(size_t size) {
    if (size == 0) {
        std::cerr << "bro what the fuck you cant allocate 0 bytes in DynamicArray\n";
        return;
    }
    this->array = new T[size];
    this->size = size;
    this->lastElement = this->array + this->size - 1;//pointer to the last element
}

template <typename T> T& DynamicArray<T>::operator[](size_t index) {
    static T justkillmealready;
    if (index < this->size) {
        return this->array[index];
    }
    return justkillmealready;
}

template <typename T> DynamicArray<T>::~DynamicArray() {
    delete[] this->array;
    this->size = 0;
}

//RamMemory

//constructor
RamMemory::RamMemory(uint32_t size) : ram(size)
{
    //bios base adres
    this->biosBaseAdres = 0x00FF;
}

void RamMemory::reset() {
    for (uint32_t i = 0; i < this->ram.size; i++) {
        this->ram[i] = 0;
    }
}

void RamMemory::load(uint32_t* data, uint32_t size, uint32_t adres) {
    for (uint32_t i = 0; i < size; i++) {
        this->ram[adres+i] = data[i];
    }
}

void RamMemory::loadBios() {
    //the bios is a fixed 512 bytes so the buffer size is sizeof(word)/512
    constexpr size_t biosSize = 512/sizeof(uint32_t);
    uint32_t bios[biosSize];//buffer for the bios
    std::ifstream input("Bios.bin", std::ios::binary);
    if (!input) {//check if the file is opened
        std::cerr << "Error: Could not open \"Bios.bin\"\n";
        return;
    }
    input.unsetf(std::ios::skipws);//read 512 bytes into the ram memory
    input.read(reinterpret_cast<char*>(bios), 512);//idk what  this stuff means lol
    load(bios, buffersize, this->biosBaseAdres);//load the bios into ram at adres 0x00FF where the reset vector points
}

void RamMemory::dump(uint32_t start, uint32_t end) {
    for (; start < end; i++) {
        std::cout << std::hex << this->ram[i] << ", ";
    }
}

//read bytes from ram
uint32_t RamMemory::read(uint32_t adres, uint8_t n) {
    uint8_t byteSelect = adres & 0b1;
    uint32_t justenditalready = 0;//very bad code since we first copy data into this var and then we make another copy since we return it
    uint8_t* datapointer = (uint8_t*) &this->ram[adres >> 1];//add the pointer to the cacheline to the byteselect to calculate the final adres
    if (&datapointer[byteSelect] + n > (uint8_t*)this->ram.lastElement) {
        return justenditalready; //if the last byte that we read is above the last element we got to return
    }
    //i dont like this but it will have to do
    memcpy(&justenditalready, &datapointer[byteSelect], n);
    return justenditalready;
}

//TODO: rewrite these functions for better performance
void RamMemory::write(uint32_t adres, uint32_t value, uint8_t n) {
    //i dont like this but it will have to do
    uint8_t byteSelect = adres & 0b1; //get the value of the first bit
    uint8_t* datapointer = (uint8_t*) &this->ram[adres >> 1];
    if (&datapointer[byteSelect] + n > (uint8_t*)this->ram.lastElement) {
        return;
    }
    memcpy(&datapointer[byteSelect], &value, n);//copy the selected bytes into ram
}

//i dont like this but im too dumb to make it better


//class FlashDevice
FlashDevice::FlashDevice(uint32_t size) : flash(size), DMAControllerDevice()
{
    loadFlash();
}

//load the Flash.bin file into the flash memory
void FlashDevice::loadFlash() {
    std::ifstream file("Flash.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Error: could not open \"Flash.bin\"\n";
        return;
    }
    file.unsetf(std::ios::skipws);
    file.read((char*)this->flash.array, this->flash.size);
}

//store the flash memory into the Flash.bin file
void FlashDevice::storeFlash() {
    std::ofstream file("Flash.bin", std::ios::binary);
    if(!file) {
        std::cerr << "Error: could not open \"Flash.bin\"\n Warning: flash memory was not saved!\n";
        return;
    }
    file.unsetf(std::ios::skipws);
    file.write((char*)this->flash.array, this->flash.size);
}

void FlashDevice::dump(uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i++) {
        std::cout << std::hex << this->flash[i] << ", ";
    }
}

//TODO: add io functionality
//this will request dma when the right io registers get the right values
void FlashDevice::update() {
    if ((parent->IOBus[IO_CR] & 0x2) == 0x2) {//check if the dma request bit is set in the control register
        bool succes = parent->DMAController.DMAOperation(
            CPU::DMA_FLASH, //dma channel of the flash device
            parent->IOBus[IO_CR] & 0x1, //check bit 0 of the control register
            &this->flash,
            parent->IOBus[IO_BLOCKS] * 512,
            parent->IOBus[IO_RAMADR],
            parent->IOBus[IO_DISKADR] * 512,
            CPU::INT_FLASHDMA
        );
        if (succes) {
            parent->IOBus[IO_CR] &= !(0x2); //clear dma request bit
        }
    }
}


//class SerialIODevice
SerialIODevice::SerialIODevice() {
    parent->IOBus.atachDev(IO_RX, NULL, &SerialIODevice<T>::RXRead);
    parent->IOBus.atachDev(IO_TX, &SerialIODevice<T>::TXWrite, NULL);
}

template <typename T> T SerialIODevice<T>::RXRead(CPU* parent) { //if the cpu reads from RX, read it from RXBuffer
    T RXSize = parent->IOBus[IO_RXSIZE];
    if (RXSize > 0) {
        parent->IOBus[IO_RXSIZE]--;
        return parent->serialIO.RXBuffer[RXSize];
    }
    return 0;
}

template <typename T> void SerialIODevice<T>::TXWrite(CPU* parent, T value) { //if the cpu writes to TX add it to TXBuffer
    T TXSize = parent->IOBus[IO_TXSIZE];
    if (TXSize < 16) {
        parent->serialIO.TXBuffer[TXSize] = value;
        parent->IOBus[IO_TXSIZE]++;
    }
}

template <typename T> void SerialIODevice<T>::update() {
    //handle serial io stuff
    Word TXSize = parent->IOBus[IO_TXSIZE];
    if (TXSize > 0) { //if there is anything in the TXBuffer, send it to stdout
        parent->IOBus[IO_TXSIZE]--;
        std::cout << this->TXBuffer[TXSize-1];
    }
    if (kbhit()) { //incoming charcters are added to the RXBuffer
        Word RXSize = parent->IOBus[IO_RXSIZE];
        if (RXSize < 16) {
            this->RXBuffer[RXSize] = getch();
            parent->IOBus[IO_RXSIZE]++;
        }
    }
}


//now the dma stuff
//class DMAChannel
DMAChannel::DMAChannel() {
    this->source = NULL;
    this->size = 0;
    this->sourceAdres = 0;
    this->destAdres = 0;
    this->RW = false;
}

//class DMAControllerDevice
DMAControllerDevice::DMAControllerDevice() : Memory(0) {
    
}

//request DMA
bool DMAControllerDevice::DMAOperation(int channel, bool RW, DynamicArray<uint8_t>* dest, uint32_t size, uint32_t destAdres, uint32_t sourceAdres, uint8_t irqVec) {
    DMAChannel* currentChannel = &DMAChannelArray[channel];
    if (currentChannel->size == 0) {
        currentChannel->source = dest;
        currentChannel->size = size;
        currentChannel->sourceAdres = destAdres;
        currentChannel->destAdres = sourceAdres;
        currentChannel->RW = RW;
        currentChannel->irqVec = irqVec;
        return true;
    } else {
        return false; //dma request got denied since the channel was busy
    }
}

//handles the DMA requests
void DMAControllerDevice::update() {
    for (int channel = 0; channel < channels; channel++) {//this fr makes my brain hurt
        DMAChannel* currentChannel = &DMAChannelArray[channel];
        if (currentChannel->size > 0) {
            if (currentChannel->RW) {//write
                this->write(currentChannel->destAdres, (*currentChannel->source)[currentChannel->sourceAdres], sizeof(uint8_t));
                currentChannel->sourceAdres++; //increment and decrement all the pointers
                currentChannel->destAdres++;
                currentChannel->size--;
            } else {//read
                (*currentChannel->source)[currentChannel->sourceAdres] = this->read(currentChannel->destAdres, sizeof(uint8_t));
                currentChannel->sourceAdres++;//increment stuff
                currentChannel->destAdres++;
                currentChannel->size--;
            }
            if (currentChannel->size == 0) {//dma finished now we should trigger a hardware interupt
                parent->interupt(currentChannel->irqVec);//FIXME: use HardwareStack component
            }
        }
    }
}


