create schema gamedb;
use gamedb;

-- 유저의 가입정보, 가입날짜등의 데이터가 들어갈 테이블
create table usertbl
(
serialnum int auto_increment primary key,
userid char(12) not null,
userpw char(15) not null,
join_time timestamp not null default current_timestamp on update current_timestamp 
);

-- 유저가 생성한 캐릭터의 직업, 닉네임, 레벨, 생성시기등의 정보를 가지고있는 테이블
create table chardata
(
serialnum int auto_increment primary key,
usercode int not null,
nickname char(12) not null,
job int not null,
char_level int not null,
create_date date not null,
foreign key(usercode) references usertbl(serialnum) on update cascade on delete cascade
);

-- 아이템 정보 테이블
create table itemdata
(
serialnum int auto_increment primary key,
itemgroup char(10),
itemname char(20) not null
);

-- 인벤토리 정보 테이블 (어떤 캐릭터가 어떤 아이템을 몇개 가지고 있는지)
create table inventory
(
itemindex int auto_increment primary key,
charcode int not null,
itemcode int not null,
item_count int not null,
foreign key(charcode) references chardata(serialnum) on update cascade on delete cascade,
foreign key(itemcode) references itemdata(serialnum) on update cascade on delete cascade
);

-- Alter문을 이용하여 테이블의 제약조건을 변경하는 Query문
-- int 형의 데이터를 char형의 데이터로 변경
alter table chardata modify job char(6);

-- insert usertbl
insert into usertbl values(null, 'KWW', 'aa11', '2019-03-17 21:48:25');
insert into usertbl values(null, 'UJS', 'q1w2', '2018-05-24 19:54:34');
insert into usertbl values(null, 'JJH', 'zdaw45', '2020-07-09 22:29:14');
insert into usertbl(userid, userpw) values('LGE', 'z48a6q');

-- insert itemdata
insert into itemdata values(null, '재료', '가죽');
insert into itemdata values(null, '재료', '결이 고운 가죽');
insert into itemdata values(null, '재료', '옷감');
insert into itemdata values(null, '재료', '고운 빛깔 가죽');
insert into itemdata values(null, '재료', '목재');
insert into itemdata values(null, '재료', '결이 고운 목재');
insert into itemdata values(null, '재료', '석재');
insert into itemdata values(null, '재료', '강도 높은 석재');
insert into itemdata values(null, '재료', '철 광석');
insert into itemdata values(null, '재료', '철 주괴');
insert into itemdata values(null, '재료', '구리 광석');
insert into itemdata values(null, '재료', '구리 주괴');
insert into itemdata values(null, '재료', '아키움 광석');
insert into itemdata values(null, '재료', '아키움 주괴');

insert into itemdata values(null, '작은 치유 물약', '한여름밤의 포근한 꿈');
insert into itemdata values(null, '작은 치유 물약', '노래하는 불사조');
insert into itemdata values(null, '작은 치유 물약', '황금 평원의 신기루');
insert into itemdata values(null, '치유 물약', '누이 여신의 백합');
insert into itemdata values(null, '치유 물약', '운명을 바꾸는 의지');
insert into itemdata values(null, '치유 물약', '끝없는 생명의 수레바퀴');
insert into itemdata values(null, '작은 명상 물약', '벽난로의 푸른 불먼지');
insert into itemdata values(null, '작은 명상 물약', '마리아노플의 분수');
insert into itemdata values(null, '작은 명상 물약', '완전무결한 영원의 기억');
insert into itemdata values(null, '명상 물약', '고뇌하는 루키우스');
insert into itemdata values(null, '명상 물약', '시간을 걷는 자의 슬픔');
insert into itemdata values(null, '명상 물약', '고요한 폭풍의 눈');

-- insert chardata
insert into chardata values(null, 1, '프삭', '창술사', 25, '2019-03-17');
insert into chardata values(null, 1, '머그컵장인', '워로드', 40, '2019-05-20');
insert into chardata values(null, 1, '로스트아키', '아르카나', 1, curdate());
insert into chardata values(null, 2, '윤루트', '호크아이', 38, '2018-05-27');
insert into chardata values(null, 2, '빅헤드', '건슬링어', 27, '2018-08-10');
insert into chardata values(null, 2, '코아', '리퍼', 41, '2019-01-24');
insert into chardata values(null, 3, '노돌리', '스트라이커', 23, '2020-08-01');
insert into chardata values(null, 3, '쫀득', '데빌헌터', 50, '2020-10-13');
insert into chardata values(null, 3, '김블루', '호크아이', 31, '2021-02-20');
insert into chardata values(null, 4, '서새봄', '바드', 23, '2021-04-10');
insert into chardata values(null, 4, '소니쇼', '서머너', 1, '2021-04-10');

