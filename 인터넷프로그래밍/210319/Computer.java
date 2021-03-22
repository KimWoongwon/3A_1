
public class Computer {

   String com, name;

   int price;   double cpu;   char grade;

   Computer()
   {
	   
   }
   Computer(String com, String name)
   {
	   this.com = com;
	   this.name = name;
   }
   
   void make(String com, String name, double cpu, int price, char grade)
   {
	   this.com = com;
	   this.name = name;
	   this.cpu = cpu;
	   this.price = price;
	   this.grade = grade;
   }
   void make(double cpu, int price, char grade)
   {
	   this.cpu = cpu;
	   this.price = price;
	   this.grade = grade;
   }
   void down()
   {
	   this.price -= 5;
   }
   void down(int discount)
   {
	   this.price -= discount;
   }
   
   void pr()
   {
	   System.out.println(com + "이 만든 " + name + " cpu " + cpu + "가격 " + price + "만 등급 " + grade);
   }

 public static void main(String[] args) {

  Computer c1=new Computer();

  c1.make("삼성","센스",3.5, 200,'A');

  c1.pr();//삼성이 만든 센스  cpu 3.5 가격 200만 등급 A

  

  Computer c2=new Computer("LG","XNOTE");

  c2.make(3.4,210,'B');

  c2.pr();//LG 만든 XNOTE cpu 3.4 가격 210만 등급 B

  

  c1.down();

  c2.down(20);

  c1.pr();//삼성이 만든 센스  cpu 3.5 가격 195만 등급 A

  c2.pr();//LG 만든 XNOTE cpu 3.4 가격 190만 등급 B

 }//main

}//class