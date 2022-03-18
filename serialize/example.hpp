#include "serialize.h"
#include <string>
#include <iostream>
struct Person {
    Person(const std::string& n, const std::string& s, uint8_t a, uint32_t p)
        : name(n),
          sex(s),
          age(a),
          property(p)
    {}
    std::string name;
    std::string sex;
    uint8_t age;
    uint32_t property;

    std::string serializeToString();
    static Person deserializeToPerson(const std::string& str);
};

std::string Person::serializeToString() {
    Serialize se(Serialize::SERIALIZER);
    se.writeString(name);
    se.writeString(sex);
    se.writeVarUint8(age);
    se.writeVarUint32(property);
    return se.toString();
}

Person Person::deserializeToPerson(const std::string& str) {
    Serialize de{Serialize::DESERIALIZER, str};
    return {
        de.readString(),
        de.readString(),
        de.readVarUint8(),
        de.readVarUint32()
    };
}

std::ostream& operator<<(std::ostream& os, const Person& p) {
    return os << p.name << " " << p.sex << " "
        << (int)p.age << " " << p.property << std::endl;
}

bool operator==(const Person& rhs, const Person& lhs) {
    return rhs.name == lhs.name && 
        rhs.sex == lhs.sex && 
        rhs.age == lhs.age && 
        rhs.property == lhs.property;
}
