import java.awt.*;

import javax.swing.*;

import java.awt.event.*;

public class Book8 extends JFrame
{
	JLabel la; JButton b1,b2;
	
	Book8(String id)
	{
		super(id+"��");
		
		la=new JLabel(id+ "�����մϴ�");
		b1=new JButton("����");
		b2=new JButton("ó������");
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(b1);	p2.add(b2);
		
		this.setLayout(new BorderLayout());
		this.add("North",p1);	
		this.add("Center", p2);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300);
		this.setVisible(true); //������ �������
	}
	
	public static void main(String[] args) 
	{
		new Book8("");
	}//main

}//class