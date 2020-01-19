# mmlite
A Multi Master (Lite) extension for PostgreSQL

The extension works exactly as postgres' own logical replication pgoutput plugin (it's basically copied) with one (1) major difference/addition. It filters out all output that's got a valid origin. That means nodes can publish/subscribe to each other without bouncing messages back and forth, giving us a lightweight multi master setup.

Only works with PostgreSQL 12.

Caveats apply!

## Install
    git clone https://github.com/Underhunden/mmlite.git

Make sure pg_config is in your path.

    PGXS=1 make
    PGXS=1 make install

## Usage

Make sure you know how logical replication works before you start.

https://www.postgresql.org/docs/12/logical-replication.html


Set `wal_level = 'logical'` in postgresql.conf and restart.

### Node 1:

    CREATE PUBLICATION node1 FOR TABLE users, departments;</code
    SELECT pg_create_logical_replication_slot('node2', 'mmlite');

### Node 2:

    CREATE PUBLICATION node2 FOR TABLE users, departments;
    SELECT pg_create_logical_replication_slot('node1', 'mmlite');

    CREATE SUBSCRIPTION from_node1 CONNECTION 'dbname=?? host=?? user=??' PUBLICATION node1 WITH (copy_data = true, create_slot = false, slot_name = 'node2');

Now node2 should be subscribed to node1, let's subscribe to node2 from node1.

### Node1:

    CREATE SUBSCRIPTION from_node2 CONNECTION 'dbname=?? host=?? user=??' PUBLICATION node2 WITH (copy_data = false, create_slot = false, slot_name = 'node1');

Now both nodes should be subscribed to each other and will be getting updates. Add more nodes by subscribing to all existing, and let existing subscribe to the newly added one.

## Caveats
### Conflicts
* No handling of conflicts included. You are responsible for designing a conflict free application.
  * Primary key serials might cause conflicts - use UUID or other random PK.
  * Use a "logical" master (can be any of the nodes, but decide on one) within your application for values such as:
    * Unique emails in a users table.
    * Transactions to/from balance tables.
    * Everything that needs to happen only once.
### Other
* If `copy_data = true` is set when setting up a new subscription, it won't set an origin for the data to be transmitted. Make sure **no** other nodes are subscribed to a **new** node before all the data gets replicated. 

## TODO
* Support for older pg versions.

## Questions/Suggestions
Please let me know, open a ticket or create a PR.
