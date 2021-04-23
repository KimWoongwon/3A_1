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
		l1 = new JLabel("이름"); 
		l2 = new JLabel("성별");
		l3 = new JLabel("취미");
		l4 = new JLabel("자기소개"); 
		
		tf = new JTextField(20);
		
		r1 = new JRadioButton("남", true); r2 = new JRadioButton("여");
		ButtonGroup p = new ButtonGroup(); p.add(r1); p.add(r2);
		
		cb1 = new JCheckBox("게임");
		cb2 = new JCheckBox("독서");
		cb3 = new JCheckBox("운동");
		
		ta = new JTextArea("100자 이하",10,20);
		ta.setBackground(Color.white);
		
		b1 = new JButton("수정");
		b2 = new JButton("완료");
		
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
		this.setTitle("이력서");
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.pack();
		this.setVisible(true);
	}
	
	public static void main(String[] args)
	{
		new Swt4();

	}

}
