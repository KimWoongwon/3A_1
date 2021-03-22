create database cookdb;
use cookdb;
create table usertbl
(	userid char(8) not null primary key, 
	username char(10) not null, 
	birthyear int not null,
    addr char(2) not null,
    mobile char(13),
    height smallint,
    joindate date		);
    
create table buytbl
(	num int auto_increment not null primary key,
	userid char(8) not null,
	prodname char(4) not null,
	proupname char(4),
	price int not null,
	amount smallint not null,
	foreign key (userid) references usertbl(userid)		);
    
INSERT INTO usertbl VALUES ('YJS', '유재석', 1972, '서울', '010-1111-1111', 178, '2008-08-08');
INSERT INTO usertbl VALUES ('KHD', '강호동', 1970, '경북', '011-2222-2222', 182, '2007-07-07');
INSERT INTO usertbl VALUES ('KKJ', '김국진', 1965, '서울', '019-3333-3333', 171, '2009-09-09');
INSERT INTO usertbl VALUES ('KYM', '김용만', 1967, '서울', '010-4444-4444', 177, '2015-05-05');
INSERT INTO usertbl VALUES ('KJD', '김제동', 1974, '경남', NULL, 173, '2013-03-03');
INSERT INTO usertbl VALUES ('NHS', '남희석', 1971, '충남', '016-6666-6666', 180, '2017-04-04');
INSERT INTO usertbl VALUES ('SDY', '신동엽', 1971, '경기', NULL, 176, '2008-10-10');
INSERT INTO usertbl VALUES ('LHJ', '이휘재', 1972, '경기', '011-8888-8888', 180, '2006-04-04');
INSERT INTO usertbl VALUES ('LKK', '이경규', 1960, '경남', '018-9999-9999', 170, '2004-12-12');
INSERT INTO usertbl VALUES ('PSH', '박수홍', 1970, '서울', '010-0000-0000', 183, '2012-05-05');

INSERT INTO buyTBL VALUES (NULL, 'KHD', '운동화', NULL, 30, 2);
INSERT INTO buyTBL VALUES (NULL, 'KHD', '노트북', '전자', 1000, 1);
INSERT INTO buyTBL VALUES (NULL, 'KYM', '모니터', '전자', 200, 1);
INSERT INTO buyTBL VALUES (NULL, 'PSH', '모니터', '전자', 200, 5);
INSERT INTO buyTBL VALUES (NULL, 'KHD', '청바지', '의류', 50, 3);
INSERT INTO buyTBL VALUES (NULL, 'PSH', '메모리', '전자', 80, 10);
INSERT INTO buyTBL VALUES (NULL, 'KJD', '책', '서적', 15, 5);
INSERT INTO buyTBL VALUES (NULL, 'LHJ', '책', '서적', 15, 2);
INSERT INTO buyTBL VALUES (NULL, 'LHJ', '청바지', '의류', 50, 1);
INSERT INTO buyTBL VALUES (NULL, 'PSH', '운동화', NULL, 30, 2);
INSERT INTO buyTBL VALUES (NULL, 'LHJ', '책', '서적', 15, 1);
INSERT INTO buyTBL VALUES (NULL, 'PSH', '운동화', NULL, 30, 2);