CREATE TABLE test.zipcode
(
    ZIPCODE    VARCHAR(5) not null, 
    SIDO       VARCHAR(100) not null, 
    GUNGU      VARCHAR(100), 
    DORO       VARCHAR(100) not null
);

load data infile "C:/ProgramData/MySQL/MySQL Server 8.0/Uploads/doro1.csv" into table zipcode character set utf8
fields terminated by ','
enclosed by '"'
lines terminated by '\n'
ignore 1 lines;

show variables like 'secure_file_priv';
