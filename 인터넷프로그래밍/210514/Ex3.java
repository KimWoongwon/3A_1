import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class Ex3 extends JFrame implements ActionListener
{
	String ID;
	
	JLabel l1,l2,l3,l4;
	JButton b1, b2;
		
	Ex3(String id, String menu, String side, String choice, int Price)
	{
		super(id + " 주문확인");
		this.ID = id;
		
		l1 = new JLabel("메뉴 선택 : "+menu);
		l2 = new JLabel("사이드 메뉴 : "+side);
		l3 = new JLabel("파무침 선택 : "+choice);
		l4 = new JLabel("총합 : "+Price);
		
		b1 = new JButton("확인");
		b1.addActionListener(this);
			
		b2 = new JButton("수정");
		b2.addActionListener(this);
				
		JPanel p1 = new JPanel();
		p1.setLayout(new FlowLayout());
		p1.add(l1);
		
		JPanel p2 = new JPanel();
		p2.setLayout(new FlowLayout());
		p2.add(l2);
		
		JPanel p3 = new JPanel();
		p3.setLayout(new FlowLayout());
		p3.add(l3);
		
		JPanel p4 = new JPanel();
		p4.setLayout(new BorderLayout());
		p4.add("North",p1); p4.add("Center",p2); p4.add("South",p3);
		
		JPanel p5 = new JPanel();
		p5.setLayout(new FlowLayout());
		p5.add(l4);
		
		JPanel p6 = new JPanel();
		p6.setLayout(new FlowLayout());
		p6.add(b1); p6.add(b2);
		
		this.setLayout(new BorderLayout());
		this.add("North", p4);
		this.add("Center", p5);
		this.add("South", p6);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(300,200); 
		this.setVisible(true); //무조건 보여줘라
		
	}
	
	@Override
	public void actionPerformed(ActionEvent e)
	{
		JButton temp = (JButton)e.getSource();
		
		if(temp.equals(b1))
			System.exit(0);
		else if(temp.equals(b2))
		{
			new Ex1();
			this.setVisible(false);
		}		
	}

	public static void main(String[] args)
	{
		// TODO Auto-generated method stub
		new Ex3("test", "후라이드", "콜라", "선택", 20500);

	}

}
