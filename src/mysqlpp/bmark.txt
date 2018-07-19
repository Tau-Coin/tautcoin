All unit tests passed
---------------- BEGIN resetdb OUTPUT ----------------
Connecting to database server...
Dropping existing sample data tables...
Creating stock table...
Populating stock table...inserted 4 rows.
Creating empty images table...
Creating deadlock testing tables...
Reinitialized sample database successfully.
================ END resetdb OUTPUT ================

---------------- BEGIN simple1 OUTPUT ----------------
We have:
	Nürnberger Brats
	Pickle Relish
	Hot Mustard
	Hotdog Buns
================ END simple1 OUTPUT ================

---------------- BEGIN simple2 OUTPUT ----------------
Item                           Num       Weight    Price     Date

Nürnberger Brats              97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.10      1998-04-23
================ END simple2 OUTPUT ================

---------------- BEGIN simple3 OUTPUT ----------------
Item                           Num       Weight    Price     Date

Nürnberger Brats              97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.10      1998-04-23
================ END simple3 OUTPUT ================

---------------- BEGIN store_if OUTPUT ----------------
Records found: 2

Item                           Num       Weight    Price     Date

Nürnberger Brats              97        1.5       8.79      2005-03-10
Hot Mustard                    73        0.95      0.97      1998-05-25
================ END store_if OUTPUT ================

---------------- BEGIN for_each OUTPUT ----------------
There are 322 items weighing 416.85 stone and costing 1147.19 cowrie shells.
================ END for_each OUTPUT ================

---------------- BEGIN multiquery OUTPUT ----------------
Multi-query: 
DROP TABLE IF EXISTS test_table; CREATE TABLE test_table(id INT); INSERT INTO test_table VALUES(10); UPDATE test_table SET id=20 WHERE id=10; SELECT * FROM test_table; DROP TABLE test_table
Result set 0 is empty.
Result set 1 is empty.
Result set 2 is empty.
Result set 3 is empty.
Result set 4 has 1 row:
  +----+
  | id |
  +----+
  | 20 |
  +----+
Result set 5 is empty.
Stored procedure query: 
DROP PROCEDURE IF EXISTS get_stock; CREATE PROCEDURE get_stock( i_item varchar(20) ) BEGIN SET i_item = concat('%', i_item, '%'); SELECT * FROM stock WHERE lower(item) like lower(i_item); END;
Result set 0 is empty.
Result set 1 is empty.
Query: CALL get_stock('relish')
Result set 0 has 1 row:
  +---------------+-----+--------+-------+------------+-------------+
  |          item | num | weight | price |      sdate | description |
  +---------------+-----+--------+-------+------------+-------------+
  | Pickle Relish |  87 |    1.5 |  1.75 | 1998-09-04 |        NULL |
  +---------------+-----+--------+-------+------------+-------------+
Result set 1 is empty.
================ END multiquery OUTPUT ================

---------------- BEGIN tquery1 OUTPUT ----------------
Query: select * from stock
Records found: 4

Item                           Num       Weight    Price     Date

Nuerenberger Bratwurst         97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.1       1998-04-23
================ END tquery1 OUTPUT ================

---------------- BEGIN resetdb OUTPUT ----------------
Connecting to database server...
Dropping existing sample data tables...
Creating stock table...
Populating stock table...inserted 4 rows.
Creating empty images table...
Creating deadlock testing tables...
Reinitialized sample database successfully.
================ END resetdb OUTPUT ================

---------------- BEGIN tquery2 OUTPUT ----------------
Query: select * from stock
Records found: 4

Item                           Num       Weight    Price     Date

Nuerenberger Bratwurst         97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.1       1998-04-23
================ END tquery2 OUTPUT ================

---------------- BEGIN tquery3 OUTPUT ----------------
Stuff we have a lot of in stock:
	Nuerenberger Bratwurst
	Pickle Relish
================ END tquery3 OUTPUT ================

---------------- BEGIN tquery4 OUTPUT ----------------
Query: update stock set num = 70 where num < 70
Query: select * from stock
Records found: 4

Item                           Num       Weight    Price     Date

Nuerenberger Bratwurst         97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    70        1.1       1.1       1998-04-23

Query: select * from stock where weight > 1.2 or description like '%Mustard%'
Records found: 3

Item                           Num       Weight    Price     Date

Nuerenberger Bratwurst         97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
================ END tquery4 OUTPUT ================

---------------- BEGIN resetdb OUTPUT ----------------
Connecting to database server...
Dropping existing sample data tables...
Creating stock table...
Populating stock table...inserted 4 rows.
Creating empty images table...
Creating deadlock testing tables...
Reinitialized sample database successfully.
================ END resetdb OUTPUT ================

---------------- BEGIN ssqls1 OUTPUT ----------------
We have:
	Nürnberger Brats
	Pickle Relish
	Hot Mustard (good American yellow mustard, not that European stuff)
	Hotdog Buns
================ END ssqls1 OUTPUT ================

