import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

public class Book4 extends JFrame
{
	static Book4 Instance = null;
	JLabel la1,la2,la3,la4;
	JTextField tf1,tf2,tf3;
	JButton b1;
	JComboBox<String> c;
	
	String index;
	
	Book4(String index, String jang, String title, String price, String com)
	{
		super("추가하기");
		Instance = null;				
		c=new JComboBox<String>();
		c.addItem("선택");	c.addItem("드라마"); c.addItem("영화"); c.addItem("만화");
		
		c.setSelectedItem(jang);
		
		la1=new JLabel("책장르");
		la2=new JLabel("책이름");
		la3=new JLabel("금액");
		la4=new JLabel("출판사"); 
		
		tf1=new JTextField(15);
		tf1.setText(title);
		tf2=new JTextField(15);
		tf2.setText(price);
		tf3=new JTextField(15);
		tf3.setText(com);
		
		this.index = index;
		
		b1=new JButton("수정"); 
		b1.addActionListener(new Change_Btn(this));
				
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la1); p1.add(c);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(la2); p2.add(tf1);
		
		JPanel p3=new JPanel(); p3.setLayout(new FlowLayout());
		p3.add(la3); p3.add(tf2);
		
		JPanel p4=new JPanel(); p4.setLayout(new FlowLayout());
		p4.add(la4); p4.add(tf3);
		
		JPanel p5=new JPanel(); p5.setLayout(new FlowLayout());
		p5.add(b1);
		
		JPanel p6=new JPanel(); p6.setLayout(new BorderLayout());
		p6.add("North",p1); p6.add("Center",p2); p6.add("South",p3);
		
		this.setLayout(new BorderLayout());
		this.add("North",p6);
		this.add("Center", p4);
		this.add("South",p5);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300); this.setVisible(true); //무조건 보여줘라
	}
	
	class Change_Btn implements ActionListener
	{
		Change_Btn(Book4 book4)
		{
			if(Instance == null)
				Instance = book4;
		}
		@Override
		public void actionPerformed(ActionEvent e)
		{
			boolean flag = false;
			String jang = (String)c.getSelectedItem();
			String title = tf1.getText();	//title
			String price = tf2.getText();	//price
			String company = tf3.getText();	//company

			if(jang.equals("선택"))
			{
				JOptionPane.showMessageDialog(null, "장르를 선택하세요.");
				flag = true;
			}
			else if(title.equals(""))
			{
				JOptionPane.showMessageDialog(null, "제목을 입력하세요.");
				flag = true;
			}
			else if(price.equals(""))
			{
				JOptionPane.showMessageDialog(null, "가격을 입력하세요.");
				flag = true;
			}
			else if(company.equals(""))
			{
				JOptionPane.showMessageDialog(null, "출판사를 입력하세요.");
				flag = true;
			}
			
			if(!flag)
			{
				new Db().up(index, jang, title, price, company);
				
				tf1.setText("");
				tf2.setText("");
				tf3.setText("");
				c.setSelectedIndex(0);
				
				new Book3();
				
				Instance.setVisible(false);
			}
		}
	}
	
	public static void main(String[] args) 
	{
		//new Book4();
	}

}