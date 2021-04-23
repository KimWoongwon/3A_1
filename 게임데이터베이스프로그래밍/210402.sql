use cookdb;
select * from usertbl u left outer join buytbl b on u.userid = b.userid where b.userid is null;

-- union 필드 갯수가 같아야 한다.
select userid, mobile from usertbl union
select userid, price from buytbl;

select e1.emp, e1.manager, e2.emptel from emptbl e1 join emptbl e2 on e1.manager = e2.emp where e1.emp = '우대리';

select max(height) from usertbl;
select userid from usertbl where addr = '서울';
select userid, username from usertbl where addr = '서울';

select userid, username, addr from usertbl where userid in 
(select userid from (select userid, sum(price*amount) 총구매액 from buytbl group by userid) tb
where tb.총구매액>1000);

select u.userid, u.username, u.addr, tb.총구매액 from usertbl u 
join (select userid, sum(price*amount) 총구매액 from buytbl group by userid) tb
on u.userid = tb.userid where tb.총구매액>1000;

create view sum_pricetbl as (select userid, sum(price*amount) 총구매액 from buytbl group by userid);

select u.userid, u.username, u.addr, tb.총구매액 from usertbl u 
join sum_pricetbl tb on u.userid = tb.userid where tb.총구매액>1000;

-- 각 출판사에서 가장 비싼 책을 구입한 고객의 이름과 주소
use madang;
create view max_price_by_publisher as 
(select * from book b1 where b1.price = (select max(price) from book b2 where b1.publisher = b2.publisher));

select c.name, c.address, mp.bookname from customer c join orders o on c.custid = o.custid
join max_price_by_publisher mp on o.bookid = mp.bookid;
