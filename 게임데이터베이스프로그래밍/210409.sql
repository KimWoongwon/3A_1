create table test1
( 
id int auto_increment primary key,
date1 date,
date2 datetime,
date3 timestamp 
);

insert into test1 (date1, date2, date3) values(curdate(), now(), now());
insert into test1 (date1) values(curdate());

alter table test1 modify date3 timestamp default current_timestamp on update current_timestamp;
insert into test1 (date1, date2) values(curdate(), now());

update test1 set id = 10 where id=2;

select cast('2020-10-19 12:35:29.123' as date) as 'date';
select cast('2020-10-19 12:35:29.123' as time) as 'time';
select cast('2020-10-19 12:35:29.123' as datetime) as 'datetime';

set @myVar1 = 5;
set @myVar2 = 3;
set @myVar3 = 4.25;
set @myVar4 = 'MC 이름 ==>';

select @myVar1;
select @myVar2 + @myVar3;
select @myVar4, username from usertbl where height > 180;

use madang;
set @aa = '나무수';
set @answer = (select price from book where publisher = @aa);
select @answer;

use cookdb;
SET @myVar1 = 3;
PREPARE myQuery FROM 'SELECT userName, height FROM userTBL ORDER BY height LIMIT ?';
EXECUTE myQuery USING @myVar1;

set @myvar1 = 190;
set @myvar2 = 180;
prepare myQuery2 from 'select username, height from usertbl where height between ? and ?';
execute myQuery2 using @myvar2, @myvar1;
execute myQuery2 using @myvar2, @myvar1;

set @addr = '경남';
prepare myquery1 from 'select username from usertbl where addr = ?';
execute myquery1 using @addr;

set @myvar1 = 180;
set @query1 = 'select * from usertbl where height >= ?';
prepare myquery2 from @query1;
execute myquery2 using @myvar1;

SELECT CAST(AVG(amount) AS SIGNED INTEGER) AS '평균 구매 개수' FROM buyTBL;
SELECT CONVERT(AVG(amount), SIGNED INTEGER) AS '평균 구매 개수' FROM buyTBL;

SELECT CAST('2020$12$12' AS DATE);
SELECT CAST('2020/12/12' AS DATE);
SELECT CAST('2020%12%12' AS DATE);
SELECT CAST('2020*12*12' AS DATE);

SELECT num, CONCAT(CAST(price AS CHAR(10)), 'X', CAST(amount AS CHAR(4)), '=') AS '단가X수량', price * amount AS '구매액' FROM buyTBL;

use employees;
select concat(first_name,' ', last_name) from employees;


