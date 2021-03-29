-- 서브쿼리 사용법
select username, addr, height from usertbl 
where height = (select max(height) from usertbl) 
or height = (select min(height) from usertbl);
-- 김용만의 키 이상의 사람들만 출력
select * from usertbl where height >= (select height from usertbl where username = "김용만");
-- 지역이 경기인 사람들보다 키가 큰사람들을 출력 (any all 사용법)
select * from usertbl where height >= all(select height from usertbl where addr = '경기');
-- 가장 나이 많은 사람
select * from usertbl where birthyear = (select min(birthyear) from usertbl);

-- join 사용법

select * from customer inner join orders on customer.custid = orders.custid; -- inner 생략가능
select * from customer cs join orders od on cs.custid = od.custid; -- 별칭 부여 가능
-- 별칭으로 보고싶은 항목만 선택가능
select cs.custid, cs.name, cs.address, od.bookid, od.saleprice, od.orderdate from customer cs join orders od on cs.custid = od.custid; 
-- 조건을 통해 속도의 차이를 많이 줄일 수 있다 join은 무거운 연산이다
select * from customer cs join orders od on cs.custid = od.custid where cs.name = '박지성'; 

-- 가격이 10000원 이상인 책의 판매 정보 조회
select bk.bookname, bk.price, od.saleprice, od.orderdate from book bk join orders od on bk.bookid = od.bookid where bk.price >= 10000;

-- 고객별 고객의 이름과 주소, 총 구매액을 조회
select cs.name, cs.address, sum(od.saleprice) from customer cs join orders od on cs.custid = od.custid group by cs.custid;

-- cookdb에서 KYM이라는 아이디를 가진 회원이 구매한 물건과 회원 정보 조회(아이디, 이름, 물품, 물품 가격, 주소, 연락처)
select u.userid, u.username, b.prodname, b.price * b.amount as `총 구매가격`, u.addr, u.mobile 
from usertbl u join buytbl b on u.userid = b.userid order by `총 구매가격` desc;

-- cookdb에서 경북에 거주하는 유저들의 아이디, 이름, 구매물품, 구매액을 조회
select u.userid, u.username, b.prodname, b.price * b.amount as `총 구매가격`
from usertbl u join buytbl b on u.userid = b.userid where u.addr = '경북' order by `총 구매가격` desc;