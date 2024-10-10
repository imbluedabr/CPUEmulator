#include "cpu.h"

//contains all the logic for the components

//ik bad code but i just needed a way to allocate memory without the bloat of vector
template <typename T> DynamicArray<T>::DynamicArray(size_t size) {
    this->array = new T[size];
    this->size = size;
}

template <typename T> T& DynamicArray<T>::operator[](size_t index) {
    static T;
    if (index < this->size) {
        return this->array[index];
    }
    return T;
}

template <typename T> DynamicArray<T>::~DynamicArray() {
    delete[] this->array;
    this->size = 0;
}

//might extend this in the future when i want to use polymorhpism for more flexibility
Component::Component(CPU* parent) { this->parent = parent; }

//RamMemory

template <typename T, typename ADR> RamMemory<T>::RamMemory(CPU* parent, ADR size) :
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
    for (ADR i = 0; i < adres; i++) {
        this->ram[i] = data[i];
    }
}

template <typename T, typename ADR> void RamMemory<T, ADR>::loadBios() {
    constexpr buffersize = wordSize/512;
    T bios[buffersize];//buffer for the bios
    std::ifstream input("Bios.bin", std::ios::binary);
    if (!input) {//check if the file is opened
        std::cerr << "Error: Could not open \"Bios.bin\"\n";
    }
    input.unsetf(std::ios::skipws);//read 512 bytes into the ram memory
    input.read(reinterpret_cast<char*>(bios), 512);//idk what  this stuff means lol
    load(bios, buffersize, 0x00FF);//load the bios into ram at adres 0x00FF where the reset vector points
}

template <typename T, typename ADR> void RamMemory<T, ADR>::dump(ADR start, ADR end) {
    for (ADR i = start, i < end; i++) {
        std::cout << std::hex << this->ram[i] << ", \n";
    }
}

template <typename T, typename ADR> T RamMemory<T, ADR>::read(ADR adres, unsigned char n) {
    
}


