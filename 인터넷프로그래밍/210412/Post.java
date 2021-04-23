import java.awt.*;
import javax.swing.*;


public class Post extends JFrame
{
	JLabel la;
	JButton b1, b2, b3, b4;
	JTextField tf1, tf2;
	JList<String> st;
	
	
	Post()
	{
		super("게임프로그래밍3A_16032020_김웅원");
		la = new JLabel("검색");
		
		b1 = new JButton("Find");
		b2 = new JButton("Del");
		b3 = new JButton("Show");
		b4 = new JButton("Update");
		
		String s[] = {	"                    ", "                    ", "                    ",
						"                    ", "                    ", "                    ",
						"                    ", "                    ", "                    ",
						"                    ", "                    ", "                    ",
						"                    ", "                    ", "                    "}; 
		st = new JList<String>(s);
		
		tf1 = new JTextField(5);
		tf2 = new JTextField(20);
		tf2.setBackground(Color.pink);
		
		JPanel p1 = new JPanel();
		p1.setLayout(new FlowLayout());
		p1.add(la); p1.add(tf1); p1.add(b1); p1.add(b2); p1.add(b3); p1.add(b4);
		
		this.setLayout(new BorderLayout());
		this.add("North", p1);
		this.add("Center", st);
		this.add("South", tf2);
				
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setSize(700, 400);
		this.setVisible(true);
		
	}
	
	public static void main(String[] args)
	{
		new Post();

	}

}
