- required: must exist on read, must be set on write
- optional: may or may not be set, entirely optional
- default: may not exist on read, always written (unless it is a null pointer)

```bash
thrift -r --gen go tutorial.thrift
thrift -r --gen cpp tutorial.thrift
```

# type
|thrift      |C++                  |Go          |
|--------------|-------------------|------------|
|bool          |bool               |bool        |
|byte          |int8_t             |int8        |
|i16           |int16_t            |int16       |
|i32           |int32_t            |int32       |
|i64           |int64_t            |int64       |
|double        |double             |float64     |
|string        |std::string        |string      |
|binary        |std::string        |[]byte      |
|`list<t1>`    |`std::vector<t1>`  |[]t1        |
|`set<t1>`     |`std::set<t1>`     |[]t1        |
|`map<t1, t2>` |`std::map<t1, t2>` |`map[t1]t2` |

- 如果 t1, t2 是复合类型, Go 里面会成为指针类型

Note the absence of unsigned integer types. This is due to the fact that there are no native unsigned integer types in
many programming languages.

## Typedefs
Thrift supports C/C++ style typedefs.
```c++
typedef i32 MyInteger
typedef Tweet ReTweet
```

## Enums
```thrift
enum ContentType {
  Text = 1,
  Img = 2,
  Video = 3,
}
```

## Constants
```C++
const i32 INT_CONST = 1234;

const map<string,string> MAP_CONST = {"hello": "world", "goodnight": "moon"}

const list<ContentType> List_Const = [ContentType.Text, ContentType.Video]
```

# 序列化及反序列化
[Is safe to rename a field in Thrift IDL?](
    https://stackoverflow.com/questions/52882370/is-safe-to-rename-a-field-in-thrift-idl)
Yes it is 100% safe. Thrift only deals with field IDs internally. The names of a struct as well as method argument names
are used to generate field names in the generated code only. They do not even go over the wire.

Furthermore, it is a recommended way to deprecate fields. Even in the case where a field is fully retired, one should
comment it out but leave it in the IDL to prevent the numeric field ID from being accidentally reused.

The only place where names are used as names is with service method calls. Methods do not have numeric identifiers, in
that case the name is used. Changing the name actually declares a new method.

# Generated Code
[Thrift: The Missing Guide](https://diwakergupta.github.io/thrift-missing-guide)

The Thrift Network Stack
```graph
+-------------------------------------------+
| cGRE                                      |
| Server                                    |
| (single-threaded, event-driven etc)       |
+-------------------------------------------+
| cBLU                                      |
| Processor                                 |
| (compiler generated)                      |
+-------------------------------------------+
| cGRE                                      |
| Protocol                                  |
| (JSON, compact etc)                       |
+-------------------------------------------+
| cGRE                                      |
| Transport                                 |
| (raw TCP, HTTP etc)                       |
+-------------------------------------------+
```

## Transport
The Transport layer provides a simple abstraction for reading/writing from/to the network. This enables Thrift to
decouple the underlying transport from the rest of the system (serialization/deserialization, for instance).
Here are some of the methods exposed by the Transport interface:

- open
- close
- read
- write
- flush

In addition to the Transport interface above, Thrift also uses a ServerTransport interface used to accept or create
rimitive transport objects.
As the name suggest, ServerTransport is used mainly on the server side to create new Transport objects for incoming
connections.

- open
- listen
- accept
- close

Here are some of the transports available for majority of the Thrift-supported languages:

- file: read/write to/from a file on disk
- http: as the name suggests

## Protocol
The Protocol abstraction defines a mechanism to map in-memory data structures to a wire-format.
In other words, a protocol specifies how datatypes use the underlying Transport to encode/decode themselves.
Thus the protocol implementation governs the encoding scheme and is responsible for (de)serialization.
Some examples of protocols in this sense include JSON, XML, plain text, compact binary etc.

Here is the Protocol interface:
```C++
writeMessageBegin(name, type, seq)
writeMessageEnd()
writeStructBegin(name)
writeStructEnd()
writeFieldBegin(name, type, id)
writeFieldEnd()
writeFieldStop()
writeMapBegin(ktype, vtype, size)
writeMapEnd()
writeListBegin(etype, size)
writeListEnd()
writeSetBegin(etype, size)
writeSetEnd()
writeBool(bool)
writeByte(byte)
writeI16(i16)
writeI32(i32)
writeI64(i64)
writeDouble(double)
writeString(string)
...
// 对应的read api
```

Thrift Protocols are stream oriented by design. There is no need for any explicit framing. For instance, it is not
necessary to know the length of a string or the number of items in a list before we start serializing them.
Here are some of the protocols available for majority of the Thrift-supported languages:

- binary: Fairly simple binary encoding — the length and type of a field are encoded as bytes followed by the actual
    value of the field.
- compact: Described in THRIFT-110
- json:

## Processor
A Processor encapsulates the ability to read data from input streams and write to output streams. The input and output
streams are represented by Protocol objects. The Processor interface is extremely simple:
```thrift
interface TProcessor {
    bool process(TProtocol in, TProtocol out) throws TException
}
```
Service-specific processor implementations are generated by the compiler.
The Processor essentially reads data from the wire (using the input protocol), delegates processing to the handler
(implemented by the user) and writes the response over the wire (using the output protocol).

## Server
A Server pulls together all of the various features described above:

- Create a transport
- Create input/output protocols for the transport
- Create a processor based on the input/output protocols
- Wait for incoming connections and hand them off to the processor

