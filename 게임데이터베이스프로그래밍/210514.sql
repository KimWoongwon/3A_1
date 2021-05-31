use madang;
create table orders_2 (select * from orders);

-- 1. madang.orders에 구매한 서적의 개수를 입력할 필드를 추가한다.(기본 값 1)
alter table orders drop column amount;
alter table orders add column amount int default 1 not null;

-- 2. madang.book에 각 서적에 대해 입고된 서적의 개수가 들어 갈 필드를 추가한다.(기본 값 100)
alter table book add column amount int default 100;

-- 3. madang.customer에 고객등급을 입력할 필드를 추가한다.
alter table customer add column grade varchar(10) not null;

-- 4. 고객등급변경내역 테이블을 생성한다.(일련번호,고객아이디, 변경날짜, 이전등급, 변경등급)
create table changement 
	(
		serialid int auto_increment primary key not null,
        userid int not null,
        changed_date date not null,
        p_grade varchar(10) not null,
        f_grade varchar(10) not null
	);
        
-- 5. 총 구입액을 기준(기준액 자유)으로 고객등급을 일괄처리하는 프로시저를 생성한다.(vip, gold, silver, bronze)
drop procedure if exists Update_Grade_proc;
delimiter //
create procedure Update_Grade_proc()
begin
	declare uid varchar(5);
    declare usum int;
    declare u_grade varchar(10);
	
	declare endofrow boolean default false;
    declare usercursor cursor for 
		select cs.custid, sum(od.saleprice * od.amount) from orders od right outer join customer cs
		on od.custid = cs.custid group by cs.custid;
    
    declare continue handler for not found
    set endofrow = true;
    
    open usercursor;
    cursor_loop:loop
		fetch usercursor into uid, usum;
        
        if endofrow then
			leave cursor_loop;
		end if;
        
        case
			when (usum >= 50000) 	then set u_grade = 'VIP';
			when (usum >= 30000) 	then set u_grade = 'Gold';
			when (usum >= 15000) 	then set u_grade = 'Silver';
			when (usum >= 10000) 	then set u_grade = 'Bronze';
			else set u_grade = 'Iron';
		end case;
		
        update customer cs set grade = u_grade where cs.custid = uid;
        
	end loop cursor_loop;
	close usercursor;

end//
delimiter ;

call Update_Grade_proc();

-- 6. 고객등급이 변경되면 변경정보를 고객등급변경내역 테이블에 추가하는 트리거를 생성하고 고객등급을 일괄처리 프로시저를 실행한다.
drop trigger if exists grade_update_trg;
delimiter //
create trigger grade_update_trg
after update on customer for each row
begin 
	insert into changement values(null, old.custid, date_format(curdate(), '%Y-%m-%d'), old.grade, new.grade);
end // 
delimiter ;

-- 7. 고객이 서적을 주문 시 실행하는 프로시저를 생성한다.
--    - 고객 아이디, 구입할 책의 이름, 구매 개수를 입력(없는 서적에 대한 오류 처리 포함)
--    - saleprice는 고객의 등급에 따라 결정된다.
--    - order 테이블에 입력하고 주문이 입력되면 book테이블에 서적 개수를 감소시키는 트리거를 생성한다.(입력한 서적 개수보다 남은 서적 개수가 적으면 오류처리)
--    - 구매실적이 생길 때마다 해당 고객의 등급을 다시계산하여 업데이트하는 트리거를 생성한다.
drop procedure if exists Order_proc;
delimiter //
create procedure Order_proc(in _id int, in bkname varchar(30), in buy_amount int)
begin
	declare usum int;
    declare bkid int;
    declare each_price int;
    declare discount_rate dec(3,2);
    declare u_grade varchar(10);
    
    declare exit handler for not found
		begin
			show warnings;
			select '없는 책입니다.' as '메시지';
			rollback;
		end;
    select b.bookid, b.price into bkid, each_price from book b where bookname = bkname;
    select cs.grade into u_grade from customer cs where cs.custid = _id;
    
    case
		when (u_grade = 'VIP') 	then set discount_rate = 0.15;
        when (u_grade = 'Gold')	then set discount_rate = 0.10;
        when (u_grade = 'Silver') 	then set discount_rate = 0.07;
        when (u_grade = 'Bronze') 	then set discount_rate = 0.03;
        else set discount_rate = 0.0;
    end case;
    
    insert into orders values(null, _id, bkid, each_price * (1 - discount_rate), date_format(curdate(), '%Y-%m-%d'), buy_amount);
end//
delimiter ;

-- test
call Order_proc(5, '피겨 교본', 2);

drop trigger if exists trg_update_stock;
delimiter //
create trigger trg_update_stock
before insert on orders for each row
begin
	declare bkamount int;
	select b.amount into bkamount from book b where b.bookid = new.bookid;
       
    if bkamount - new.amount < 0 then 
		signal sqlstate '45000'	set message_text = '재고가 없습니다.';
	else
		update book b set b.amount = (b.amount - new.amount) where b.bookid = new.bookid;
	end if;
end //
delimiter ;

-- test
call Order_proc(1, 'Olympic Champions', 1);

drop trigger if exists trg_update_grade;
delimiter //
create trigger trg_update_grade
after insert on orders for each row
begin
	declare usum int;
    declare old_grade varchar(10);
    declare u_grade varchar(10);
	select cs.grade, sum(od.saleprice * od.amount) into old_grade, usum from orders od right outer join customer cs
		on od.custid = cs.custid where cs.custid = new.custid;
        
	case
		when (usum >= 50000) 	then set u_grade = 'VIP';
		when (usum >= 30000) 	then set u_grade = 'Gold';
		when (usum >= 15000) 	then set u_grade = 'Silver';
		when (usum >= 10000) 	then set u_grade = 'Bronze';
		else set u_grade = 'Iron';
	end case;
    
    if(old_grade != u_grade) then
		update customer cs set grade = u_grade where cs.custid = new.custid;
    end if;
    
end //
delimiter ;

call Order_proc(3, '축구의 역사', 1);

select * from orders;
select * from book;
select * from customer;
select * from changement;


