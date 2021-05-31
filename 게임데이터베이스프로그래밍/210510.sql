use cookdb;

drop procedure if exists update_discount_rate;
delimiter //
create procedure update_discount_rate()
begin
	declare tname varchar(20);
    declare u_id varchar(10);
    declare jdate date;
    declare total int;
    declare t_month int;
    declare rate dec(3,2);
    
    declare endofrow boolean default false;
    declare usercursor cursor for 
		select u.userid, u.joindate, sum(b.price * b.amount) from usertbl u
        right outer join buytbl b on b.userid = u.userid group by u.userid;
        
	declare continue handler for not found
		set endofrow = true;
    
    set tname = concat('tbl_', date_format(curdate(), '%Y_%m_%d'));
    set @drop_query = concat('drop table if exists ', tname);
    set @create_query = concat('create table ', tname, ' (userid varchar(10), discount_rate dec(3,2))');
    
    prepare drop_query from @drop_query;
    execute drop_query;
    
    prepare create_query from @create_query;
    execute create_query;
    
    open usercursor;
		cursor_loop:Loop
			fetch usercursor into u_id, jdate, total;
			
			if endofrow then 
				leave cursor_loop;
			end if;
			
            set t_month = datediff(curdate(), jdate) / 30;
			case
				when t_month >= 150 or total >= 1000 then set rate = 0.15;
				when t_month >= 100 or total >= 500 then set rate = 0.10;
				when t_month >= 50 or total >= 200 then set rate = 0.05;
				else set rate = 0.0;
            end case;
            
            set @insert_query = concat('insert into ', tname, ' values("', u_id,'",', rate,')');
            prepare insert_query from @insert_query;
            execute insert_query;
            
		end loop cursor_loop;
	close usercursor;
end //
delimiter ;

call update_discount_rate();

-- ===================================================================================================

SET GLOBAL log_bin_trust_function_creators = 1;
drop function if exists set_rate;
delimiter // 
create function set_rate(jdate date, total int) returns dec(3,2)
begin
	declare t_month int;
    declare rate dec(3,2);
	set t_month = datediff(curdate(), jdate) / 30;
		case
			when t_month >= 150 or total >= 1000 then set rate = 0.15;
			when t_month >= 100 or total >= 500 then set rate = 0.10;
			when t_month >= 50 or total >= 200 then set rate = 0.05;
			else set rate = 0.0;
		end case;
        
	return rate;
end //
delimiter ;



drop procedure if exists update_discount_rate2;
delimiter //
create procedure update_discount_rate2()
begin
	declare tname varchar(20);
               
	set tname = concat('tbl_', date_format(curdate(), '%Y_%m_%d'));
    set @drop_query = concat('drop table if exists ', tname);
    set @create_query = concat('create table ', tname, 
    ' (select u.userid, set_rate(u.joindate, sum(b.price * b.amount)) from usertbl u
        right outer join buytbl b on b.userid = u.userid group by u.userid)');
    
    prepare drop_query from @drop_query;
    execute drop_query;
    
    prepare create_query from @create_query;
    execute create_query;
    
 end //
delimiter ;

call update_discount_rate2();

create table if not exists testTBL (id int, txt varchar(10));
insert into testtbl values(1, 'exid');
insert into testtbl values(2, '블랙핑크');
insert into testtbl values(3, '에이핑크');

drop trigger if exists testtrg;
delimiter //
create trigger testtrg
after delete on testtbl
for each row
begin
	set @msg = '가수 그룹이 삭제됨';
end//
delimiter ;

set @msg = "";
insert into testtbl values(4, '여자친구');
select @msg;

update testtbl set txt = '레드벨벳' where id = 3;
select @msg;

delete from testtbl where id = 4;
select @msg;

USE cookDB;
DROP TABLE buyTBL; -- 구매 테이블은 실습에 필요 없으므로 삭제 
CREATE TABLE backup_userTBL 
( userID char(8) NOT NULL, 
  userName varchar(10) NOT NULL, 
  birthYear int NOT NULL, 
  addr char(2) NOT NULL, 
  mobile char(15), 
  height smallint, 
  mDate date, 
  modType char(2), -- 변경된 유형(‘수정’ 또는 ‘삭제’) 
  modDate date, -- 변경된 날짜 
  modUser varchar(256) -- 변경한 사용자 
);

