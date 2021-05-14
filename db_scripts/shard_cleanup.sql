use sql_test;
DELETE FROM Person
-- sharding:0
;
DELETE FROM Person
-- sharding:1
;
DELETE FROM Person
-- sharding:2
;