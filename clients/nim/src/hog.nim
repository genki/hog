import net, asyncnet, asyncdispatch, sequtils, tables, options

type
  Hog* = ref object
    socket: AsyncSocket
    commands: Table[string,int]

const TYPES: Table[string,int] = {
  "object": 2, "boolean": 3, "bool": 3, "int8": 4, "uint8": 5, "int16": 6,
  "uint16": 7, "int32": 8, "uint32": 9, "int64": 10, "uint64": 11, "float": 12,
  "time": 13, "short_text": 14, "text": 15, "long_text": 16, "delimit": 65,
  "unigram": 66, "bigram": 67, "trigram": 68, "mecab": 64
}.toTable

proc readUint8(hog:Hog): Future[uint8] {.async.} =
  let data = await hog.socket.recv(1)
  return data[0].uint8

proc readUint32BE(hog:Hog): Future[uint32] {.async.} =
  let data = await hog.socket.recv(4)
  return (data[0].uint32 shl 24) or (data[1].uint32 shl 16) or
    (data[2].uint32 shl 8) or data[3].uint32

proc uint32BE(value:uint32):seq[byte] =
  return @[byte((value shr 24) and 0xff), byte((value shr 16) and 0xff),
    byte((value shr 8) and 0xff), byte(value and 0xff)]

proc new*(_:type Hog): Hog =
  let hog = Hog()
  hog.socket = newAsyncSocket()
  return hog

proc connect*(hog:Hog, host:string, port:int) {.async.} =
  await hog.socket.connect(host, Port(port))
  let num = await hog.readUint8()
  hog.commands = initTable[string,int]()
  for id in 0..<int(num):
    let len = await hog.readUint32BE()
    let cmd = await hog.socket.recv(int(len))
    hog.commands[cmd] = id

proc close*(hog:Hog) =
  hog.socket.close()

proc ping*(hog:Hog): Future[bool] {.async.} =
  var buf = newSeq[byte](1)
  buf[0] = hog.commands["ping"].byte
  await hog.socket.send(addr(buf[0]), 1)
  let pong = await hog.socket.recv(1)
  return pong[0].uint8 == 1

proc command*(hog:Hog, cmd:string, column:string, tio:seq[string],
  keys_kvs:seq[string], kvs:bool = false
) {.async.} =
  let num = if kvs: keys_kvs.len shr 1 else: keys_kvs.len
  if (kvs and num*2 != keys_kvs.len):
    raise newException(ValueError, "Invalid number of keys/values")
  # prepare string stream
  var buf = newSeq[byte]()
  buf.add(hog.commands[cmd].byte)
  buf.add(uint32BE(column.len.uint32))
  buf.add(column.toOpenArrayByte(0, column.len-1))
  for t in tio:
    buf.add(TYPES[t].byte)
  buf.add(uint32BE(num.uint32))
  for kv in keys_kvs:
    buf.add(uint32BE(kv.len.uint32))
    buf.add(kv.toOpenArrayByte(0, kv.len-1))
  await hog.socket.send(addr(buf[0]), buf.len)

proc mget*(hog:Hog, column:string, tin:string, tout:string, keys:seq[string]
): Future[seq[Option[string]]] {.async.} =
  await hog.command("get", column, @[tin, tout], keys)
  result = newSeq[Option[string]](keys.len)
  for i in 0..<keys.len:
    let len = await hog.readUint32BE()
    if len > 0: result[i] = some(await hog.socket.recv(int(len)))
    else: result[i] = none(string)
