import java.awt.*;

import javax.swing.*;

import java.awt.event.*;


public class Book6 extends JFrame 
{
	JLabel la1,la2,la3;
	JButton b1,b2;
	String id,t,b,d;
	
	Book6(String id, String t, String b, String d)
	{
		super(id+"��");
		
		this.id=id; 
		this.t=t; 
		this.b=b; 
		this.d=d;
		
		la1=new JLabel("å�̸� :"+b);
		la2=new JLabel("�뿩�Ⱓ:"+d);
		la3=new JLabel("�ݾ� "+t);
		
		b1=new JButton("Ȯ��");
		b2=new JButton("�����ϱ�");
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la1);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(la2); 
		
		JPanel p3=new JPanel(); p3.setLayout(new FlowLayout());
		p3.add(la3); 
		
		JPanel p4=new JPanel(); p4.setLayout(new FlowLayout());
		p4.add(b1);	p4.add(b2);
		
		JPanel p5=new JPanel(); p5.setLayout(new BorderLayout());
		p5.add("North",p1); p5.add("Center",p2); p5.add("South",p3);
		
		this.setLayout(new BorderLayout());
		this.add("North",p5);
		this.add("Center", p4);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300); this.setVisible(true); //������ �������
		
	}
	
	public static void main(String[] args) 
	{
		//new Book6();
	}//main

}//Book6