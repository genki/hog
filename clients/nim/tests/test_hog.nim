import unittest
import os, asyncdispatch
import std/osproc
import sequtils, options
import hog

suite "Hog":
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
  let pHog = startProcess("docker", args=["run", "--rm", "-v",
    getCurrentDir() / "tmp/groonga:/tmp", "-p", "18618:18618",
    "s21g/hog:0.9.0", "/tmp/test"], options={poUsePath})

  # wait for Hog server to start and connect
  let hog = Hog.new()
  let connect = proc() {.async.} =
    while true:
      try:
        await hog.connect("127.0.0.1", 18618)
        break
      except OSError:
        sleep(100)
  waitFor connect()

  test "can get instance and ping":
    check hog is Hog
    check waitFor hog.ping()

  test "can mget records":
    let keys = @["alice", "bob", "carol"]
    let names = waitFor hog.mget("User.name", "short_text", "short_text", keys)
    check names == @[some("Alice"), some("Bob"), some("Carol")]

  hog.close()
  pHog.terminate()
