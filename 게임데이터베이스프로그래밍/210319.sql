-- 부서별 매니저 수
select dept_no, count(*) as 부서별매니저수 from dept_manager group by dept_no;
-- 부서별 입사일이 1995년 이후인 사람수
select dept_no, count(*) as 인원수 from dept_emp where from_date >= '1995-01-01' group by dept_no;
-- 1985년 이후로 매니저가 2번이상 바뀐 부서
select dept_no, count(*) as 매니저수 from dept_manager where from_date >= '1985-01-01' group by dept_no having 매니저수>=3;
-- 각 직원별 최고연봉 조회
select emp_no, max(salary) from salaries group by emp_no;
-- 현재 재직중인 직원의 현재 연봉
select emp_no, salary from salaries where to_date = '9999-01-01';
