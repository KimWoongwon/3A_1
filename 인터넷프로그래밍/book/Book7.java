import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class Book7 extends JFrame  
{
	JLabel la1,la2,la3,la4; 
	JRadioButton cb1,cb2,cb3;
	JComboBox<String> c;
	JList<String> li; 
	JButton b1;
	String id;
	
	Book7(String id)
	{
		super(id+"��");
		
		this.id=id;
		
		c=new JComboBox<String>();
		c.addItem("����");
		c.addItem("���");
		c.addItem("��ȭ");
		c.addItem("��ȭ");
		
		String[] data = {"               ","    ","    "};
		li=new JList<String>(data);
		
		la1=new JLabel("�뿩�ϱ�                ");
		la2=new JLabel("å�帣");
		la3=new JLabel("å����");
		la4=new JLabel("�뿩�Ⱓ                  "); 
		
		cb1= new JRadioButton("3��", true);
		cb2= new JRadioButton("5��");
		cb3= new JRadioButton("7��");
		
		ButtonGroup p =new ButtonGroup();
		p.add(cb1); p.add(cb2);p.add(cb3);
		
		b1=new JButton("Ȯ��");
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la1);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(la2); p2.add(c);
		
		JPanel p3=new JPanel(); p3.setLayout(new FlowLayout());
		p3.add(la3); p3.add(li);
		
		JPanel p4=new JPanel(); p4.setLayout(new FlowLayout());
		p4.add(la4); 
		
		JPanel p5=new JPanel(); p5.setLayout(new FlowLayout());
		p5.add(cb1); p5.add(cb2); p5.add(cb3); 
		
		JPanel p6=new JPanel(); p6.setLayout(new FlowLayout());
		p6.add(b1);
		
		JPanel p7=new JPanel(); p7.setLayout(new BorderLayout());
		p7.add("North",p1); p7.add("Center",p2); p7.add("South",p3);
		
		JPanel p8=new JPanel(); p8.setLayout(new BorderLayout());
		p8.add("North",p4); p8.add("Center",p5); p8.add("South",p6);
		
		this.setLayout(new BorderLayout());
		this.add("North",p7);
		this.add("Center", p8);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300);
		this.setVisible(true); //������ �������
	}
	public static void main(String[] args) 
	{
		new Book7("test");
	}//main

}//Book5