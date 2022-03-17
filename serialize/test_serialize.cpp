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

void TestSFixed32And64() {
    Serialize se(Serialize::SERIALIZER);
    int32_t a = 0xf0ffffff;
    int64_t b = 0xff0fffffffffffff;
    int32_t c = -1;
    int64_t d = -2;
    printf("Before serialize:\na: %x\tb: %lx\tc: %d\td: %ld\n", a, b, c, d);
    se.writeSFixed32(a);
    se.writeSFixed64(b);
    se.writeSFixed32(c);
    se.writeSFixed64(d);
    std::string res = se.toString();

    Serialize de(Serialize::DESERIALIZER, res);
    
    printf("After serialize and deserialize:\na: %x\tb: %lx\tc: %d\td: %ld\n", 
            de.readSFixed32(), de.readSFixed64(),
            de.readSFixed32(), de.readSFixed64());
}

void TestVar8() {
    Serialize se(Serialize::SERIALIZER);
    int8_t  a = -127;
    uint8_t b = 224;
    printf("Before serialize:\na: %d\tb: %d\n", a, b);
    se.writeVarInt8(a);
    se.writeVarUint8(b);
    std::string res = se.toString();

    Serialize de(Serialize::DESERIALIZER, res);
    
    printf("After serialize and deserialize:\na: %d\tb: %d\n", 
            de.readVarInt8(), de.readVarUint8());
}

void TestVar16to64() {
    Serialize se(Serialize::SERIALIZER);
    uint16_t a = 10;
    int16_t  b = -1;
    uint32_t c = 165536;
    int32_t  d = -16;
    uint64_t e = 0x00000000ffffffff;
    int64_t  f = 0x000000ffffffffff;

    printf("a: %d\tb: %d\tc: %d\td: %d\te: %lx\tf: %lx\n", 
            a, b, c, d, e, f);
    se.writeVarUint16(a);
    int last = 0;
    printf("sizeof a: %d\tsize after serialized: %d\n", sizeof a, se.toString().size() - last);
    last = se.toString().size();

    se.writeVarInt16(b);
    printf("sizeof b: %d\tsize after serialized: %d\n", sizeof b, se.toString().size() - last);
    last = se.toString().size();

    se.writeVarUint32(c);
    printf("sizeof c: %d\tsize after serialized: %d\n", sizeof c, se.toString().size() - last);
    last = se.toString().size();

    se.writeVarInt32(d);
    printf("sizeof d: %d\tsize after serialized: %d\n", sizeof d, se.toString().size() - last);
    last = se.toString().size();

    se.writeVarUint64(e);
    printf("sizeof e: %d\tsize after serialized: %d\n", sizeof e, se.toString().size() - last);
    last = se.toString().size();

    se.writeVarInt64(f);
    printf("sizeof f: %d\tsize after serialized: %d\n", sizeof f, se.toString().size() - last);
    last = se.toString().size();

    Serialize de(Serialize::DESERIALIZER, se.toString());
    printf("After serialize and deserialize:\na: %d\tb: %d\tc: %d\td: %d\te: %lx\tf: %lx\n", 
            de.readVarUint16(), de.readVarInt16(),
            de.readVarUint32(), de.readVarInt32(),
            de.readVarUint64(), de.readVarInt64());
}

void TestFloatAndDouble() {
    float f = 3.14;
    double d = -123.456;
    printf("f: %f\td: %lf\n", f, d);
    Serialize se(Serialize::SERIALIZER);
    se.writeFloat(f);
    se.writeDouble(d);

    Serialize de(Serialize::DESERIALIZER, se.toString());
    printf("after serialize and deserialize:\nf: %f\td: %lf\n", de.readFloat(), de.readDouble());
}

void TestString() {
    std::string str("hello world!");
    Serialize se(Serialize::SERIALIZER);
    se.writeString(str);
    printf("str: %s\n", str.c_str());
    
    Serialize de(Serialize::DESERIALIZER, se.toString());
    printf("after serialize and deserialize:\nstr: %s\n", de.readString().c_str());
}

int main(int argc, char** argv) {
    TestFixed32And64();
    TestSFixed32And64();
    TestVar8();
    TestVar16to64();
    TestFloatAndDouble();
    TestString();
    return 0;
}
