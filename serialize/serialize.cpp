#include "serialize.h"
#include <assert.h>
#include <byteswap.h>

#include "flog.h"

bool isLittleEndian();
static bool g_isLittleEndian = isLittleEndian();

bool isLittleEndian() {
    unsigned short a = 0x1234;
    char* p = (char*) &a;
    if (*((uint8_t*)&p[0]) == 0x12) return false;
    return true;
}

Serialize::Node::Node(size_t base_size)
{
    this->base_size = base_size;
    str = (char*) ::malloc(base_size);
    if (str == nullptr) {
        LOG_ERROR << "OOM!\n";
        exit(1);
    }
}

Serialize::Node::~Node() {
    if (str) {
        ::free(str);
        str = nullptr;
    }
}

Serialize::Serialize(Serialize::type t, size_t base_size)
    : _type(t),
      _base_size(base_size),
      _head(new Node(_base_size)),
      _tail(_head),
      _capacity(_base_size)
{
    _cur = _head->str; // 能否保证 Node 以及构造完成？
}

Serialize::Serialize(Serialize::type t, const std::string& bytearray) {
    assert(t == DESERIALIZER && bytearray.size() > 0);
    _buf = (char*) ::malloc(bytearray.size());
    if (_buf == nullptr) {
        LOG_ERROR << "OOM\n";
        exit(1);
    }
    _len = bytearray.size();
    ::memmove(_buf, &bytearray[0], _len);
}

Serialize::~Serialize() {
    // 释放完 Node 链表
    while(_head) {
        Node* tmp = _head;
        _head = _head->next;
        delete tmp;
    }
    // 释放 反序列化时 malloc 出来的 buf
    if (_buf) {
        ::free(_buf);
        _buf = nullptr;
    }
}

// 写到 Node 中去
void Serialize::write(void* buf, size_t len) {
    assert(_type == SERIALIZER);
    if (_capacity > len) {
        ::memmove(_cur, buf, len);
        _cur += len;
        _capacity -= len;
        _size += len;
    } else {
        size_t remain = len - _capacity;
        char* p = (char*) buf;
        ::memmove(_cur, p, _capacity);
        p += _capacity;
        _size += _capacity;
        _capacity = 0;

        while (remain > _base_size) {
            _tail->next = new Node(_base_size);
            _tail = _tail->next;
            
            ::memmove(_tail->str, p, _base_size);
            p += _base_size;
            _size += _base_size;
            remain -= _base_size;
        }

        _tail->next = new Node(_base_size);
        _tail = _tail->next;
             
        ::memmove(_tail->str, p, remain);
        _cur += remain;
        _size += remain;
        _capacity = _base_size - remain;
    }
}

// 从 Node 中读出来
void Serialize::read(void* buf, size_t len) {
    assert(_type == DESERIALIZER);
    assert(len + _readed <= _len);
    // 对于反序列化的一端，从 _buf 起读出 len 个字节放到 buf 中去
    char* p = _buf + _readed;
    ::memmove(buf, p, len);
    _readed += len;
}

// write
// 序列化从 p 开始的 4 个字节到 
// 序列化失败返回 -1，成功返回 0
// fixed32 可以是 int32 uint32 float
int Serialize::writeBit32(void* p) {
    uint32_t v;
    ::memcpy(&v, p, sizeof v);
    if (g_isLittleEndian) {
        v = ::bswap_32(v);
    }
    write(&v, sizeof v);
    return 0;
}

int Serialize::writeBit64(void* p) {
    uint64_t v;
    ::memcpy(&v, p, sizeof v);
    if (g_isLittleEndian) {
        v = ::bswap_64(v);
    }
    write(&v, sizeof v);
    return 0;
}

int Serialize::writeFixed32(uint32_t value) {
    writeBit32(&value);
}

int Serialize::writeFixed64(uint64_t value) {
    writeBit64(&value);
}

int Serialize::writeSFixed32(int32_t value) {
}

int Serialize::writeSFixed64(int64_t value) {
}

int Serialize::writeVarInt8(int8_t value) {
}

int Serialize::writeVarUint8(uint8_t value) {
}

int Serialize::writeVarInt16(int16_t value) {
}

int Serialize::writeVarUint16(uint16_t value) {
}

int Serialize::writeVarInt32(int32_t value) {
}

int Serialize::writeVarUint32(uint32_t value) {
}

int Serialize::writeVarInt64(int64_t value) {
}

int Serialize::writeVarUint64(uint64_t value) {
}

int Serialize::writeFloat(float value) {
}

int Serialize::writeDouble(double value) {
}

int Serialize::writeString(const std::string& str) {
}

// read
int Serialize::readBit32(void* p) {
    read(p, sizeof(uint32_t));
    if (isLittleEndian) {
        *(uint32_t*)p = bswap_32(*(uint32_t*)p);
    }
    return 0;
}

int Serialize::readBit64(void* p) {
    read(p, sizeof(uint64_t));
    if (isLittleEndian) {
        *(uint64_t*)p = bswap_64(*(uint64_t*)p);
    }
    return 0;
}

uint32_t Serialize::readFixed32() {
    uint32_t v;
    readBit32(&v);
    return v;
}

uint64_t Serialize::readFixed64() {
    uint64_t v;
    readBit64(&v);
    return v;
}

int32_t Serialize::readSFixed32() {
}

int64_t Serialize::readSFixed64() {
}


int Serialize::readVarInt8(int8_t* value) {
}

int Serialize::readVarUint8(uint8_t* value) {
}

int Serialize::readVarInt16(int16_t* value) {
}

int Serialize::readVarUint16(uint16_t* value) {
}

int Serialize::readVarInt32(int32_t* value) {
}

int Serialize::readVarUint32(uint32_t* value) {
}

int Serialize::readVarInt64(int64_t* value) {
}

int Serialize::readVarUint64(uint64_t* value) {
}

float Serialize::readFloat() {
}

double Serialize::readDouble() {
}

std::string readString() {
}

std::string Serialize::toString() {
    std::string ret;
    Node* tmp = _head;
    uint32_t s = _size;
    while (tmp && s > _base_size) {
        ret.append(tmp->str, _base_size);
        s -= _base_size;
        tmp = tmp->next;
    }
    ret.append(tmp->str, s);
    return ret;
}
