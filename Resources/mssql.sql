SELECT CAST(stamp AS bigint) AS stamp FROM ue5_test.dbo.Table_1

UPDATE ue5_test.dbo.Table_1 SET notes = 'note sample' WHERE id=2

INSERT INTO ue5_test.dbo.Table_1 (id, name, address, phone, exprience, notes, last_login, membership_date) VALUES (99, 'John Doe', 'New York', '+410104952147', 5.5, 'Developer', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)

DELETE FROM ue5_test.dbo.Table_1 WHERE id >= 6
