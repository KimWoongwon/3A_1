CREATE DATABASE  IF NOT EXISTS `cookdb` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci */ /*!80016 DEFAULT ENCRYPTION='N' */;
USE `cookdb`;
-- MySQL dump 10.13  Distrib 8.0.20, for Win64 (x86_64)
--
-- Host: localhost    Database: cookdb
-- ------------------------------------------------------
-- Server version	8.0.20

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `buytbl`
--

DROP TABLE IF EXISTS `buytbl`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `buytbl` (
  `num` int NOT NULL AUTO_INCREMENT,
  `userid` char(8) NOT NULL,
  `prodname` char(6) NOT NULL,
  `groupname` char(4) DEFAULT NULL,
  `price` int NOT NULL,
  `amount` smallint NOT NULL,
  PRIMARY KEY (`num`),
  KEY `buytbl_ibfk_1` (`userid`),
  CONSTRAINT `buytbl_ibfk_1` FOREIGN KEY (`userid`) REFERENCES `usertbl` (`userid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `buytbl`
--

LOCK TABLES `buytbl` WRITE;
/*!40000 ALTER TABLE `buytbl` DISABLE KEYS */;
INSERT INTO `buytbl` VALUES (1,'KHD','운동화',NULL,30,2),(2,'KHD','노트북','전자',1000,1),(3,'KYM','모니터','전자',200,1),(4,'PSH','모니터','전자',200,5),(5,'KHD','청바지','의류',50,3),(6,'PSH','메모리','전자',80,10),(7,'KJD','책','서적',15,5),(8,'LHJ','책','서적',15,2),(9,'LHJ','청바지','의류',50,1),(10,'PSH','운동화',NULL,30,2),(11,'LHJ','책','서적',15,1),(12,'PSH','운동화',NULL,30,2);
/*!40000 ALTER TABLE `buytbl` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `usertbl`
--

DROP TABLE IF EXISTS `usertbl`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `usertbl` (
  `userid` char(8) NOT NULL,
  `username` char(10) NOT NULL,
  `birthyear` int NOT NULL,
  `addr` char(2) NOT NULL,
  `mobile` char(13) DEFAULT NULL,
  `height` smallint DEFAULT NULL,
  `joindate` date DEFAULT NULL,
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `usertbl`
--

LOCK TABLES `usertbl` WRITE;
/*!40000 ALTER TABLE `usertbl` DISABLE KEYS */;
INSERT INTO `usertbl` VALUES ('KHD','강호동',1970,'경북','010-2222-2222',182,'2007-07-07'),('KJD','김제동',1974,'경남',NULL,173,'2013-03-03'),('KKJ','김국진',1965,'서울','010-3333-3333',171,'2009-09-09'),('KYM','김용만',1967,'서울','010-4444-4444',177,'2015-05-05'),('LHJ','이휘재',1972,'경기','010-8888-8888',180,'2006-04-04'),('LKK','이경규',1960,'경남','010-9999-9999',170,'2004-12-12'),('NHS','남희석',1971,'충남','010-6666-6666',180,'2017-04-04'),('PSH','박수홍',1970,'서울','010-0000-0000',183,'2012-05-05'),('SDY','신동엽',1971,'경기',NULL,176,'2008-10-10'),('YJS','유재석',1972,'서울','010-1111-1111',178,'2008-08-08');
/*!40000 ALTER TABLE `usertbl` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping events for database 'cookdb'
--

--
-- Dumping routines for database 'cookdb'
--
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-03-15 13:22:57
