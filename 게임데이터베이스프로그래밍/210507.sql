-- 커서를 이용한 회원 테이블의 고객 등급 열에
-- 최우수 고객, 우수 고객, 일반 고객, 유령고객등의 값을 입력하는 프로시저
use cookdb;

alter table usertbl add grade varchar(10);

drop procedure if exists cursorgradeproc;
delimiter //
create procedure cursorgradeproc()
begin

	declare uid varchar(5);
    declare usum int;
    declare u_grade varchar(10);
	
	declare endofrow boolean default false;
    declare usercursor cursor for 
		select u.userid, sum(b.price * b.amount) from buytbl b right outer join usertbl u
		on b.userid = u.userid group by u.userid;
    
    declare continue handler for not found
    set endofrow = true;
    
    open usercursor;
    cursor_loop:loop
		fetch usercursor into uid, usum;
        
        if endofrow then
			leave cursor_loop;
		end if;
        
        case
			when (usum >= 1000) then set u_grade = 'VIP';
			when (usum >= 500) 	then set u_grade = 'Gold';
			when (usum >= 100) 	then set u_grade = 'Silver';
			when (usum >= 1) 	then set u_grade = 'Bronze';
			else set u_grade = 'Iron';
		end case;
		
        update usertbl u set grade = u_grade where u.userid = uid;
        
	end loop cursor_loop;
	close usercursor;

end//
delimiter ;

call cursorgradeproc();

set @u_grade = 'Bronze';
prepare search_user from 'select * from usertbl where grade = ?';
execute search_user using @u_grade;

set @t_name = concat('usertbl_', date_format(curdate(), '%Y_%m_%d'));
select @t_name;
set @create_query = concat('create table ', @t_name, ' (id int, txt varchar(10))');
select @create_query;

prepare search_user from @create_query;
execute search_user;

-- 고객별로 할인률 구하기 (매달 업데이트) 가입일, 구매실적
-- update_discount_rate
use cookdb;

drop procedure if exists update_discount_rate;
delimiter //
create procedure update_discount_rate()
begin
	declare _id char(5);
    declare _date date;
    declare _grade char(10);
    declare _discount decimal(0.00);

	declare endofrow boolean default false;
    
    declare usercursor cursor for 
		select userid, joindate, grade from usertbl;
    
    declare continue handler for not found
    set endofrow = true;

	-- create & alter table
    set @tbl_name = concat('user_discount_tbl_', date_format(curdate(), '%Y_%m_%d'));
    
    set @drop_tbl = concat('drop table if exists ', @tbl_name);
    prepare drop_tbl from @drop_tbl;
    execute drop_tbl;
    
    set @create_tbl = concat('create table ', @tbl_name, ' ( select * from usertbl )');
    prepare create_tbl from @create_tbl;
    execute create_tbl;
    
    set @alter_tbl = concat('alter table ', @tbl_name, ' add discount decimal(0.00)');
    prepare alter_tbl from @alter_tbl;
	execute alter_tbl;
	
    open usercursor;
    cursor_loop:loop
		fetch usercursor into _id, _date, _grade;
        
        if endofrow then
			leave cursor_loop;
		end if;
        
        case
			when (year(curdate()) - year(_date) >= 20 or _grade = 'VIP') then set _discount = 10.0;
			when (year(curdate()) - year(_date) >= 16 or _grade = 'Gold') then set _discount = 7.0;
			when (year(curdate()) - year(_date) >= 13 or _grade = 'Silver') then set _discount = 5.0;
			when (year(curdate()) - year(_date) >= 10 or _grade = 'Bronze') then set _discount = 3.0;
			else set _discount = 0.0;
		end case;
		
        set @update_tbl = concat('update ', @tbl_name, ' u set discount = ', _discount, " where u.userid = '",  _id, "'");
 		prepare update_tbl from @update_tbl;
		execute update_tbl;

	end loop cursor_loop;
	close usercursor;
    
    
end//
delimiter ;

call update_discount_rate();
SELECT * FROM cookdb.user_discount_tbl_2021_05_07;


