#include "serialize.h"
void TestFixed32And64() {
    Serialize se(Serialize::SERIALIZER);

    uint32_t value1 = 0x12345678;
    uint64_t value2 = 0x0123456789abcdef;
    se.writeFixed32(value1);
    se.writeFixed64(value2);
    std::string res = se.toString();

    Serialize de(Serialize::DESERIALIZER, res);
    uint32_t ret1 = de.readFixed32();
    uint64_t ret2 = de.readFixed64();

    printf("value1: %x\tvalue2: %lx\n", value1, value2);
    printf("ret1  : %x\tret2  : %lx\n", ret1, ret2);

}

int main(int argc, char** argv) {
    // TestFixed32And64();
    return 0;
}
