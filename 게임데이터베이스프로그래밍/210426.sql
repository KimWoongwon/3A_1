use cookdb;
select u.userid, u.username, sum(price*amount) as '총구매액' from buytbl b
right outer join usertbl u on u.userid = b.userid
group by b.userid order by sum(price*amount) desc;