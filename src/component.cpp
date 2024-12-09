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

//might extend this in the future when i want to use polymorhpism for more flexibility
Component::Component(CPU* parent) { this->parent = parent; }

//RamMemory

template <typename T, typename ADR> RamMemory<T, ADR>::RamMemory(CPU* parent, ADR size) :
							ram(size),
							Component(parent)
{

}

template <typename T, typename ADR> void RamMemory<T, ADR>::reset() {
    for (ADR i = 0; i < this->ram.size; i++) {
        this->ram[i] = 0;
    }
}

template <typename T, typename ADR> void RamMemory<T, ADR>::load(T* data, ADR size, ADR adres) {
    for (ADR i = 0; i < size; i++) {
        this->ram[adres+i] = data[i];
    }
}

template <typename T, typename ADR> void RamMemory<T, ADR>::loadBios() {
    constexpr size_t buffersize = wordSize/512;
    T bios[buffersize];//buffer for the bios
    std::ifstream input("Bios.bin", std::ios::binary);
    if (!input) {//check if the file is opened
        std::cerr << "Error: Could not open \"Bios.bin\"\n";
        return;
    }
    input.unsetf(std::ios::skipws);//read 512 bytes into the ram memory
    input.read(reinterpret_cast<char*>(bios), 512);//idk what  this stuff means lol
    load(bios, buffersize, 0x00FF);//load the bios into ram at adres 0x00FF where the reset vector points
}

template <typename T, typename ADR> void RamMemory<T, ADR>::dump(ADR start, ADR end) {
    for (ADR i = start; i < end; i++) {
        std::cout << std::hex << this->ram[i] << ", ";
    }
}

//read bytes from ram
template <typename T, typename ADR> T RamMemory<T, ADR>::read(ADR adres, unsigned char n) {
    char byteSelect = adres & 0b1;
    T justenditalready = 0;//very bad code since we first copy data into this var and then we make another copy since we return it
    unsigned char* datapointer = (unsigned char*) &this->ram[adres >> 1];//add the pointer to the cacheline to the byteselect to calculate the final adres
    if (&datapointer[byteSelect] + n > (unsigned char*)this->ram.lastElement) {
        return justenditalready; //if the last byte that we read is above the last element we got to return
    }
    //i dont like this but it will have to do
    memcpy(&justenditalready, &datapointer[byteSelect], n);
    return justenditalready;
}

template <typename T, typename ADR> void RamMemory<T, ADR>::write(ADR adres, T value, unsigned char n) {
    //i dont like this but it will have to do
    char byteSelect = adres & 0b1; //get the value of the first bit
    unsigned char* datapointer = (unsigned char*) &this->ram[adres >> 1];
    if (&datapointer[byteSelect] + n > (unsigned char*)this->ram.lastElement) {
        return;
    }
    memcpy(&datapointer[byteSelect], &value, n);//copy the selected bytes into ram
}

//finely regained some of my sanity after finishing these trash pieces of code


//class FlashDevice
FlashDevice::FlashDevice(CPU* parent, unsigned int size) : flash(size),
                                                           Component(parent)
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

void FlashDevice::dump(unsigned int start, unsigned int end) {
    for (int i = start; i < end; i++) {
        std::cout << std::hex << this->flash[i] << ", ";
    }
}

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
template <typename T> SerialIODevice<T>::SerialIODevice(CPU* parent) : Component(parent) {
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
template <typename T, typename ADR> DMAChannel<T, ADR>::DMAChannel() {
    this->source = NULL;
    this->size = 0;
    this->sourceAdres = 0;
    this->destAdres = 0;
    this->RW = false;
}

//class DMAControllerDevice
template <typename T, typename ADR> DMAControllerDevice<T, ADR>::DMAControllerDevice(CPU* parent) : Component(parent) {
    
}

//request DMA
template <typename T, typename ADR> bool DMAControllerDevice<T, ADR>::DMAOperation(int channel, bool RW, DynamicArray<T>* dest, ADR size, unsigned int destAdres, ADR sourceAdres, unsigned char irqVec) {
    DMAChannel<T, ADR>* currentChannel = &DMAChannelArray[channel];
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
template <typename T, typename ADR> void DMAControllerDevice<T, ADR>::update() {
    for (int channel = 0; channel < channels; channel++) {//this fr makes my brain hurt
        DMAChannel<T, ADR>* currentChannel = &DMAChannelArray[channel];
        if (currentChannel->size > 0) {
            if (currentChannel->RW) {//write
                parent->memory.write(currentChannel->destAdres, (*currentChannel->source)[currentChannel->sourceAdres], sizeof(T));
                currentChannel->sourceAdres++; //this needs to be changed so that it depends on the size of T
                currentChannel->destAdres++;
                currentChannel->size--;
            } else {//read
                (*currentChannel->source)[currentChannel->sourceAdres] = parent->memory.read(currentChannel->destAdres, sizeof(T));
                currentChannel->sourceAdres++;//this also needs to be changed
                currentChannel->destAdres++;
                currentChannel->size--;
            }
            if (currentChannel->size == 0) {//dma finished now we should trigger a hardware interupt
                parent->interupt(currentChannel->irqVec);
            }
        }
    }
}

//predefine template classes in this tu, dont forget to add if u use one with a new type

template class RamMemory<Word, Word>;
template class SerialIODevice<Word>;
template class DMAControllerDevice<Byte, Word>;
template class DynamicArray<Word>; //bruh thought it would recursivly figure out that these also need to be compiled but ig g++ is dumb ah
template class DynamicArray<Byte>;