-- insert inventory
insert into inventory values(null, 1, 1, 20);
insert into inventory values(null, 1, 2, 1);
insert into inventory values(null, 1, 5, 20);
insert into inventory values(null, 1, 6, 1);
insert into inventory values(null, 1, 16, 200);
insert into inventory values(null, 1, 20, 100);
insert into inventory values(null, 3, 16, 200);
insert into inventory values(null, 4, 20, 100);
insert into inventory values(null, 6, 24, 200);
insert into inventory values(null, 5, 26, 100);
insert into inventory values(null, 8, 14, 20);
insert into inventory values(null, 8, 13, 3);
insert into inventory values(null, 2, 11, 157);
insert into inventory values(null, 2, 17, 15);
insert into inventory values(null, 7, 12, 30);
insert into inventory values(null, 9, 7, 200);
insert into inventory values(null, 10, 8, 3);
insert into inventory values(null, 3, 6, 500);
insert into inventory values(null, 3, 5, 34);
insert into inventory values(null, 7, 3, 300);
insert into inventory values(null, 6, 4, 14);
insert into inventory values(null, 4, 2, 100);
insert into inventory values(null, 9, 6, 57);

-- and | or로 구성되는 where절을 포함한 Select문(단일)
-- 직업이 창술사 혹은 호크아이인 캐릭터의 정보 출력
select * from chardata where job = '창술사' or job = '호크아이';

-- group by와 집계함수가 포함된 select문(단일)
-- 각 유저별 생성한 캐릭터의 갯수를 출력
select usercode, count(*) from chardata group by usercode;

-- 집계함수가 사용된 독립 Sub query를 포함하는 중첩 Query문(Select)
-- 레벨이 가장 낮은 캐릭터의 정보 출력
select * from chardata where char_level in (select min(char_level) from chardata);

-- 집계함수가 사용된 상관 Sub query를 포함하는 중첩 Query문(Select)
-- 보유중인 아이템의 갯수가 총 300개가 넘는 캐릭터들의 정보 출력
select * from chardata where serialnum in
(select i1.charcode from inventory i1 where (select sum(i2.item_count) from inventory i2 where i1.charcode = i2.charcode) >= 300);

-- 테이블 형태의 결과를 만드는 Sub query를 포함한 중첩 Query문(Join)
-- 캐릭터별 보유중 아이템중 itemgroup이 '재료'인 아이템의 종류 현황
select c.serialnum, c.nickname, groupcount.재료보유수 from chardata c 
join (select charcode, count(*) as 재료보유수 from (select iv.itemindex, iv.charcode, id.itemgroup, id.itemname, iv.item_count 
from inventory iv join itemdata id on iv.itemcode = id.serialnum) as inventory_info where inventory_info.itemgroup = '재료' group by charcode) as groupcount 
on c.serialnum = groupcount.charcode;

-- Query의 결과를 테이블로 저장하고 이 테이블을 사용한 Query문(Join)
-- 보유중 아이템중 '작은 치유 물약'종류의 아이템을 보유한 캐릭터 정보 및 아이템의 이름과 아이템의 갯수 출력
create table inventory_info
(
select iv.itemindex, iv.charcode, id.itemgroup, id.itemname, iv.item_count 
from inventory iv join itemdata id on iv.itemcode = id.serialnum
);

select c.serialnum, c.nickname, c.char_level, ii.itemname, ii.item_count from chardata c 
join inventory_info ii on c.serialnum = ii.charcode
where ii.itemgroup = '작은 치유 물약';

-- View를 포함한 중첩 Query문(Join) 
-- '결이 고운'이 이름에 포함된 아이템을 가진 캐릭터의 정보 출력
create view material_info as
(
select * from inventory_info where itemgroup = '재료'
);

select c.serialnum, c.nickname, c.char_level, mi.itemname, mi.item_count from chardata c
join material_info mi on c.serialnum = mi.charcode
where mi.itemname like '결이 고운%';

-- Update문을 사용하여 레코드를 변경하는 Query문
-- 직업이 아르카나인 캐릭터의 레벨을 27로 변경
update chardata set char_level = 27 where job = '아르카나';
-- Delete문을 사용하여 레코드를 삭제하는 Query문 (Where절 필수)
-- 캐릭터의 레벨이 1인 캐릭터의 정보 삭제
delete from chardata where char_level = 1;
