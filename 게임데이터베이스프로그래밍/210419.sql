select if(100<200, '참', '거짓');

use cookdb;
set @avg_height = (select avg(height) from usertbl);
select username, height, if(height >= @avg_height, "평균 이상", "평균 이하") as '키' from usertbl;

select ifnull(null, 100), ifnull(200,100); 
update buytbl set groupname = null where groupname = '없음';

select userid, prodname, ifnull(groupname, '없음') from buytbl;

select nullif(100, 100), nullif(300, 200);
use madang;

create database MovieDB;
use moviedb;

create table movietbl
(
movie_id int,
movie_title varchar(30),
movie_director varchar(20),
movie_star varchar(20),
movie_script longtext,
movie_film longblob
) default charset = utf8mb4;
