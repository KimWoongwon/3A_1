import java.awt.*;

import javax.swing.*;

import java.awt.event.*;

public class Book6 extends JFrame implements ActionListener
{

	JLabel la1, la2, la3;
	JButton b1, b2;
	String id, j, t, d;
	
	int total;

	Book6(String id, String j, String t, String d)
	{

		super(id + "님");

		this.id = id;
		this.j = j;
		this.t = t;
		this.d = d;

		String p=new Db().get(t);
		int ip = Integer.parseInt(p);// 금액
		int dd = Integer.parseInt(d);// day
		total = ip * dd;

		la1 = new JLabel("책이름 :" + t);
		la2 = new JLabel("대여기간:" + d);
		la3 = new JLabel("금액 " + total);

		b1 = new JButton("확인");
		b1.addActionListener(this);

		b2 = new JButton("수정하기");
		b2.addActionListener(this);

		JPanel p1 = new JPanel();	p1.setLayout(new FlowLayout());
		p1.add(la1);

		JPanel p2 = new JPanel();	p2.setLayout(new FlowLayout());
		p2.add(la2);

		JPanel p3 = new JPanel();	p3.setLayout(new FlowLayout());
		p3.add(la3);

		JPanel p4 = new JPanel();	p4.setLayout(new FlowLayout());
		p4.add(b1);	p4.add(b2);

		JPanel p5 = new JPanel();	p5.setLayout(new BorderLayout());
		p5.add("North", p1); p5.add("Center", p2); p5.add("South", p3);

		this.setLayout(new BorderLayout());
		this.add("North", p5);
		this.add("Center", p4);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(400, 300);
		this.setVisible(true); // 무조건 보여줘라

	}

	public static void main(String[] args)
	{
		// new Book6();
	}// main

	@Override
	public void actionPerformed(ActionEvent e)
	{
		JButton bb = (JButton) e.getSource();
		if (bb.equals(b1))
		{
			new Book8(id);
			new Db().In_user(id, t, d, total+"");
			this.setVisible(false);

		} 
		else
		{
			new Book7(id,j,t,d);
			this.setVisible(false);
		}

	}// actionPerformed

}// Book6