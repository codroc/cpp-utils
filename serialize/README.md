# Serialize

为什么需要将数据序列化？

- 方便数据存储
- 方便数据传递



​	对于数据存储来说，一般进程通过直接访问内存来读写数据。例如初始化一个对象，就需要对对象中的各个变量进行读写。如果需要持久化一个对象怎么办，当然是写入磁盘，那么怎么把一个存储在内存中的对象写入到磁盘中呢？我们既要知道变量的类型，又要知道变量的值。

​	再从数据传递的角度来说，数据往往需要通过网络传递给另一台计算机，而我们知道在链路层上传输的数据都是字节流，那么如何把一个存储在内存中的对象变成字节流传递给网络上的其他计算机呢？

​	这一系列问题都可以用序列化和反序列化来解决。

# Google ProtoBuf

​	Protocol Buffers 是一种开源跨平台的序列化数据结构的协议。其对于存储数据或在网络上进行通信的程序是很有用的。

​	这里需要掌握的是，ProtoBuf 是如何编码以及序列化数据的。可以看这篇文章 [深入 ProtoBuf - 编码](https://www.jianshu.com/p/73c9ed3a4877)。

​	我参考 ProtoBuf 设计了一套简易的 序列化工具。它支持 LV 格式，以及 Varint 编码，我目前还没搞清楚 Tag 的作用，其实用 LV 格式以及足够我目前使用了。。。。。

​	Serialize 库总共支持 4 种类型：`string`、`varint` 、`fixed32` 、`fixed64`

- string：任意字符串
- varint：int8_t、int16_t、int32_t、int64_t、uint8_t、uint16_t、uint32_t、uint64_t
- fixed32：int32_t、uint32_t、float
- fixed64：int64_t、uint64_t、double

# Interface

参考 ProtoBuf，用户自己先定义一个类，然后根据类内成员变量按简单的规则定制接口。

- 对象——>序列化字符串，序列化字符串——>对象
- 对象——>文件，文件——>对象

简单的规则如下：

1. 仅支持 `u/sint8~64_t`，`string`，`float`，`double`，`bool` 等基本类型
2. 按照成员变量的声明顺序进行序列化与反序列化

#### 对象<——>字符串

​	，并提供 `serializeToString` 和 `deserializeToPerson` 接口来实现 对象<——>字符串 之间的 序列化与反序列化：

```C++
// example.hpp
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
    static Person deserializeToPerson(const std::string& serialized_string);
};
```

**其中 `serializeToString` 接口用于将对象序列化成一个字符串 *serialized_string*，`deserializeToPerson` 接口用于将序列化后的字符串 *serialized_string* 反序列**化成一个对象。

这两个接口的实现很简单：

`serializeToString` 只需要一次对成员变量序列化：

```C++
std::string Person::serializeToString() {
    Serialize se(Serialize::SERIALIZER);
    // 依次序列化
    se.writeString(name);
    se.writeString(sex);
    se.writeVarUint8(age);
    se.writeVarUint32(property);
    return se.toString();
}
```

`deserializeToPerson` 只需要一次反序列化就可以了：

```C++
Person Person::deserializeToPerson(const std::string& str) {
    Serialize de{Serialize::DESERIALIZER, str};
    return {
        de.readString(),
        de.readString(),
        de.readVarUint8(),
        de.readVarUint32()
    };
}
```

#### 对象<——>文件

包含两个固定的接口：`serializeToFile`，`deserializeFromFile` 。一般情况下对于不同的类型只需要修改下 `deserializeFromFile` 的返回类型就可以了。

```C++
bool Person::serializeToFile(const std::string& filepath) {
    if (!Serialize::toFile(filepath, serializeToString())) return false;
    return true;
}
Person Person::deserializeFromFile(const std::string& filepath) {
    return deserializeToPerson(Serialize::fromFile(filepath));
}
```

