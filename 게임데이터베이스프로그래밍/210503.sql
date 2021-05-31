-- 부서명을 입력해서 그 부서에서 연봉이 가장 높은 사람을 구하는 프로시저 구현
-- call maxsalaryproc('Customer Service', @empname, @salary);
use employees;
drop procedure if exists maxsalaryproc;
delimiter //
create procedure maxsalaryproc(in dp_name varchar(30), out emp_name varchar(30), out _salary int)
begin
	declare dpnum varchar(5);
    declare empnum varchar(6);
    
    declare exit handler for not found
		begin
			show warnings;
			select '없는 부서명입니다.' as '메시지';
			rollback;
		end;
    select dept_no into dpnum from departments where dept_name = dp_name;
    
    with test1(emp_no) as 
    (select emp_no from dept_emp where dept_no = dpnum and to_date = '9999-01-01'),
    
    test2(emp_no, salary) as
    (select t1.emp_no, s.salary from test1 t1 join salaries s on t1.emp_no = s.emp_no where to_date = '9999-01-01'),
    
    test3(emp_no, salary) as
    (select emp_no, salary from test2 where salary = (select max(salary) from test2))
    
    select concat(e.first_name, " ", e.last_name), t3.salary into emp_name, _salary from test3 t3
    join employees e on t3.emp_no = e.emp_no;
    
    -- select max(salary) into _salary from emp_salary group by dept_no having dept_no = dpnum;
-- 	select emp_no into empnum from emp_salary group by dept_no having dept_no = dpnum;

-- 	select concat(first_name, " ", last_name) into emp_name from employees where emp_no = empnum;
     
end //
delimiter ;

call maxsalaryproc('Customer Service', @empname, @salary);
select @empname, @salary;

set global log_bin_trust_function_creators = 1;

use cookdb;
drop function if exists getAgefunc;
delimiter //
create function getAgefunc(byear int) returns int
begin
	declare age int;
    
    set age = year(curdate()) - byear;
    return age;
end //
delimiter ;

select getAgefunc(1997);
select userid, username, getAgefunc(birthyear) as 만나이 from usertbl;

drop procedure if exists functestproc;
delimiter //
create procedure functestproc()
begin
	select userid, username, getAgefunc(birthyear) as 만나이 from usertbl;
end //
delimiter ;

call functestproc();

-- 판매된 도서에 대한 이익을 계산하는 함수
-- (가격이 20000원 이상이면 10%, 20000원 미만이면 5%)

use madang;
drop function if exists fnc_Interest;
delimiter //
create function fnc_Interest(saleprice int) returns decimal(6,2)
begin
	declare myinterest decimal(6,2);
    
    if saleprice >= 20000 then
		set myinterest = saleprice * 0.1;
	else
		set myinterest = saleprice * 0.05;
	end if;
    
    return myinterest;
end //
delimiter ;

select custid, orderid, saleprice, fnc_Interest(saleprice) as 이익 from orders;

-- 커서 프로시저
use cookdb;
drop procedure if exists usercursorproc;
delimiter //
create procedure usercursorproc()
begin
	declare uheight int;
    declare cnt int default 0;
	declare total int default 0;
    
    declare endofrow boolean default false;
    
    declare usercursor cursor for 
		select height from usertbl;
        
	declare continue handler for not found
    set endofrow = true;
    
    open usercursor;
    cursor_loop:loop
		fetch usercursor into uheight;
        
        if endofrow then
			leave cursor_loop;
		end if;
        
        set cnt = cnt+1;
        set total = total + uheight;
	end loop cursor_loop;
    
    select concat('평균 키 : ', (total/cnt));
    close usercursor;
end//
delimiter ;

call usercursorproc();


