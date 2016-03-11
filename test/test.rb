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

def submit(s, column, cmd, types, kvs)
    write s, [cmd].pack('c') # PUT
    write s, [column.length].pack('N')
    write s, column
    write s, types.pack('c*')
    write s, [kvs.size].pack('N') # #kvs
    if kvs.is_a?(Hash)
        kvs.each_pair do |k,v|
            write s, [k.length].pack('N') # key len
            write s, k
            write s, [v.length].pack('N') # value len
            write s, v
        end
    else
        kvs.each do |k,v|
            write s, [k.length].pack('N') # key len
            write s, k
        end
    end
end

ths = []
1.times do |idx|
    th = Thread.new do
        cmds = {}
        s = TCPSocket.open "localhost", 18618
        n = s.getc.unpack('c')[0]
        n.times do |i|
            len = read(s, 4).unpack('N')[0]
            name = read s, len
            cmds[name] = i
        end

        # ping
        #write s, [cmds["ping"]].pack('c') # PUT
        #pong = read(s, 1).unpack('c').first
        #puts "ping -> #{pong}"

        types = [Groonga::Type::SHORT_TEXT, Groonga::Type::TEXT]
        #submit s, "Foo.bar", cmds["put"], types, "hello" => "world"

        now = Time.now
        2000.times do
            submit s, "Foo.bar", cmds["get"], types, ["hello"]
            len = read(s, 4).unpack('N').first
            read(s, len)
        end
        puts "[#{idx}] %.2f s" % (Time.now - now)

        #submit s, "Foo.bar", cmds["exist"], [types[0]], ["hello", "foo"]
        #read(s, 2).unpack('c*').each{|n| puts "exist: #{n}"}

        #submit s, "Foo.bar", cmds["del"], [types[0]], ["hello"]
        #submit s, "Foo.bar", cmds["get"], types, ["hello"]
        #len = read(s, 4).unpack('N').first
        #puts len
        #write s, [cmds["fin"]].pack('c')
        #s.shutdown
        s.close
    end
    ths << th
end
ths.each &:join
