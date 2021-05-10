import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.util.*;

public class Book5 extends JFrame  
{
	JLabel la1,la2,la3,la4; 
	JRadioButton cb1,cb2,cb3;
	JComboBox<String> c;
	JList<String> li; 
	JButton b1;
	String id;
	
	Vector<String> v = new Vector<String>();
	
	Book5(String id)
	{
		super(id+"님");
		
		this.id=id;
		
		c=new JComboBox<String>();
		c.addItem("선택");
		c.addItem("드라마");
		c.addItem("영화");
		c.addItem("만화");
		
		c.addItemListener(new Ck());
		 
		String[] data = {"               ","    ","    "};
		li=new JList<String>(data);
		
		la1=new JLabel("대여하기");
		la2=new JLabel("책장르");
		la3=new JLabel("책선택");
		la4=new JLabel("대여기간"); 
		
		cb1= new JRadioButton("3일", true);
		cb2= new JRadioButton("5일");
		cb3= new JRadioButton("7일");
		
		ButtonGroup p =new ButtonGroup();
		p.add(cb1); p.add(cb2);p.add(cb3);
		
		b1=new JButton("확인");
		
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
		this.setVisible(true); //무조건 보여줘라
		
	}
	
	class Ck implements ItemListener
	{
		@Override
		public void itemStateChanged(ItemEvent e)
		{
			String m = (String)c.getSelectedItem();
			if(!m.equals("선택"))
			{
				v.clear();
				v = new Db().find(m);
				li.setListData(v);
			}
			else
			{
				v.clear();
				String[] data = {"               ","    ","    "};
				li.setListData(data);
			}
		}
	}
	
	public static void main(String[] args) 
	{
		new Book5("test");
	}//main

}//Book5