import java.sql.*;
import java.util.*;

public class Db 
{

	String dbDriver ="com.mysql.cj.jdbc.Driver";  
	String dbUrl    = "jdbc:mysql://localhost:3306/test?characterEncoding=UTF-8 & serverTimezone=UTC";
	String dbId     = "root";
	String dbPw     = "1234";
	
	Connection con = null;
	Statement stmt = null;  

	Db()
	{
		try
		{
			Class.forName( dbDriver );
			con = DriverManager.getConnection( dbUrl, dbId, dbPw );  
			stmt = con.createStatement();  
			System.out.println("mysql jdbc test: connect ok!!");
		}
		catch(Exception e) 
		{
			System.out.println(e);
		} 
	}
	
	Vector<String> all()
	{
		Vector<String> v = new Vector<String>();
		
		try
		{
			String sql = "select * from db_book";			
			ResultSet rs = stmt.executeQuery(sql);
			
			while(rs.next())
			{
				String s = "";
				String s0 = rs.getString("idx");
				String s1 = rs.getString("jang");
				String s2 = rs.getString("title");
				String s3 = rs.getString("price");
				String s4 = rs.getString("com");
				 
				s = s0+"@"+s1+"@"+s2+"@"+s3+"@"+s4;
				v.add(s);
			}
			
			stmt.close();
			con.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return v;	
	}
	
	Vector<String> all_user()
	{
		Vector<String> v = new Vector<String>();
		
		try
		{
			String sql = "select * from db_user";			
			ResultSet rs = stmt.executeQuery(sql);
			
			while(rs.next())
			{
				String s = "";
				String s0 = rs.getString("idx");
				String s1 = rs.getString("name");
				String s2 = rs.getString("bkname");
				String s3 = rs.getString("borrowdate");
				String s4 = rs.getString("price");
				 
				s = s0+"@"+s1+"@"+s2+"@"+s3+"@"+s4;
				v.add(s);
			}
			
			stmt.close();
			con.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return v;
	}
	
	Vector<String> find(String m)
	{
		Vector<String> v = new Vector<String>();
		
		try
		{
			String sql = "select title from db_book where jang = '" + m + "'";			
			ResultSet rs = stmt.executeQuery(sql);
			
			while(rs.next())
			{
				String s = rs.getString("title");
				v.add(s);
			}
			
			stmt.close();
			con.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return v;	
	}
	
	void In(String j, String t, String p, String c)
	{
		try
		{
			String sql =  "INSERT INTO `test`.`db_book` (`jang`, `title`, `price`, `com`)"
						+ " VALUES ('"+j+"', '"+t+"', '"+p+"', '"+c+"')";
			stmt.executeUpdate(sql);
			System.out.println("Insert ok");
			
			Ordering_Index();
			
			stmt.close();
			con.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	void In_user(String n, String bn, String d, String p)
	{
		try
		{
			String sql =  "INSERT INTO `test`.`db_user` (`name`, `bkname`, `borrowdate`, `price`)"
						+ " VALUES ('"+ n +"', '"+ bn +"', '"+ d +"', '"+ p +"')";
			stmt.executeUpdate(sql);
			System.out.println("UserInfo Insert ok");
			
			Ordering_Index();
			
			stmt.close();
			con.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	void del(String idx)
	{
		try
		{
			String sql = "delete from db_book where idx = '" + idx + "'";
			stmt.executeUpdate(sql);
			System.out.println("del ok");
			
			Ordering_Index();
			
			stmt.close();
			con.close();
		} 
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
	
	void up(String index, String jang, String title, String price, String com)
	{
		try
		{
			String sql = "update db_book set jang = '" + jang + "' where idx = " + index;
			stmt.executeUpdate(sql);
			sql = "update db_book set title = '" + title + "' where idx = " + index;
			stmt.executeUpdate(sql);
			sql = "update db_book set price = '" + price + "' where idx = " + index;
			stmt.executeUpdate(sql);
			sql = "update db_book set com = '" + com + "' where idx = " + index;
			stmt.executeUpdate(sql);
			
			System.out.println("up ok");
			
			Ordering_Index();
			
			stmt.close();
			con.close();
		} 
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
	String get(String t)
	{
		String price = "";

		try
		{
			String sql = "select price from db_book where title='" + t + "'";
			ResultSet rs = stmt.executeQuery(sql);

			while (rs.next())
			{
				price = rs.getString("price");
			} // while

			System.out.println("get ok");

			stmt.close();
			con.close();

		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		return price;
	}// get
	
	void Ordering_Index()
	{
		try
		{
			String sql = "set @CNT = 0";
			stmt.executeUpdate(sql);
			sql = "update db_book set db_book.idx = @CNT:=@CNT+1";
			stmt.executeUpdate(sql);
			System.out.println("up ok");
			
			stmt.close();
			con.close();
		} 
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void main(String args[]) 
	{
		new Db();
	}

}

