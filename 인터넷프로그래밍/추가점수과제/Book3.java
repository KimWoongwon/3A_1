import java.awt.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.table.DefaultTableModel;

class Book3 extends JFrame
{
	static Book3 Instance;
	JTable li;
	JButton b1, b2, b3;
	DefaultTableModel model;
	
	Vector<String> booklist = new Vector<String>();

	Book3()
	{
		super("책리스트");
		
		b1 = new JButton("수정");
		b1.addActionListener(new Change_Btn(this));
		b2 = new JButton("삭제");
		b2.addActionListener(new del(this));
		b3 = new JButton("추가");
		b3.addActionListener(new Add_Btn(this));
		
		booklist = new Db().all();
		
		Object[][] data = new Object[booklist.size()][5];
		
		for (int i = 0; i < booklist.size(); i++)
		{
			String t[] = booklist.get(i).split("@");
			System.out.println(booklist.get(i));
			data[i][0] = t[0];
			data[i][1] = t[1];
			data[i][2] = t[2];
			data[i][3] = t[3];
			data[i][4] = t[4];
		}
				
		/*
		 * Object[][] data = { { book, jang, com, price } };
		 */
		String colNames[] =
		{ "번호","장르", "제목", "출판사", "가격" };
		model = new DefaultTableModel(data, colNames);
		li = new JTable(model);
		JScrollPane jp = new JScrollPane(li);

		JPanel p3 = new JPanel();
		p3.setLayout(new FlowLayout());
		p3.add(b1);
		p3.add(b2);
		p3.add(b3);

		this.setLayout(new BorderLayout());
		this.add("Center", jp);
		this.add("South", p3);

		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.pack();
		this.setVisible(true);

	}
	class Change_Btn implements ActionListener
	{
		
		Change_Btn(Book3 book3)
		{
			if(Instance != null)
				Instance = book3;
		}
		@Override
		public void actionPerformed(ActionEvent e)
		{
			int i = li.getSelectedRow();
			if(li.getSelectedRow() == -1)
				return;
			
			String index = (String)li.getValueAt(i,0);
			String jang = (String)li.getValueAt(i,1);
			String title = (String)li.getValueAt(i,2);
			String price = (String)li.getValueAt(i,3);
			String com = (String)li.getValueAt(i,4);
			
			new Book4(index, jang, title, price, com);
			
			Instance.setVisible(false);
		}
	}
	
	class Add_Btn implements ActionListener
	{
		
		Add_Btn(Book3 book3)
		{
			if(Instance != null)
				Instance = book3;
		}
		@Override
		public void actionPerformed(ActionEvent e)
		{
			new Book2();
			Instance.setVisible(false);
		}
	}

	class del implements ActionListener
	{
		del(Book3 book3)
		{
			if(Instance != null)
				Instance = book3;
		}
		public void actionPerformed(ActionEvent e)
		{
			int i = li.getSelectedRow();
			if(li.getSelectedRow() == -1)
				return;
			
			String index = (String)li.getValueAt(i, 0);
			
			new Db().del(index);
			
			if (i != -1)
				model.removeRow(i);
		}
	}// b2.addActionListener(new del());

	public static void main(String[] args)
	{
		new Book3();
		

	}

}
