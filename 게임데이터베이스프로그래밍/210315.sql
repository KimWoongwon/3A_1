
select userid, username from usertbl where height between 175 and 180;
select * from usertbl where addr in ('경남', '경북', '충남');
select username, height from usertbl where username like '김%';
select * from usertbl where userid like '_K%';
select * from usertbl order by joindate;
select * from usertbl order by joindate desc;
select * from usertbl order by joindate limit 5;
select distinct addr from usertbl;
select addr, count(*) as '거주인원수' from usertbl group by addr;
select count(*) from usertbl where joindate >= '2010-01-01';
select count(mobile) from usertbl;
select userid, prodname, price*amount from buytbl;
select userid, sum(price*amount)from buytbl group by userid;
select userid, avg(price*amount)from buytbl group by userid;
select avg(price*amount) from buytbl;
select addr, count(*) from usertbl where height >= 175 group by addr;
select userid, sum(price*amount) from buytbl group by userid having sum(price * amount) >= 1000;
select userid, sum(price*amount) as 총구매액 from buytbl group by userid having 총구매액 >= 1000;
select userid, sum(price*amount) as 총구매액 from buytbl where amount >= 5 group by userid having 총구매액 >= 1000;

