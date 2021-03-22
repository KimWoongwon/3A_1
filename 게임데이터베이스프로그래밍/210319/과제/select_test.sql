-- 1. 가격이 20000원 미만인 도서를 검색
select * from book where price < 20000;
-- 2. 가격이 10000원 이상 20000원 이하인 도서를 검색
select * from book where price between 10000 and 20000;
-- 3. 출판사가 굿스포츠 혹은 대한미디어인 도서를 검색
select * from book where publisher in('굿스포츠', '대한미디어');
-- 4. 출판사가 굿스포츠 혹은 대한미디어가 아닌 도서를 검색
select * from book where publisher not in('굿스포츠', '대한미디어');
-- 5. '축구의 역사'를 출간한 출판사를 검색
select bookname, publisher from Book where bookname like '축구의 역사';
-- 6. 도서이름에 '축구'가 포함된 출판사를 검색
select * from book where bookname like '%축구%';
-- 7. 도서이름이 여섯 글자인 도서를 검색
select * from book where bookname like '______';
-- 8. 도서이름의 왼쪽 두 번째 위치에 '구'라는 문자열을 갖는 도서를 검색
select * from book where bookname like '_구%';
-- 9. 축구에 관한 도서 중 가격이 20000원 이상인 도서를 검색
select * from book where bookname like '%축구%' and price >= 20000;
-- 10. 야구에 관한 책을 모두 구입하려면 필요한 금액 계산
select sum(price) from book where bookname like '%야구%';
-- 11. 도서를 가격 순으로 검색하고, 가격이 같이면 이름순으로 검색
select * from book order by price , bookname;
-- 12. 도서를 가격의 내림차순으로 검색하고 만약 가격이 같다면 출판사의 오름차순으로 검색
select * from book order by price desc , publisher asc;
-- 13. 주소가 우리나라나 영국인 선수 정보 조회
select * from customer where (address like '%대한민국%') or (address like '%영국%');
-- 14. 고객이 주문한 도서의 총 판매액 조회
select sum(saleprice) from orders where saleprice;
-- 15. 2번 김연아 고객이 주문한 도서의 총 판매액 조회
select custid, sum(saleprice) from orders where custid = 2;
-- 16. 고객이 주문한 도서의 총 판매액, 평균값, 최저가, 최고가 조회
select sum(saleprice), avg(saleprice), min(saleprice), max(saleprice) from orders;
-- 17. 마당서점의 도서 판매 건수 조회
select count(*) from orders;
-- 18. 고객별로 주문한 도서의 총 수량과 총 판매액 조회
select custid, count(*) ,sum(saleprice) from orders group by custid;
-- 19. 가격이 8000원 이상인 도서를 구매한 고객에 대하여 고객별 주문 도서의 총 수량을 구하시오. 단 , 두 권 이상 구매한 고객만 조회
select custid, count(*) as `총 수량` from orders where saleprice >= 8000 group by custid having `총 수량` >= 2;
-- 20. 날짜별 총 무개건수와 총 판매액을 조회
select orderdate, count(*) as `날짜별 판매건수` , sum(saleprice) as `총 판매액` from orders group by orderdate;
-- 21. 총 판매액이 20000원이 넘는 날짜의 총 구매건수를 조회
select orderdate, count(*) as `날짜별 판매건수` , sum(saleprice) as `총 판매액` from orders group by orderdate having `총 판매액` >= 20000;
-- 22. 가장 구매건수가 많은 날짜를 조회 구매건수가 같은 경우 가장 최근 날짜를 조회
select orderdate, count(*) as 판매건수 from orders group by orderdate order by 판매건수 desc limit 1;