import java.awt.*;
import javax.swing.*;

public class Swt4 extends JFrame
{
	JLabel l1, l2, l3, l4;
	JButton b1, b2;
	JCheckBox cb1, cb2, cb3;
	JTextArea ta;
	JTextField tf;
	JRadioButton r1, r2;

	Swt4()
	{
		l1 = new JLabel("�̸�"); 
		l2 = new JLabel("����");
		l3 = new JLabel("���");
		l4 = new JLabel("�ڱ�Ұ�"); 
		
		tf = new JTextField(20);
		
		r1 = new JRadioButton("��", true); r2 = new JRadioButton("��");
		ButtonGroup p = new ButtonGroup(); p.add(r1); p.add(r2);
		
		cb1 = new JCheckBox("����");
		cb2 = new JCheckBox("����");
		cb3 = new JCheckBox("�");
		
		ta = new JTextArea("100�� ����",10,20);
		ta.setBackground(Color.white);
		
		b1 = new JButton("����");
		b2 = new JButton("�Ϸ�");
		
		JPanel p1 = new JPanel();
		p1.setLayout(new FlowLayout());
		p1.add(l1); p1.add(tf);
		
		JPanel p2 = new JPanel();
		p2.setLayout(new FlowLayout());
		p2.add(l2); p2.add(r1); p2.add(r2);
				
		JPanel p3 = new JPanel();
		p3.setLayout(new FlowLayout());
		p3.add(l3); p3.add(cb1); p3.add(cb2); p3.add(cb3);
		
		JPanel Bp = new JPanel();
		Bp.setLayout(new BorderLayout());
		Bp.add("North", p1);
		Bp.add("Center", p2);
		Bp.add("South", p3);
		
		JPanel p4 = new JPanel();
		p4.setLayout(new FlowLayout());
		p4.add(l4); p4.add(ta);
		
		JPanel p5 = new JPanel();
		p5.setLayout(new FlowLayout());
		p5.add(b1); p5.add(b2);
		
		this.setLayout(new BorderLayout());
		this.add("North", Bp);
		this.add("Center", p4);
		this.add("South", p5);
		this.setTitle("�̷¼�");
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.pack();
		this.setVisible(true);
	}
	
	public static void main(String[] args)
	{
		new Swt4();

	}

}
