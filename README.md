# mmlite
A Multi Master (Lite) extension for PostgreSQL

The extension works exactly as postgres' own logical replication pgoutput plugin (it's basically copied) with one (1) major difference/addition. It filters out all output that's got a valid origin. That means nodes can publish/subscribe to each other without bouncing messages back and forth, giving us a multi master (lite) setup.

Caveats apply!

## Install
Make sure pg_config is in your path.

<code>PGXS=1 make</code>

<code>PGXS=1 make install</code>

## Usage
Node 1:

Node N:

## Caveats
* serials - use uuid/other random PK.
* copy_data = true in subscription setup - won't give you an origin, do it before subscribing.
* uniques - treat one of your nodes as a logical master node.
