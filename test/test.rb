require "socket"

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
def submit_w(s, column, cmd, types, kvs)
  submit s, column, cmd, types, kvs
  read s, 1
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
    write s, [cmds["ping"]].pack('c') # PUT
    pong = read(s, 1).unpack('c').first
    #puts "ping -> #{pong}"

    #types = [Groonga::Type::SHORT_TEXT, Groonga::Type::TEXT]
    types = [14, 15]

    now = Time.now
    1000.times do
      submit_w s, "Foo.bar", cmds["put"], types, "hello" => "world"
      submit s, "Foo.bar", cmds["get"], types, ["hello"]
      len = read(s, 4).unpack('N').first
      read(s, len)
      submit s, "Foo.bar", cmds["exist"], [types[0]], ["hello", "foo"]
      read(s, 2).unpack('c*')
      submit_w s, "Foo", cmds["del"], [types[0]], ["hello"]
    end
    puts "[#{idx}] %.2f s" % (Time.now - now)

    submit_w s, "Foo.bar", cmds["put"], types, "hello" => "world"
    submit_w s, "Foo.bar", cmds["put"], types, "foo" => "bar!"
    puts "** each"
    #submit s, "Foo", cmds["each"], 14, 0, -1
    write s, [cmds["each"]].pack('c') # PUT
    write s, ["Foo.bar".length].pack('N')
    write s, "Foo.bar"
    write s, [14].pack('c*')
    write s, [1].pack('N') # 1 col
    write s, [15].pack('c*') # type text
    write s, ["bar".length].pack('N')
    write s, "bar" # key bar
    write s, [0, -1].pack('N*') # #kvs
    loop do
      blen = read(s, 4).unpack('N').first
      break if blen == 0xFFFFFFFF
      key = read(s, blen)
      blen = read(s, 4).unpack('N').first
      value = read(s, blen)
      puts "#{key} => #{value}"
    end

    #write s, [cmds["fin"]].pack('c')
    #s.shutdown
    s.close
  end
  ths << th
end
ths.each &:join
