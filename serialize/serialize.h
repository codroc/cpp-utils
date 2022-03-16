#ifndef SERIALIZE_H
#define SERIALIZE_H
#include <stdint.h>
#include <memory>
#include <string>

// 模仿 google protobuf 的序列化规则
// Tag [Length] Value
// TLV: string
// TV: varint, fixed32, fiexd64
//
class Serialize {
public:
    using ptr = std::shared_ptr<Serialize>;
    enum type {
        SERIALIZER,
        DESERIALIZER
    };
    // 对应于 serializer
    Serialize(type t, size_t base_size = 4096);
    // 对应于 deserializer
    Serialize(type t, const std::string& bytearray);
    ~Serialize();

    // write
    // 序列化从 p 开始的 4 个字节到 
    // 序列化失败返回 -1，成功返回 0
    int writeBit32(void* p);
    int writeBit64(void* p);
    int writeFixed32(uint32_t value);
    int writeFixed64(uint64_t value);
    int writeSFixed32(int32_t value);
    int writeSFixed64(int64_t value);

    int writeVarInt8(int8_t value);
    int writeVarUint8(uint8_t value);
    int writeVarInt16(int16_t value);
    int writeVarUint16(uint16_t value);
    int writeVarInt32(int32_t value);
    int writeVarUint32(uint32_t value);
    int writeVarInt64(int64_t value);
    int writeVarUint64(uint64_t value);

    int writeFloat(float value);
    int writeDouble(double value);

    int writeString(const std::string& str);

    // read
    int readBit32(void* p);
    int readBit64(void* p);
    uint32_t readFixed32();
    uint64_t readFixed64();
    int32_t readSFixed32();
    int64_t readSFixed64();

    int readVarInt8(int8_t* value);
    int readVarUint8(uint8_t* value);
    int readVarInt16(int16_t* value);
    int readVarUint16(uint16_t* value);
    int readVarInt32(int32_t* value);
    int readVarUint32(uint32_t* value);
    int readVarInt64(int64_t* value);
    int readVarUint64(uint64_t* value);

    float readFloat();
    double readDouble();

    std::string readString();

    // 写到 Node 中去
    void write(void* buf, size_t len);

    // 从 Node 中读出来
    void read(void* buf, size_t len);

    // 把 Node 中的所有信息放到 string 中返回
    std::string toString();
private:
    struct Node {
        Node(size_t base_size);
        ~Node();

        // 对于拷贝函数来说，因为这里的拷贝涉及深浅拷贝，也不知道后面用不用得到，所以先 delete 掉，后面用到的时候再说
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        char *str{nullptr};
        size_t base_size{0};
        Node* next{nullptr};
    };

    type _type;
    size_t _base_size{4096}; // 默认一个 Node 占用一页
    Node* _head{nullptr}; // Node 链表头
    Node* _tail{nullptr}; // Node 链表尾
    char*  _cur{nullptr};
    uint32_t _capacity{0}; // 序列化容器 当前容量
    uint32_t _size{0}; // 已经写了多少个字节

    // deserializer
    char* _buf{nullptr};
    size_t _len{0};
    uint32_t _readed{0}; // 对于 deserializer 来说已经读了的字节数
};

#endif
