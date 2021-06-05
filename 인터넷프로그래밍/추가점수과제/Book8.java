import java.awt.*;

import javax.swing.*;

import java.awt.event.*;

public class Book8 extends JFrame implements ActionListener
{
	JLabel la; JButton b1,b2,b3;
	String id;
	
	Book8(String id)
	{
		super(id+"님");
		this.id = id;
		la=new JLabel(id+ "감사합니다");
		b1=new JButton("종료");
		b1.addActionListener(new ActionListener()
		{
			
			@Override
			public void actionPerformed(ActionEvent e)
			{
				System.exit(0); 
			}
		});
		
		b2=new JButton("처음으로");
		b2.addActionListener(this);
		
		b3 = new JButton("추가 대여");
		b3.addActionListener(this);
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(b1);	p2.add(b2); p2.add(b3);
		
		this.setLayout(new BorderLayout());
		this.add("North",p1);	
		this.add("Center", p2);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400,300);
		this.setVisible(true); //무조건 보여줘라
	}
	
	@Override
	public void actionPerformed(ActionEvent e)
	{
		JButton bb = (JButton) e.getSource();
		if (bb.equals(b2))
		{
			new Book0();
			this.setVisible(false);
		} 
		else
		{
			new Book5(id);
			this.setVisible(false);
		}
	}
	
	public static void main(String[] args) 
	{
		new Book8("");
	}//main

}//class