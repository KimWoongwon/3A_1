import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class Book0 extends JFrame
{
	static Book0 Instance = null;
	JRadioButton cb1,cb2;
	JTextField tf1;
	JPasswordField tf2;
	JButton b1,b2;
	JLabel a1,a2;
	
	
	Book0()
	{
		super("들어가기");
		Instance = null;
		b1=new JButton("들어가기");
		b1.addActionListener(new Enter(this));
		
		b2=new JButton("수정");
		b2.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				tf2.setEnabled(false);
				tf1.setText("");
				tf2.setText("");
				cb1.setSelected(true);
			}
		});
		
		cb1=new JRadioButton("회원",true);
		cb1.addActionListener(new ActionListener()
		{
			
			@Override
			public void actionPerformed(ActionEvent e)
			{
				tf2.setEnabled(false);
				tf1.setText("");
				tf2.setText("");
			}
		});
		
		cb2=new JRadioButton("관리자");
		cb2.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				tf2.setEnabled(true);
			}
		});
		
		ButtonGroup p =new ButtonGroup();
		p.add(cb1); p.add(cb2);
		
		a1=new JLabel("ID "); a2=new JLabel("PW");
		
		tf1=new JTextField(7);
		tf2=new JPasswordField(7);
		tf2.setEnabled(false);
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(cb1); p1.add(cb2);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(a1); p2.add(tf1);
		
		JPanel p3=new JPanel(); p3.setLayout(new FlowLayout());
		p3.add(a2); p3.add(tf2);
		
		JPanel p4=new JPanel(); p4.setLayout(new FlowLayout());
		p4.add(b1); p4.add(b2);
		
		JPanel p5=new JPanel(); p5.setLayout(new BorderLayout());
		p5.add("North",p1); p5.add("Center",p2); p5.add("South",p3);
		
		this.setLayout(new BorderLayout());
		this.add("North",p5);
		this.add("Center", p4);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300); this.setVisible(true); //무조건 보여줘라
	}
	
	public static void main(String[] args) 
	{
		new Book0();
	}//main

	class Enter implements ActionListener
	{
		
		Enter(Book0 book)
		{
			if(Instance == null)
				Instance = book;
		}
		@Override
		public void actionPerformed(ActionEvent e)
		{
			if(cb1.isSelected())
			{
				// 회원 모드
				String id = tf1.getText();
				System.out.println(id);
				new Book5(id);
				Instance.setVisible(false);
			}
			else
			{
				// 관리자 모드
				String id = tf1.getText();
				char pw[] = tf2.getPassword();
				String pw2 = new String(pw);
				
				if(id.equals("admin") && pw2.equals("1234"))
				{
					new Book1();
					Instance.setVisible(false);
				}
				else
				{
					tf1.setText("id pw check");
				}
				
			}
		}
	}
	



}//Book0

