import java.awt.*;

import javax.swing.*;

import java.awt.event.*;

public class Book8 extends JFrame implements ActionListener
{
	JLabel la; JButton b1,b2;
	
	Book8(String id)
	{
		super(id+"님");
		
		la=new JLabel(id+ "감사합니다");
		b1=new JButton("종료");
		b1.addActionListener(this);
		b2=new JButton("처음으로");
		b2.addActionListener(this);
		
		JPanel p1=new JPanel(); p1.setLayout(new FlowLayout());
		p1.add(la);
		
		JPanel p2=new JPanel(); p2.setLayout(new FlowLayout());
		p2.add(b1);	p2.add(b2);
		
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
		JButton bb = (JButton)e.getSource();
		if(bb.equals(b1))
			System.exit(0);
		else
		{
			new Book0();
			this.setVisible(false);
		}
			
		
	}
	
	public static void main(String[] args) 
	{
		new Book8("");
	}//main

}//class