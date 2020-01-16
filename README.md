# mmlite
A Multi Master (Lite) extension for PostgreSQL

The extension works exactly as postgres' own logical replication pgoutput plugin (it's basically copied) with one (1) major difference/addition. It filters out all output that's got a valid origin. That means nodes can publish/subscribe to each other without bouncing messages back and forth, giving us a multi master (lite) setup.

Only tested with PostgreSQL 12.

Caveats apply!

## Install
<code>git clone https://github.com/Underhunden/mmlite.git</code>

Make sure pg_config is in your path.

<code>PGXS=1 make</code>

<code>PGXS=1 make install</code>

## Usage

Set <code>wal_level = 'logical'</code> in postgresql.conf and restart.

Node 1:
<code>CREATE PUBLICATION node1 FOR TABLE users, departments;</code>
<code>SELECT pg_create_logical_replication_slot('node2', 'mmlite');</code>

Node 2:
<code>CREATE PUBLICATION node2 FOR TABLE users, departments;</code>
<code>SELECT pg_create_logical_replication_slot('node1', 'mmlite');</code>
<code>CREATE SUBSCRIPTION from_node1 CONNECTION 'dbname=?? host=?? user=??' PUBLICATION node1 WITH (copy_data = true, create_slot = false, slot_name = 'node2');</code>

Now node2 should be subscribed to node1, now let's subscribe to node2 from node1.

Node1:
<code>CREATE SUBSCRIPTION from_node2 CONNECTION 'dbname=?? host=?? user=??' PUBLICATION node2 WITH (copy_data = false, create_slot = false, slot_name = 'node1');</code>

Now both nodes should be subscribed to each other and will be getting updates. Add more nodes by subscribing to all existing, and let existing subscribe to the newly added one.

## Caveats
* serials - use uuid/other random PK.
* copy_data = true in subscription setup - won't give you an origin, do it before subscribing.
* uniques - treat one of your nodes as a logical master node.
