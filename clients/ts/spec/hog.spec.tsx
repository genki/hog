import { describe, it, expect, beforeAll, afterAll } from 'vitest';
import Hog from '../src/index';
import { spawn, execSync } from 'child_process';
import fs from 'fs';

let toBuf = (x) => Buffer.from(x, 'utf-8');

describe('[Hog]', () => {
  // store the return value of spawn
  let hog: Hog;

  beforeAll(async () => {
    prepareGrnDir();
    await setupFixture();
    await startHog();

    let retry = 20;
    while (retry > 0) {
      try {
        hog = await Hog.connect('127.0.0.1', 18618);
        break;
      } catch (e) {
        retry--;
        await new Promise((resolve) => setTimeout(resolve, 100));
      }
    }
  });

  afterAll(async () => {
    stopHog();
  });

  it('should be able to create instance', async () => {
    expect(hog).toBeInstanceOf(Hog);
  });

  it('should be respond to ping', async () => {
    expect(await hog.ping()).toBe(true);
  });

  it('counts records', async () => {
    let count = await hog.count('User');
    expect(count).toEqual(3);
  });

  it('shows table list', async () => {
    let tables = await hog.exec('table_list');
    expect(tables).toEqual({});
  });

  it('can mget records', async () => {
    let keys = ['alice', 'bob', 'carol'].map(toBuf);
    let names = await hog.mget('User.name', 'short_text', 'short_text', keys);
    names = names.map((name) => name.toString("utf-8"));
    expect(names).toEqual(['Alice', 'Bob', 'Carol']);
  });

  it('can add records', async () => {
    const doo = toBuf('doo');
    await hog.mput('User.name', 'short_text', 'short_text',
      [doo, toBuf('Doo')]);
    let name = await hog.get('User.name', 'short_text', 'short_text', doo);
    expect(name.toString("utf-8")).toEqual('Doo');
    let buf = Buffer.alloc(4);
    buf.writeInt32BE(30, 0);
    await hog.store("User", "short_text", doo, [["int32", "age", buf]]);
    let age = await hog.get('User.age', 'short_text', 'int32', doo);
    expect(age.readInt32BE()).toEqual(30);
  });

  it('can query records', async () => {
    let [total, count, keys] =
      await hog.query('User.name', 'short_text', 'Alice');
    expect(total).toEqual(1);
    expect(count).toEqual(1);
    expect(keys[0].toString("utf-8")).toEqual('alice');
  });
});

function prepareGrnDir() {
  execSync('mkdir -p tmp/groonga');
  execSync('rm -rf tmp/groonga/test*');
}

async function setupFixture() {
  // open new file tmp/groonga/setup.grn to write.
  const grn = fs.createWriteStream('tmp/groonga/setup.grn');
  // write groonga command to setup.grn
  // create table, column, load data
  // and execute groonga command
  // and close setup.grn
  grn.write(`
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
  `);

  // init groonga db by CLI via docker.
  let cp = spawn('docker', [ 'run', '--rm', '-v',
    `${process.cwd()}/tmp/groonga:/tmp`,
    'groonga/groonga:12.0.8-alpine', '-n',
    '--file', '/tmp/setup.grn', '/tmp/test']);
  // wait for the process to end.
  await new Promise((resolve) => cp.on('exit', resolve));
}

async function startHog() {
  // Read version number from ../../VERSION
  const ver = fs.readFileSync('../../VERSION', 'utf-8').trim();
  let cp = spawn('docker', [ 'run', '--rm', '--name', 'tsclient-hog',
    '-p', '18618:18618', '-v', `${process.cwd()}/tmp/groonga:/tmp`,
    `s21g/hog:${ver}`, '/tmp/test', ]);
  // wait for the message 'hog server started accepting' to be output.
  await new Promise((resolve) => {
    cp.stdout.on('data', (data) => {
      if (data.toString().includes('hog server started accepting')) {
        resolve();
      }
    });
  });
}

async function stopHog() {
  await execSync('docker rm -f tsclient-hog');
}
