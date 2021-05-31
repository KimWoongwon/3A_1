import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class Ex1 extends JFrame implements ActionListener
{
	JLabel l1,l2,l3;
	JButton b;
	JTextField tf;
	JPasswordField pwf;
	
	Ex1()
	{
		super("login");
		
		l1 = new JLabel("BBQ");
		l1.setHorizontalAlignment(JLabel.CENTER);
		l2 = new JLabel("아이디");
		l3 = new JLabel("비번");
				
		b = new JButton("확인");
		b.addActionListener(this);
		
		tf = new JTextField(10);
		pwf = new JPasswordField(10);
		
		JPanel p1 = new JPanel();
		p1.setLayout(new FlowLayout());
		p1.add(l2); p1.add(tf);
		
		JPanel p2 = new JPanel();
		p2.setLayout(new FlowLayout());
		p2.add(l3); p2.add(pwf);
		
		JPanel p3 = new JPanel();
		p3.setLayout(new BorderLayout());
		p3.add("North", p1); p3.add("Center", p2);
		
		JPanel p4 = new JPanel();
		p4.setLayout(new FlowLayout());
		p4.add(b);
		
		this.setLayout(new BorderLayout());
		this.add("North",l1);
		this.add("Center", p3);
		this.add("South", p4);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setSize(300,200); 
		this.setVisible(true); //무조건 보여줘라
		
	}
	
	
	@Override
	public void actionPerformed(ActionEvent e)
	{
		String _id = tf.getText();
		char[] _pw = pwf.getPassword();
		String str_pw = new String(_pw);
		
		if(str_pw.equals("4567"))
		{
			new Ex2(_id);
			this.setVisible(false);
		}
		
	}
	
	public static void main(String[] args)
	{
		// TODO Auto-generated method stub
		new Ex1();

	}

}
