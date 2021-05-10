import java.sql.*;
import java.util.*;

public class Db 
{

	String dbDriver ="com.mysql.cj.jdbc.Driver";  
	String dbUrl    = "jdbc:mysql://localhost:3306/test?characterEncoding=UTF-8 & serverTimezone=UTC";
	String dbId     = "root";
	String dbPw     = "Rladnddnjs97!";
	
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
	
	Vector all()
	{
		Vector<String> v = new Vector<String>();
		
		try
		{
			String sql = "select * from zipcode";			
			ResultSet rs = stmt.executeQuery(sql);
			
			while(rs.next())
			{
				String s = "";
				String s1 = rs.getString("ZIPCODE");
				String s2 = rs.getString("SIDO");
				String s3 = rs.getString("GUNGU");
				String s4 = rs.getString("DORO");
				 
				s = s1+" "+s2+" "+s3+" "+s4;
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
	
	Vector find(String m)
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
			
			stmt.close();
			con.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
	}
	
	String getPrice(String title)
	{
		String price = "";
		try
		{
			String sql = "select * from db_book where title = '" + title + "'";
			ResultSet rs = stmt.executeQuery(sql);
			while (rs.next())
			{
				price = rs.getString("price");
			}
			System.out.println("getPrice ok");
			stmt.close(); con.close();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		return price;
	}
	
	void del(String d)
	{
		try
		{
			String sql = "delete from zipcode where doro = '" + d + "'";
			stmt.executeUpdate(sql);
			System.out.println("del ok");
			
			stmt.close();
			con.close();
		} 
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
	void up(String s, String m)
	{
		try
		{
			String sql = "update zipcode set doro = '" + m + "' where doro = '" + s + "'";
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

