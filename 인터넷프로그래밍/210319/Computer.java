
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
	   System.out.println(com + "�� ���� " + name + " cpu " + cpu + "���� " + price + "�� ��� " + grade);
   }

 public static void main(String[] args) {

  Computer c1=new Computer();

  c1.make("�Ｚ","����",3.5, 200,'A');

  c1.pr();//�Ｚ�� ���� ����  cpu 3.5 ���� 200�� ��� A

  

  Computer c2=new Computer("LG","XNOTE");

  c2.make(3.4,210,'B');

  c2.pr();//LG ���� XNOTE cpu 3.4 ���� 210�� ��� B

  

  c1.down();

  c2.down(20);

  c1.pr();//�Ｚ�� ���� ����  cpu 3.5 ���� 195�� ��� A

  c2.pr();//LG ���� XNOTE cpu 3.4 ���� 190�� ��� B

 }//main

}//class