drop trigger if exists backuptbl_update_trg;
delimiter //
create trigger backuptbl_update_trg
after update on usertbl for each row
begin 
	insert into backup_usertbl values(old.userid, old.username, old.birthyear, old.addr, old.mobile, old.height, old.joindate, '수정', curdate(), current_user());
end // 
delimiter ;

drop trigger if exists backuptbl_delete_trg;
delimiter //
create trigger backuptbl_delete_trg
after delete on usertbl for each row
begin
	insert into backup_usertbl values(old.userid, old.username, old.birthyear, old.addr, old.mobile, old.height, old.joindate, '삭제', curdate(), current_user());
end // 
delimiter ;


update usertbl set addr = '제주' where userid = 'KHD';
delete from usertbl where height >= 180;
select * from backup_usertbl;

drop trigger if exists inserttrg;
delimiter //
create trigger inserttrg
before insert on usertbl for each row
begin
	if new.birthyear < 1900 then
		set new.birthyear = 0;
	elseif new.birthyear > year(curdate()) then
		set new.birthyear = year(curdate());
	end if;
end //
delimiter ;

drop trigger if exists inserttrg2;
delimiter //
create trigger inserttrg2
before insert on usertbl for each row
begin
	signal sqlstate '45000'
		set message_text = '데이터의 입력을 시도했습니다.', mysql_errno = 1001;
end //
delimiter ;

insert into usertbl values('aaa', '에이', 1877, '서울', '01011111111', 181, '2019-12-25');
insert into usertbl values('bbb', '비비', 2025, '서울', '01011111111', 181, '2019-12-25');
select * from usertbl;

drop trigger if exists test_multitrg1;
delimiter //
create trigger test_multitrg1
before insert on usertbl for each row
begin
	set @msg = concat('userid = ', new.userid);	
end//
delimiter ;

drop trigger if exists test_multitrg2;
delimiter //
create trigger test_multitrg2
before insert on usertbl for each row
follows test_multitrg1
begin
	set @msg = concat(@msg, '에 대한 입력이 발생했습니다.');	
    signal sqlstate '45000'	set message_text = @msg;
end//
delimiter ;

insert into usertbl values('abc', '에비씨', 1977, '서울', '01012345678', 179, '2019-12-25');

create database triggerdb;
use triggerdb;

CREATE TABLE orderTBL -- 구매 테이블 
( 
  orderNo INT AUTO_INCREMENT PRIMARY KEY, -- 구매 일련번호 
  userID VARCHAR(5), -- 구매한 회원 아이디 
  prodName VARCHAR(5), -- 구매한 물건 
  orderamount INT -- 구매한 개수 
);
CREATE TABLE prodTBL -- 물품 테이블 
( 
  prodName VARCHAR(5), -- 물건 이름 
  account INT -- 남은 물건 수량 
);
CREATE TABLE deliverTBL -- 배송 테이블 
( 
  deliverNo INT AUTO_INCREMENT PRIMARY KEY, -- 배송 일련번호 
  prodName VARCHAR(5), -- 배송할 물건 
  account INT -- 배송할 물건 개수 
);

INSERT INTO prodTBL VALUES ('사과', 100);
INSERT INTO prodTBL VALUES ('배', 100);
INSERT INTO prodTBL VALUES ('귤', 100);

drop trigger if exists ordertrg;
delimiter //
create trigger ordertrg
after insert on ordertbl for each row
begin
	update prodTBL set account = account - new.orderamount where prodName = new.prodName;
end // 
delimiter ;

drop trigger if exists prodtrg;
delimiter //
create trigger prodtrg
after update on prodtbl for each row
begin
	declare orderAmount int;
    -- 주문 개수 = (변경 전 개수 - 변경 후 개수)
	set orderAmount = old.account - new.account;
    insert into delivertbl(prodname, account) values(new.prodname, orderAmount);
end //
delimiter ;

insert into ordertbl values(null, 'JOHN', '배', 5);
select * from delivertbl;
select * from ordertbl;
select * from prodtbl;
