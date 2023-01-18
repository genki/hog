import unittest
import os, asyncdispatch
import osproc, streams
import sequtils, options
import hog

const PORT = 18618

suite "Hog":
  os.removeDir("tmp/groonga")
  discard ["tmp","tmp/groonga"].map(existsOrCreateDir)
  # create fixture file to load
  let f = open("tmp/groonga/setup.grn", fmWrite)
  f.write("""
  table_create --name User --flags TABLE_HASH_KEY --key_type ShortText
  column_create --table User --name name --type ShortText
  column_create --table User --name age --type Int32
  load --table User --values [
    ["_key", "name", "age"],
    ["alice", "Alice", 20],
    ["bob", "Bob", 21],
    ["carol", "Carol", 22]
  ]
  table_create --name Terms --flags TABLE_PAT_KEY --key_type ShortText \
    --default_tokenizer TokenBigram
  index_column --table Terms --name User.name --source User.name
  quit
  """)
  f.close()

  # init groonga db
  discard execProcess("docker", args=["run", "--rm", "-v",
    getCurrentDir() / "tmp/groonga:/tmp", "groonga/groonga:12.0.8-alpine",
    "-n", "--file", "/tmp/setup.grn", "/tmp/test"], options={poUsePath})

  # run Hog server
  echo "start hog server"
  let pHog = startProcess("docker", args=["run", "--rm", "-v",
    getCurrentDir() / "tmp/groonga:/tmp", "-p", $PORT & ":" & $PORT,
    "s21g/hog:0.9.0", "/tmp/test"], options={poUsePath})
  defer:
    echo "stop hog server"
    pHog.terminate()

  # wait for Hog server to start and connect
  echo "wait for hog server to start"
  let hog = Hog.new()
  let connect = proc() {.async.} =
    while true:
      try:
        await hog.connect("127.0.0.1", PORT)
        break
      except OSError:
        await sleepAsync(100)
  waitFor connect()
  defer: hog.close()
  echo "hog server started"

  test "can get instance and ping":
    check hog is Hog
    check waitFor hog.ping()

  test "can mget records":
    let keys = @["alice", "bob", "carol"]
    let names = waitFor hog.mget("User.name", "short_text", "short_text", keys)
    check names == @[some("Alice"), some("Bob"), some("Carol")]

  #test "can add records":
  #  check waitFor hog.mput("User.name", "short_text", "short_text",
  #    @["dave", "Dave", "eve", "Eve"])
  #  let names = waitFor hog.mget("User.name", "short_text", "short_text",
  #    @["dave", "eve"])
  #  check names == @[some("Dave"), some("Eve")]
  #  var buf = newSeq[byte](4)
  #  buf[3] = 30
  #  check waitFor hog.store("User.name", "short_text", "dove",
  #    @[("uint32", "age", buf)])
  #  let age = waitFor hog.get("User.age", "short_text", "uint32", "dove")
  #  case age:
  #  of Some(@x):
  #    check newStringStream(x).readUInt32() == 30
  #  of None():
  #    check false
