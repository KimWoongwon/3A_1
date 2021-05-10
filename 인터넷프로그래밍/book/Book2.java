import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

public class Book2 extends JFrame
{
	JLabel la1,la2,la3,la4;
	JTextField tf1,tf2,tf3;
	JButton b1,b2;
	JComboBox<String> c;
	
	Book2()
	{
		super("�߰��ϱ�");
		
		c=new JComboBox<String>();
		c.addItem("���"); c.addItem("��ȭ"); c.addItem("��ȭ");
		
		la1=new JLabel("å�帣");
		la2=new JLabel("å�̸�");
		la3=new JLabel("�ݾ�");
		la4=new JLabel("���ǻ�"); 
		
		tf1=new JTextField(7);
		tf2=new JTextField(7);
		tf3=new JTextField(7);
		
		b1=new JButton("�߰�");
		b1.addActionListener(new In());
		
		b2=new JButton("����"); 
		b2.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				tf1.setText("");
				tf2.setText("");
				tf3.setText("");
				c.setSelectedIndex(0);
			}
		});
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la1); p1.add(c);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(la2); p2.add(tf1);
		
		JPanel p3=new JPanel(); p3.setLayout(new FlowLayout());
		p3.add(la3); p3.add(tf2);
		
		JPanel p4=new JPanel(); p4.setLayout(new FlowLayout());
		p4.add(la4); p4.add(tf3);
		
		JPanel p5=new JPanel(); p5.setLayout(new FlowLayout());
		p5.add(b1); p5.add(b2);
		
		JPanel p6=new JPanel(); p6.setLayout(new BorderLayout());
		p6.add("North",p1); p6.add("Center",p2); p6.add("South",p3);
		
		this.setLayout(new BorderLayout());
		this.add("North",p6);
		this.add("Center", p4);
		this.add("South",p5);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300); this.setVisible(true); //������ �������
	}
	
	class In implements ActionListener
	{
		@Override
		public void actionPerformed(ActionEvent e)
		{
			String jang = (String)c.getSelectedItem();
			String title = tf1.getText();	//title
			String price = tf2.getText();	//price
			String company = tf3.getText();	//company
			
			new Db().In(jang, title, price, company);
			
			tf1.setText("");
			tf2.setText("");
			tf3.setText("");
			c.setSelectedIndex(0);
		}
	}
	
	public static void main(String[] args) 
	{
		new Book2();
	}

}