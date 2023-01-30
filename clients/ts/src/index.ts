/**
 * Client for hog server.
 * This class is used to communicate with the hog server via TCP sockets.
 */
import { Socket } from "net";

const TYPES: {[key:string]:number} = {
  object: 2, boolean: 3, bool: 3, int8: 4, uint8: 5, int16: 6, uint16: 7,
  int32: 8, uint32: 9, int64: 10, uint64: 11, float: 12, time: 13,
  short_text: 14, text: 15, long_text: 16, delimit: 65, unigram: 66,
  bigram: 67, trigram: 68, mecab: 64,
};

export default class Hog {
  host: string;
  port: number;
  socket: Socket;
  chunks: Buffer[] = [];
  cursor: number = 0;
  wait: (() => void) | null = null;
  commands: { [key:string]: number } = {};

  private constructor(host: string, port: number) {
    this.host = host;
    this.port = port;
    this.socket = new Socket();
  }

  static async connect(host: string, port: number): Promise<Hog> {
    var hog = new Hog(host, port);
    hog.socket.on("data", (data:Buffer) => hog.onData(data));
    return new Promise((resolve, reject) => {
      // connect to hog server with retry upto 3 times with 1 second interval
      hog.socket.on("error", reject);
      hog.socket.connect(port, host, async () => {
        await hog.setup();
        resolve(hog);
      });
    });
  }

  close() {
    this.socket.end();
  }

  async ping(): Promise<boolean> {
    var buf = Buffer.alloc(1);
    buf.writeUInt8(this.commands["ping"], 0);
    await this.send(buf);
    return (await this.pop(1)).readUInt8(0) == 1;
  }

  async mget(column:string, tin:string, tout:string, keys:Buffer[]
  ): Promise<(Buffer|null)[]> {
    await this.command("get", column, [tin, tout], keys);
    return new Promise(async (resolve) => {
      let values: (Buffer|null)[] = [];
      for(var i = 0; i < keys.length; i++) {
        var size = (await this.pop(4)).readUInt32BE(0);
        values.push(size > 0 ? await this.pop(size) : null);
      }
      resolve(values);
    });
  }
  async get(column:string, tin:string, tout:string, key:Buffer
  ): Promise<Buffer|null> {
    return (await this.mget(column, tin, tout, [key]))[0];
  }

  async mput(column:string, tin:string, tout:string, kvs:Buffer[]
  ): Promise<boolean> {
    await this.command("put", column, [tin, tout], kvs, true);
    return (await this.pop(1)).readUInt8(0) == 1;
  }
  async put(column:string, tin:string, tout:string, key:Buffer, value:Buffer
  ): Promise<boolean> {
    return await this.mput(column, tin, tout, [key, value]);
  }

  async mset(column:string, tin:string, tout:string, kvs:Buffer[]
  ): Promise<boolean> {
    await this.command("set", column, [tin, tout], kvs, true);
    return (await this.pop(1)).readUInt8(0) == 1;
  }
  async set(column:string, tin:string, tout:string, key:Buffer, value:Buffer
  ): Promise<boolean> {
    return await this.mset(column, tin, tout, [key, value]);
  }

  async mdelete(column:string, tin:string, keys:Buffer[]): Promise<boolean> {
    await this.command("delete", column, [tin], keys);
    return (await this.pop(1)).readUInt8(0) == 1;
  }
  async delete(column:string, tin:string, key:Buffer): Promise<boolean> {
    return await this.mdelete(column, tin, [key]);
  }

  async count(column:string): Promise<number> {
    await this.command("count", column);
    return (await this.pop(4)).readUInt32BE(0);
  }

  async exec(cmd:string): Promise<string> {
    await this.command("exec", cmd);
    let len = (await this.pop(4)).readUInt32BE(0);
    return JSON.parse((await this.pop(len)).toString());
  }

  // returns [total, count, [keys]]
  async query(column:string, tin:string, query:string,
    sortby:string = "_id", offset:number = 0, limit:number = -1
  ): Promise<[number,number,Buffer[]]> {
    let bufs:Buffer[] = await this.bufsFor("query", column, [tin]);
    bufs.push(numAsBuffer(Buffer.byteLength(query), 4));
    bufs.push(Buffer.from(query));
    bufs.push(numAsBuffer(Buffer.byteLength(sortby), 4));
    bufs.push(Buffer.from(sortby));
    bufs.push(intAsBuffer(offset, 4));
    bufs.push(intAsBuffer(limit, 4));
    await this.send(Buffer.concat(bufs));
    var result = await this.pop(8);
    var total = result.readUInt32BE(0);
    var count = result.readUInt32BE(4);
    return new Promise(async (resolve) => {
      let keys: Buffer[] = [];
      for(var i = 0; i < count; i++) {
        var len = (await this.pop(4)).readUInt32BE(0);
        keys.push(await this.pop(len));
      }
      resolve([total, count, keys]);
    });
  }

