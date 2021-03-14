# mmlite
A lightweight multi master extension for PostgreSQL.

The extension works exactly as postgres' own logical replication pgoutput plugin (it's basically copied) with one (1) major difference/addition. It filters out all output that's got a valid origin. This means you can publish/subscribe to tables existing in all nodes without bouncing messages back and forth, giving us a lightweight multi master setup.

Tested with PostgreSQL 11, 12 and 13.

Caveats apply!

## Install
    git clone https://github.com/Underhunden/mmlite.git

Make sure pg_config is in your path and you have development tools installed.

    cd pg13 # or pg11, pg12 - depending on the major version you have installed
    PGXS=1 make
    PGXS=1 make install

## Usage

Make sure you know how logical replication works before you start.
* https://www.postgresql.org/docs/11/logical-replication.html
* https://www.postgresql.org/docs/12/logical-replication.html
* https://www.postgresql.org/docs/13/logical-replication.html

Set `wal_level = logical` in postgresql.conf and restart.

### Node 1:

    CREATE PUBLICATION node1 FOR TABLE users, departments;
    SELECT pg_create_logical_replication_slot('node2', 'mmlite');

Make sure your replication slot use the new mmlite extension.

    SELECT slot_name, plugin FROM pg_replication_slots;

    slot_name | plugin
    -----------+--------
    node2     | mmlite

### Node 2:

    CREATE PUBLICATION node2 FOR TABLE users, departments;
    SELECT pg_create_logical_replication_slot('node1', 'mmlite');

    CREATE SUBSCRIPTION from_node1 CONNECTION 'dbname=?? host=?? user=??' PUBLICATION node1 WITH (copy_data = true, create_slot = false, slot_name = 'node2');

Now node2 should be subscribed to node1, let's subscribe to node2 from node1.

### Node 1:

    CREATE SUBSCRIPTION from_node2 CONNECTION 'dbname=?? host=?? user=??' PUBLICATION node2 WITH (copy_data = false, create_slot = false, slot_name = 'node1');

Now both nodes should be subscribed to each other and will be getting updates.

### Node 3:

This module do support having many to many subscriptions, however care must be taken when adding new nodes. The easiest way to add a node without losing transactions is to shut down or put your application in read-only mode, that way no new transactions will being created during the creation of a new node. If shutting down your application is not possible, you're up for something more complicated.

Here's one way of doing it:
* Decide on one node to be used as source for the initial data copy, in this case we choose Node 1.
* As quickly/synced as possible, run the creation of slots (Node 1 and 2) and the `CREATE SUBSCRIPTION` command on Node 3.
  * Start with Node 2, then Node 1 and last the `CREATE SUBSCRIPTION` on Node 3.
* Every transaction that happened on Node 2 between creation of a new slot on Node 2 and Node 1 will later possibly become a conflict on the new node.
* You will have to look at the log on Node 3 after subscribing to Node 2.
  * If conflicts arise, those have to be solved.
    * Change the data to that those duplicated transactions can be replayed from Node 2.
    * Skip those transactions. The manual suggests this:
	  * https://www.postgresql.org/docs/11/logical-replication-conflicts.html
	  * https://www.postgresql.org/docs/12/logical-replication-conflicts.html
	  * https://www.postgresql.org/docs/13/logical-replication-conflicts.html
	  * A function is on the TODO list to skip transactions step by step.
* When all data is copied and Node 3 is subscribing to Node 1 and 2, it's time for Node 1 and 2 to subscribe to Node 3.

Note: If more than 3 nodes is needed, these steps will include handling conflicts from several sources.

Here's another way of doing it:
* Decide on one node to be used as source for the initial data copy, in this case we choose Node 2.
* We make Node 3 a sync standby of Node 2.
  * One option is to use pg_restore from Node 3 reading everything from Node 2 and connecting up to be a sync standby.
  * https://www.postgresql.org/docs/current/app-pgbasebackup.html
  * Another option could be to restore a backup using pgbackrest - Choose the best tool for the job.
* When Node 3 has catched up, we remove Node 2 and Node 3 from the mmlite cluster (as well as from receiving traffic from clients) - by changing pg_hba, iptables or in any other way.
* Promote Node 3 to be a primary node.
* Run the `CREATE PUBLICATION` step for adding Node 3 on Node 1 and Node 2.
* On node 1, use `pg_copy_logical_replication_slot` for copying the same replication slot as Node 2 for Node 3.
  * Or create a slot, but use `pg_replication_slot_advance` if you're on pg11.
* Run `CREATE PUBLICATION` and `CREATE SUBSCRIPTION` appropriately.
* Open up the traffic for Node 2 and Node 3.

Note 1: The node(s) that are still in production during the pg_hba/iptables/network block will be accumulating WAL - have enough free disk!

Note 2: These steps can be repeated for more nodes - just think about Note 1.

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
### All restrictions with logical replication also applies to this extension
* Read more here:
  * https://www.postgresql.org/docs/11/logical-replication-restrictions.html
  * https://www.postgresql.org/docs/12/logical-replication-restrictions.html
  * https://www.postgresql.org/docs/13/logical-replication-restrictions.html

## TODO
* Setup REGRESS tests.
* Fix LICENSE?
* Create a function for skipping transactions step-by-step.

## Questions/Suggestions
Please let me know, open a ticket or create a PR.
