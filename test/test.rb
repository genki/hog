require "socket"
require "groonga"

def read(s, len)
    buf = ""
    while len > 0
        x = s.read len
        buf += x
        len -= x.length
    end
    buf
end

def write(s, buf)
    cur = 0
    while cur < buf.length
        x = s.send buf[cur..-1], 0
        cur += x
    end
end

cmds = {}
s = TCPSocket.open "localhost", 18618
n = s.getc.unpack('c')[0]
n.times do |i|
    len = read(s, 4).unpack('N')[0]
    name = read s, len
    cmds[name] = i
    puts "#{i}: #{name}"
end

# <cmd> {<len> <column id>} <type> <#kvs> [{<len> <key>} {<len> <value>}]...
column = "Foo.bar"
write s, [cmds["put"]].pack('c') # PUT
write s, [column.length].pack('N')
write s, column
write s, [Groonga::Type::TEXT].pack('c')
write s, [1].pack('N') # #kvs
write s, ["hello".length].pack('N') # key len
write s, "hello"
write s, ["world".length].pack('N') # value len
write s, "world"

# <cmd> {<len> <column id>} <types> <#keys> [{<len> <key>}]...
write s, [cmds["get"]].pack('c') # GET
write s, [column.length].pack('N')
write s, column
write s, [Groonga::Type::SHORT_TEXT].pack('c')
write s, [Groonga::Type::TEXT].pack('c')
write s, [1].pack('N') # #kvs
write s, ["hello".length].pack('N') # key len
write s, "hello"
len = read(s, 4).unpack('N').first
puts read(s, len)
