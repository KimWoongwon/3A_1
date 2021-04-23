-- 현재 Development 부서에서 근무하는 직원의 이름과 생년월일을 조회
-- My Work
select e.emp_no, d.dept_no, e.first_name, e.last_name, e.birth_date from employees e join dept_emp d on e.emp_no = d.emp_no
 where dept_no = (select dept_no from departments where dept_name = 'Development') and d.to_date = '9999-01-01';

-- Teacher's Work
select e.emp_no, d.dept_no, e.first_name, e.last_name, e.birth_date from employees e join dept_emp d on e.emp_no = d.emp_no
 join departments dt on d.dept_no = dt.dept_no where dt.dept_name = 'Development' and d.to_date = '9999-01-01';
 
-- name이 Christ Muchinsky 인 직원의 현재 소속 부서명과 현재 연봉
select e.emp_no, e.first_name, e.last_name, dt.dept_name, s.salary from employees e 
join dept_emp dm on e.emp_no = dm.emp_no
join departments dt on dm.dept_no = dt.dept_no 
join salaries s on e.emp_no = s.emp_no 
where e.first_name = 'Christ' and e.last_name = 'Muchinsky' and dm.to_date = '9999-01-01' and s.to_date = '9999-01-01';

-- 현재 title이 Senior Engineer 로 일하고 있는 직원의 이름과 연봉
select t.title, e.emp_no, e.first_name, e.last_name, s.salary from employees e
join titles t on e.emp_no = t.emp_no 
join salaries s on e.emp_no = s.emp_no
where t.title = 'Senior Engineer' and t.to_date = '9999-01-01' and s.to_date = '9999-01-01';

-- 부서별로 매니저가 바뀐 횟수를 조회(부서명, 역대 매니저 수)
-- My Work
select dt.dept_no, dt.dept_name, count(*) from departments dt 
join dept_manager dm on dt.dept_no = dm.dept_no 
group by dm.dept_no order by dt.dept_no;

-- Teacher's Work
select dept_no, count(*) as countmng from dept_manager group by dept_no;

select dt.dept_name, count_mng.countmng from departments dt
join (select dept_no, count(*) as countmng from dept_manager group by dept_no) as count_mng
on dt.dept_no = count_mng.dept_no;

-- 가장 자주 매니저가 바뀐 부서의 이름과 바뀐 횟수
create table MaxCntMng
(select dt.dept_name, count_mng.countmng from departments dt
join (select dept_no, count(*) as countmng from dept_manager group by dept_no) as count_mng
on dt.dept_no = count_mng.dept_no);

select dept_name, countmng from maxcntmng 
where countmng = (select max(countmng) from maxcntmng);

-- 각 부서별 최고 연봉을 받는 직원들의 이름과 연봉
create table emp_salary
(
select de.dept_no, de.emp_no, s.salary from dept_emp de
join salaries s on de.emp_no = s.emp_no where s.to_date = '9999-01-01' and de.to_date = '9999-01-01'
);
create view emp_max_salary as 
(select * from emp_salary where (dept_no, salary) in 
(select dept_no, max(salary) from emp_salary group by dept_no));

select dm.dept_name, e.first_name, e.last_name, ems.salary from departments dm 
join emp_max_salary ems on dm.dept_no = ems.dept_no
join employees e on ems.emp_no = e.emp_no;

use cookdb;
alter table buytbl auto_increment = 100;
set @@auto_increment_increment = 1;

use madang;
update book set price = price+1000 where publisher = '이상미디어';

create table book2
(select * from book);

update book set price = 
(select price from book2 where publisher = 'person')
where publisher = '굿스포츠';

use empolyees;
create table test1 (select * from employees);
create table test2 (select * from employees);
create table test3 (select * from employees);

delete from test1;
drop table test2;
truncate table test3;

use cookdb;
create table membertbl
(
select userid, username, addr from usertbl limit 3
);
alter table membertbl add constraint pk_membertbl primary key(userid);
alter table membertbl drop primary key;

create table buytbl2 (select * from buytbl);
alter table buytbl2 add constraint pk_buytbl2 primary key(num);

alter table buytbl2 add constraint fk_buytbl2 foreign key(userid)
references usertbl(userid) on update cascade on delete cascade;

alter table buytbl2 drop foreign key fk_buytbl2;
alter table buytbl2 drop index fk_buytbl2;

use madang;
alter table book add column isbn varchar(13); 
alter table book modify column isbn int;
alter table book drop column isbn;





