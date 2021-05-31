import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class Ex2 extends JFrame implements ActionListener
{
	String ID, menu, side, choice;
	int totalprice = 0;
	
	JLabel l1,l2,l3;
	JComboBox<String> cb1, cb2;
	JRadioButton r1, r2;
	JButton b;
		
	Ex2(String id)
	{
		super(id + " 주문하기");
		this.ID = id;
		
		l1 = new JLabel("메뉴");
		l2 = new JLabel("사이드 메뉴");
		l3 = new JLabel("파무침");
				
		b = new JButton("확인");
		b.addActionListener(this);
		
		String[] menu = {"후라이드", "양념", "강정"};
		cb1 = new JComboBox<String>(menu);
		cb1.setSelectedItem("후라이드");
		
		String[] side = {"콜라", "사이다", "무"};
		cb2 = new JComboBox<String>(side);
		cb2.setSelectedItem("콜라");
				
		r1 = new JRadioButton("선택", true);
		r2 = new JRadioButton("미선택");
		
		ButtonGroup g = new ButtonGroup();
		g.add(r1); g.add(r2);
		
		
		JPanel p1 = new JPanel();
		p1.setLayout(new FlowLayout());
		p1.add(l1); p1.add(cb1);
		
		JPanel p2 = new JPanel();
		p2.setLayout(new FlowLayout());
		p2.add(l2); p2.add(cb2);
		
		JPanel p3 = new JPanel();
		p3.setLayout(new FlowLayout());
		p3.add(l3); p3.add(r1); p3.add(r2);
		
		JPanel p4 = new JPanel();
		p4.setLayout(new BorderLayout());
		p4.add("North", p1); p4.add("Center", p2); p4.add("South", p3);
		
		JPanel p5 = new JPanel();
		p5.setLayout(new FlowLayout());
		p5.add(b);
		
		this.setLayout(new BorderLayout());
		this.add("Center", p4);
		this.add("South", p5);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(300,200); 
		this.setVisible(true); //무조건 보여줘라
		
	}
	
	@Override
	public void actionPerformed(ActionEvent e)
	{
		menu = (String)cb1.getSelectedItem();
		side = (String)cb2.getSelectedItem();
		
		if(r1.isSelected())
		{
			choice = "선택";
			totalprice += 1000;
		}
		else
		{
			choice = "미선택";
		}
				
		String temp = new Ex_db().getPrice(menu);
		totalprice += Integer.parseInt(temp);
		
		temp = new Ex_db().getPrice(side);
		totalprice += Integer.parseInt(temp);
		
		new Ex3(ID, menu, side, choice, totalprice);
		this.setVisible(false);
	}
	
	public static void main(String[] args)
	{
		// TODO Auto-generated method stub
		new Ex2("test");

	}

}
