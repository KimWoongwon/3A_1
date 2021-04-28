use cookdb;
SET GLOBAL log_bin_trust_function_creators = 1;

drop FUNCTION if exists Customer_Grade;
delimiter //
create FUNCTION Customer_Grade(총구매액 int) returns char(10)
begin
	case
		when (총구매액 >= 1000) then return 'VIP';
        when (총구매액 >= 500) then return 'Gold';
        when (총구매액 >= 100) then return 'Silver';
        when (총구매액 >= 1) then return 'Bronze';
        else return 'Iron';
	end case;
end //
delimiter ;
-- call Customer_Grade();

drop procedure if exists Classify_Customer;
delimiter //
create procedure Classify_Customer()
begin
	select u.userid, u.username, sum(price * amount) as 총구매액, Customer_Grade(sum(price * amount)) as 고객등급
    from buytbl b right outer join usertbl u on b.userid = u.userid
    group by u.userid, u.username order by sum(price * amount) desc;
end //
delimiter ;
call Classify_Customer();
