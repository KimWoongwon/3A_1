import java.awt.*;

import javax.swing.*;

import java.awt.event.*;


public class Book6 extends JFrame implements ActionListener
{
	JLabel la1,la2,la3;
	JButton b1,b2;
	String id,j,t,d;
	
	Book6(String id, String j, String t, String d)
	{
		super(id+"��");
		
		this.id=id; this.j=j; this.t=t; this.d=d;
		
		String price = new Db().getPrice(t);
		int total = Integer.parseInt(price) * Integer.parseInt(d);
		
		la1=new JLabel("å�̸� 	: " + t);
		la2=new JLabel("�뿩�Ⱓ	: " + d);
		la3=new JLabel("�ݾ�		: " + total);
		
		b1=new JButton("Ȯ��");
		b1.addActionListener(this);
		b2=new JButton("�����ϱ�");
		b2.addActionListener(this);
		
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
	
	@Override
	public void actionPerformed(ActionEvent e)
	{
		JButton bb = (JButton)e.getSource();
		if(bb.equals(b1))
		{
			new Book8(id);
			this.setVisible(false);
		}
		else
		{
			new Book7(id, j, t, d);
			this.setVisible(false);
		}
		
	}
	
	public static void main(String[] args) 
	{
		//new Book6();
	}//main

}//Book6