  async store(table:string, tin:string, key:Buffer,
    values:[string, string, Buffer][]
  ): Promise<boolean> {
    let bufs:Buffer[] = await this.bufsFor("store", table, [tin]);
    bufs.push(numAsBuffer(key.length, 4));
    bufs.push(key);
    bufs.push(numAsBuffer(values.length, 4));
    for (const [tout, column, value] of values) {
      bufs.push(numAsBuffer(TYPES[tout], 1));
      bufs.push(numAsBuffer(Buffer.byteLength(column), 4));
      bufs.push(Buffer.from(column));
      bufs.push(numAsBuffer(value.length, 4));
      bufs.push(value);
    }
    await this.send(Buffer.concat(bufs));
    return (await this.pop(1)).readUInt8(0) == 1;
  }

  private async command(
    cmd: string, column_cmd:string, tio:string[] = [],
    keys_kvs:Buffer[] = [], kvs:boolean = false
  ): Promise<void> {
    let num = kvs ? keys_kvs.length / 2 : keys_kvs.length;
    if (kvs && num * 2 != keys_kvs.length) {
      throw new Error("keys and values must be in pairs");
    }
    let bufs:Buffer[] = await this.bufsFor(cmd, column_cmd, tio);
    if (keys_kvs.length > null) {
      bufs.push(numAsBuffer(num, 4));
      for (const kv of keys_kvs) {
        bufs.push(numAsBuffer(kv.length, 4));
        bufs.push(kv);
      }
    }
    await this.send(Buffer.concat(bufs));
  }

  private async bufsFor(cmd: string, column: string, tio: string[]
  ): Promise<Buffer[]> {
    var bufs:Buffer[] = [];
    bufs.push(numAsBuffer(this.commands[cmd], 1));
    bufs.push(numAsBuffer(Buffer.byteLength(column), 4));
    bufs.push(Buffer.from(column));
    for(const t of tio) {
      bufs.push(numAsBuffer(TYPES[t], 1));
    }
    return bufs;
  }

  private async setup() {
    var numCmds = (await this.pop(1)).readUInt8(0);
    for (var i = 0; i < numCmds; i++) {
      var len = (await this.pop(4)).readUInt32BE(0);
      var cmd = (await this.pop(len)).toString();
      this.commands[cmd] = i;
    }
  }

  private onData(data: Buffer) {
    this.chunks.push(data);
    if (this.wait != null) {
      this.wait();
    }
  }

  private async send(buf: Buffer) {
    return new Promise((resolve, reject) => {
      this.socket.write(buf, resolve);
    });
  }

  private async pop(size: number): Promise<Buffer> {
    var chunk = this.chunks[0];
    if(chunk == null) {
      return new Promise((resolve, reject) => {
        this.wait = () => {
          this.wait = null;
          resolve(this.pop(size));
        };
      });
    }
    if (chunk.length < size + this.cursor) {
      // chunk is not enough
      var head = chunk.slice(this.cursor);
      this.chunks.shift();
      this.cursor = 0;
      return Buffer.concat([head, await this.pop(size - head.length)]);
    }else{
      // chunk is enough
      var buf = chunk.slice(this.cursor, this.cursor + size);
      if (chunk.length == size + this.cursor) {
        this.chunks.shift();
        this.cursor = 0;
      }else{
        this.cursor += size;
      }
      return buf;
    }
  }
}

function numAsBuffer(num: number, size: number): Buffer {
  var buf = Buffer.alloc(size);
  switch (size) {
    case 1: buf.writeUInt8(num, 0); break;
    case 2: buf.writeUInt16BE(num, 0); break;
    case 4: buf.writeUInt32BE(num, 0); break;
    case 8: buf.writeBigUInt64BE(BigInt(num), 0); break;
  }
  return buf;
}

function intAsBuffer(num: number, size: number): Buffer {
  var buf = Buffer.alloc(size);
  switch (size) {
    case 1: buf.writeInt8(num, 0); break;
    case 2: buf.writeInt16BE(num, 0); break;
    case 4: buf.writeInt32BE(num, 0); break;
    case 8: buf.writeBigInt64BE(BigInt(num), 0); break;
  }
  return buf;
}