---------------- BEGIN ssqls2 OUTPUT ----------------
Query: INSERT INTO `stock` (`item`,`num`,`weight`,`price`,`sDate`,`description`) VALUES ('Hot Dogs',100,1.5,NULL,'1998-09-25',NULL)
Query: select * from stock
Records found: 5

Item                           Num       Weight    Price     Date

Nürnberger Brats              97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.1       1998-04-23
Hot Dogs                       100       1.5       (NULL)    1998-09-25
================ END ssqls2 OUTPUT ================

---------------- BEGIN ssqls3 OUTPUT ----------------
Query: UPDATE `stock` SET `item` = 'Nuerenberger Bratwurst',`num` = 97,`weight` = 1.5,`price` = 8.7899999999999991,`sDate` = '2005-03-10',`description` = NULL WHERE `item` = 'Nürnberger Brats'
Query: select * from stock
Records found: 5

Item                           Num       Weight    Price     Date

Nuerenberger Bratwurst         97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.1       1998-04-23
Hot Dogs                       100       1.5       (NULL)    1998-09-25
================ END ssqls3 OUTPUT ================

---------------- BEGIN ssqls4 OUTPUT ----------------
Records found: 5

Item                           Num       Weight    Price     Date

Hot Dogs                       100       1.5       (NULL)    1998-09-25
Hot Mustard                    73        0.95      0.97      1998-05-25
Hotdog Buns                    65        1.1       1.1       1998-04-23
Nuerenberger Bratwurst         97        1.5       8.79      2005-03-10
Pickle Relish                  87        1.5       1.75      1998-09-04

Currently 65 hotdog buns in stock.
================ END ssqls4 OUTPUT ================

---------------- BEGIN ssqls5 OUTPUT ----------------
Custom query:
select * from stock where `weight` = 1.5 and `price` = 8.7899999999999991
================ END ssqls5 OUTPUT ================

---------------- BEGIN ssqls6 OUTPUT ----------------
Query: select * from stock
Records found: 26

Item                           Num       Weight    Price     Date

Tiny Screws                    1000      0.01      0.05      2008-11-11
Needle-nose Pliers             50        0.5       5.95      2008-11-12
Small Soldering Iron           40        0.5       15.95     2008-09-01
Large Soldering Iron           35        0.75      24.95     2008-08-01
Solder Wick                    100       0.1       2.95      2008-04-01
Mini Screwdrivers, 3 pc.       30        0.4       8.95      2008-03-25
Mini Screwdrivers, 6 pc.       40        0.6       12.95     2008-04-01
Wire-wrapping Tool             25        0.2       4.95      2008-04-23
Red LED, 5mm, 3000mcd          300       0.01      0.29      2008-10-02
Orange LED, 5mm, 2500mcd       250       0.01      0.29      2008-07-31
Yellow LED, 5mm, 3000mcd       400       0.01      0.25      2008-09-30
Green LED, 5mm, 1000mcd        350       0.01      0.45      2008-09-27
Blue LED, 5mm, 3900mcd         500       0.01      0.34      2007-12-01
White LED, 5mm, 15000mcd       750       0.01      0.43      2008-02-01
AA Battery, single             220       0.05      0.5       2007-09-19
AA Battery, 4-pack             60        0.2       1.79      2007-08-03
AA Battery, 24-pack            8         1.2       9.99      2007-04-25
C Battery, single              100       0.075     0.65      2007-11-14
C Battery, 4-pack              25        0.3       2.29      2007-06-05
C Battery, 24-pack             5         1.8       10.99     2007-06-13
D Battery, single              180       0.08      0.7       2007-12-03
D Battery, 4-pack              45        0.3       2.59      2007-04-01
D Battery, 24-pack             12        1.9       11.99     2007-05-15
9-volt Battery, single         90        0.06      0.75      2008-01-02
9-volt Battery, 3-pack         17        0.2       1.99      2008-02-28
9-volt Batter, 20-pack         12        1.2       12.99     2007-12-28
================ END ssqls6 OUTPUT ================

---------------- BEGIN load_jpeg OUTPUT ----------------
Inserted "NULL" into images table, 0 bytes, ID 1
================ END load_jpeg OUTPUT ================

---------------- BEGIN cgi_jpeg OUTPUT ----------------
Content-type: text/plain

No image content!
================ END cgi_jpeg OUTPUT ================

--- BEGIN ssqlsxlat -i examples/common.ssqls -o ERROR OUTPUT ---
==== END ssqlsxlat -i examples/common.ssqls -o ERROR OUTPUT ====
--- BEGIN ssqlsxlat -i examples/stock.ssqls -o ERROR OUTPUT ---
==== END ssqlsxlat -i examples/stock.ssqls -o ERROR OUTPUT ====
--- BEGIN ssqlsxlat -i test/test1.ssqls -o ERROR OUTPUT ---
==== END ssqlsxlat -i test/test1.ssqls -o ERROR OUTPUT ====
--- BEGIN ssqlsxlat -i test/test2.ssqls -o ERROR OUTPUT ---
==== END ssqlsxlat -i test/test2.ssqls -o ERROR OUTPUT ====
