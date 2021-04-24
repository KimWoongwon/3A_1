use madang;
-- 출판사 별로 출판사의 평균 도서 가격보다 비싼 도서를 구입한 고객정보 조회
-- (custid, name, bookname, price, publisher, avg_price)
with avg_price(publisher, avgprice) as
(select publisher, truncate(avg(price), 0) from book group by publisher),
gt_avg_price(bookid, bookname, price, publisher, avgprice) as
(select b.bookid, b.bookname, b.price, b.publisher, ap.avgprice 
from book b join avg_price ap on b.publisher = ap.publisher where b.price >= ap.avgprice)

select c.custid, c.name, gt.bookname, gt.price, gt.publisher, gt.avgprice
from customer c join orders od on c.custid = od.custid
join gt_avg_price gt on od.bookid = gt.bookid;

-- 각 부서별 최고 연봉을 받는 직원들을 조회(부서명, 직원명, 연봉)
use employees;
with emp_salary(dept_no, emp_no, salary) as
(
	select de.dept_no, de.emp_no, s.salary from dept_emp de join salaries s on de.emp_no = s.emp_no 
	where s.to_date = '9999-01-01' and de.to_date = '9999-01-01'
),
emp_max_salary(dept_no, emp_no, salary) as
(
	select * from emp_salary where (dept_no, salary) in 
	(select dept_no, max(salary) from emp_salary group by dept_no)
)
select dm.dept_name, concat(e.first_name, ' ', e.last_name) as Fullname, ems.salary from departments dm 
join emp_max_salary ems on dm.dept_no = ems.dept_no
join employees e on ems.emp_no = e.emp_no;

drop procedure if exists userpro1;
delimiter //
create procedure userpro1()
begin
	declare var1 int;
	set var1 = 100;
    
    if var1 = 100 then
		select '100입니다';
	else 
		select '100이 아닙니다';
	end if;
end //
delimiter ;
call userpro1();

drop procedure if exists userpro2;
delimiter //
create procedure userpro2()
begin
	declare hiredate date;
    declare currdate date;
    declare days int;
    
    select hire_date into hiredate from employees where emp_no = 10001;
    
    set currdate = curdate();
    set days = datediff(currdate, hiredate);
    
    if(days/365) >= 5 then
		select concat('입사한지', days, '일이나 지났습니다. 축하합니다') as '메시지';
	else
		select concat('입사한지', days, '일밖에 안되었네요. 열심히 일하세요.') as '메시지';
	end if;
end //
delimiter ;
call userpro2();

drop procedure if exists userpro3;
delimiter //
create procedure userpro3()
begin
	declare point int;
    declare credit char(1);
    set point = 77;
    
    case 
		when point >= 90 then
			set credit = 'A';
		when point >= 80 then
			set credit = 'B';
		when point >= 70 then
			set credit = 'C';
		when point >= 60 then
			set credit = 'D';
		else
			set credit = 'F';
	end case;
    
    select concat('취득점수 : ', point), concat('학점 : ', credit);
    
end //
delimiter ;
call userpro3();

drop procedure if exists userpro4;
delimiter //
create procedure userpro4(in point int)
begin
	declare credit char(1);
       
    case 
		when point >= 90 then
			set credit = 'A';
		when point >= 80 then
			set credit = 'B';
		when point >= 70 then
			set credit = 'C';
		when point >= 60 then
			set credit = 'D';
		else
			set credit = 'F';
	end case;
    
    select concat('취득점수 : ', point), concat('학점 : ', credit);
    
end //
delimiter ;
call userpro4(80);
