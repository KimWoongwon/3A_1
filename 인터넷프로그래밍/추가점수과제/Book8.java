import java.awt.*;

import javax.swing.*;

import java.awt.event.*;

public class Book8 extends JFrame implements ActionListener
{
	JLabel la; JButton b1,b2,b3;
	String id;
	
	Book8(String id)
	{
		super(id+"��");
		this.id = id;
		la=new JLabel(id+ "�����մϴ�");
		b1=new JButton("����");
		b1.addActionListener(new ActionListener()
		{
			
			@Override
			public void actionPerformed(ActionEvent e)
			{
				System.exit(0); 
			}
		});
		
		b2=new JButton("ó������");
		b2.addActionListener(this);
		
		b3 = new JButton("�߰� �뿩");
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
		this.setVisible(true); //������ �������
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