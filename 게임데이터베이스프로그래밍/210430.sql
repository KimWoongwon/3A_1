-- book table에 저장된 도서들의 평균 값을 반환하는 프로시저
use madang;
drop procedure if exists avg_book_price;
delimiter //
create procedure avg_book_price(out avgprice int)
begin
	select cast(avg(price) as signed int) into avgprice from book;
end //
delimiter ;

call avg_book_price(@avg_price);
select @avg_price;

-- 각 출판사의 최고가(max), 최저가(min)중 하나를 입력하면 그에 해당하는 책의 이름과 가격을 리턴하는 프로시저 구현
-- call getbookprice('나무수', 'max', @bkname, @bkprice);

use madang;
drop procedure if exists get_book_price;
delimiter //
create procedure get_book_price(in pub varchar(20), in minmax varchar(5), out bkname varchar(20), out bkprice int)
begin
	declare err char(6) default '잘못된 입력';
    declare tempprice int default -1;
    case
		when minmax = 'min' then
			set tempprice = (select min(price) from book2 where publisher = pub);
        when minmax = 'max' then
			set tempprice = (select max(price) from book2 where publisher = pub);
		else 
			set tempprice = -1;
	end case;
    
    if tempprice != -1 then
		select bookname into bkname from book2 where (price = tempprice and publisher = pub);
        select price into bkprice from book2 where (price = tempprice and publisher = pub);
    else
		set bkname = 'input error';
        set bkprice = tempprice;
    end if;
    
end //
delimiter ;

call get_book_price('굿스포츠', 'min', @asd, @bkprice);
select @asd, @bkprice;

-- groupname 을 입력하여 그 group에 해당하는 제품의 총 판매량을 구하는 프로시저 구현 (없는 group 명에 대한 오류 처리)
-- call totalpriceproc('서적', @total);
use cookdb;
drop procedure if exists totalpriceproc;
delimiter //
create procedure totalpriceproc(in gro varchar(5), out _amount int)
begin
	declare temp int default -1;
    set temp = (select count(*) from buytbl group by gro);
    
    if temp != 0 then
		set _amount = (select sum(price*amount) from buytbl where groupname = gro);
    else
		set _amount = 999;
    end if;
    
end //
delimiter ;

call totalpriceproc("악세사리", @total);
select @total;

drop procedure if exists Teachers_totalpriceproc;
delimiter //
create procedure Teachers_totalpriceproc(in group_name varchar(5), out total_price int)
begin
	declare temp varchar(10);
    DECLARE exit handler for not found
		begin
			show warnings;
			select '없는 그룹명입니다.' as '메시지';
			rollback;
		end;
	-- select groupname into temp from buytbl where groupname = group_name;
	select sum(price*amount) into total_price from buytbl where groupname = temp;

end //
delimiter ;

call Teachers_totalpriceproc("서적", @total_price);
select @total_price;

