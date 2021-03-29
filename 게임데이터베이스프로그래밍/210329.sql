-- 날짜 별로 가장 돈을 많이 쓴 사람, 구매 가격, 날짜 조회
select c.name, o.saleprice, o.orderdate from customer c
join (select * from orders where (orderdate, saleprice) in (select orderdate, max(saleprice) from orders group by orderdate)) o
on c.custid = o.custid;

-- 가격이 10000원보다 비싼책을 구입한 고객의 이름과 주소 조회
select c.name, c.address from customer c where custid in (select custid from orders where saleprice > 10000);

-- 도서를 구매한 적이 있는 고객의 이름을 검색
select name from customer where custid in (select custid from orders);

-- 각 지역별로 가장 키가 큰 사람들의 모든 정보 조회
use cookdb;
select * from usertbl where (addr, height) in (select addr, max(height) from usertbl group by addr);

-- 대한 미디어에서 출판한 도서를 구매한 고객의 이름을 조회
use madang;
select name from customer where custid in (select custid from orders where bookid in (select bookid from book where publisher = "대한미디어"));

-- 총 구매액이 1000원이 넘는 고객들의 정보 조회
use cookdb;
select * from usertbl where userid in (select userid from buytbl group by userid having sum(price * amount) > 1000);

-- 출판서 별로 출판사의 평균 도서 가격보다 비싼 도서를 조회
use madang;
select * from book b1 where b1.price > (select avg(b2.price) from book b2 where b1.publisher = b2.publisher);

-- 각 지역별로 가장 키가 큰 사람들의 모든 정보 조회 (상관쿼리 버전)
use cookdb;
select * from usertbl u1 where u1.height = (select max(height) from usertbl u2 where u1.addr = u2.addr); 

-- 총 구매액이 1000원이 넘는 고객들의 정보 조회
use cookdb;
select * from usertbl where userid in
(select b1.userid from buytbl b1 where (select sum(b2.price * b2.amount) from buytbl b2 where b1.userid = b2.userid) > 1000);

select u.userid, u.username, u.addr from usertbl u where exists (select * from buytbl b where u.userid = b.userid);

select userid, username, addr from usertbl where userid in 
(select userid from (select userid, sum(price * amount) as 총구매액 from buytbl group by userid) as tb
where tb.총구매액 > 1000);

select u.userid, u.username, b.총구매액 from usertbl u join 
(select userid, sum(price * amount) as 총구매액 from buytbl group by userid) as b
on u.userid = b.userid where b.총구매액 > 1000;

-- 제품 분류 별로 가장구매액이 큰 유저의 이름, 연락처, 구매액 조회
select b.groupname, u.username, u.mobile, b.price * b.amount
from usertbl u join 
(select * from buytbl where (groupname, price*amount) in
(select groupname, max(price * amount) from buytbl group by groupname)) as b
on u.userid = b.userid;

use cookdb;
update buytbl set groupname='없음' where groupname is null;

select * from usertbl u left outer join buytbl b on u.userid = b.userid where b.userid is null;
select * from stdtbl s left outer join stdclubtbl sc on s.stdName = sc.stdName;

select c.clubname, c.roomno, sc.stdname, s.addr from clubtbl c 
left outer join stdclubtbl sc on c.clubname = sc.clubname
left outer join stdtbl s on sc.stdname = s.stdname;