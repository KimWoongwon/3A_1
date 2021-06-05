import java.awt.*;
import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import java.awt.event.*;
import java.util.*;



class Book1 extends JFrame implements ActionListener
{
	static Book1 Instance;
	JTable li;
	JLabel la;
	JButton b1;
	DefaultTableModel model;
	
	Vector<String> Userlist = new Vector<String>();
	int total = 0;

	Book1()
	{
		super("�뿩��Ȳ");
		Instance = null;
		
		b1 = new JButton("å�߰�");
		b1.addActionListener(this);
		
		Userlist = new Db().all_user();

		Object[][] data = new Object[Userlist.size()][5];
		
		for (int i = 0; i < Userlist.size(); i++)
		{
			String t[] = Userlist.get(i).split("@");
			System.out.println(Userlist.get(i));
			data[i][0] = t[0];
			data[i][1] = t[1];
			data[i][2] = t[2];
			data[i][3] = t[3];
			data[i][4] = t[4];
			total += Integer.parseInt(t[4]);
		}
		
		la = new JLabel("�ѱݾ� : " + total + "��");
		
		String colNames[] =
		{ "��ȣ", "�̸�", "����", "�뿩�Ⱓ", "�ݾ�" };
		model = new DefaultTableModel(data, colNames);
		li = new JTable(model);
		JScrollPane jp = new JScrollPane(li);

		JPanel p2 = new JPanel();
		p2.setLayout(new FlowLayout(1));
		p2.add(la);

		JPanel p3 = new JPanel();
		p3.setLayout(new FlowLayout());
		p3.add(b1);

		this.setLayout(new BorderLayout());
		this.add("North", jp);
		this.add("Center", p2);
		this.add("South", p3);

		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.pack();
		this.setVisible(true);

	}

	public static void main(String[] args)
	{
		new Book1();

	}

	@Override
	public void actionPerformed(ActionEvent e)
	{
		new Book2();
		this.setVisible(false);
	}

}
