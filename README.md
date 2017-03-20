Backend Server.

It uses SQL to store settings & DB should have 2 tables & follow this format:

MariaDB [smartoffice_srv]> describe nodes;
+------------+--------------+------+-----+---------+----------------+
| Field      | Type         | Null | Key | Default | Extra          |
+------------+--------------+------+-----+---------+----------------+
| id         | int(11)      | NO   | PRI | NULL    | auto_increment |
| hash       | varchar(255) | NO   | UNI | NULL    |                |
| ip         | varchar(255) | NO   | MUL | NULL    |                |
| port       | int(11)      | NO   |     | NULL    |                |
| type       | varchar(255) | NO   |     | NULL    |                |
| attributes | blob         | NO   |     | NULL    |                |
+------------+--------------+------+-----+---------+----------------+
6 rows in set (0.00 sec)

MariaDB [smartoffice_srv]> describe config;
+-------+--------------+------+-----+---------+-------+
| Field | Type         | Null | Key | Default | Extra |
+-------+--------------+------+-----+---------+-------+
| name  | varchar(255) | NO   | PRI | NULL    |       |
| value | varchar(255) | NO   |     | NULL    |       |
+-------+--------------+------+-----+---------+-------+
2 rows in set (0.00 sec)

MariaDB [smartoffice_srv]> 
 

Examples:
MariaDB [smartoffice_srv]> select * from nodes;
+----+-------------------------+---------------+------+------+-------------------+
| id | hash                    | ip            | port | type | attributes        |
+----+-------------------------+---------------+------+------+-------------------+
|  1 | bo_test                 | 192.168.1.188 |   80 | bo   | {"default":true}  |
|  5 | 237c8e4d3d631e604fsdf23 | 192.168.1.188 |  255 |      | {"default":false} |
|  6 | dest_test               | 192.168.1.188 | 2525 | bo   |                   |
|  8 | call_test               | 192.168.1.188 | 3222 | call |                   |
| 10 | lock_test               | 192.168.1.188 | 4444 | pcd  |                   |
+----+-------------------------+---------------+------+------+-------------------+
5 rows in set (0.00 sec)

MariaDB [smartoffice_srv]> select * from config;
+----------------------------+--------------------------------+
| name                       | value                          |
+----------------------------+--------------------------------+
| API:auth:Api-key           | AJqoaxcJda2891akd              |
| API:auth:Secret-key        | djkasdhajksHHAJsj29109daks     |
| API:bo_test:access request | GET:/api/v1/node/access        |
| API:bo_test:event dump     | POST:/api/v1/node/event        |
| API:bo_test:host           | somi-d8.loc                    |
| API:bo_test:time-sync      | POST:/api/v1/node/time-sync    |
+----------------------------+--------------------------------+
6 rows in set (0.00 sec)

MariaDB [smartoffice_srv]> 

