import java.sql.*;
import java.util.*;

public class Ex_db 
{

	String dbDriver ="com.mysql.cj.jdbc.Driver";  
	String dbUrl    = "jdbc:mysql://localhost:3306/test?characterEncoding=UTF-8 & serverTimezone=UTC";
	String dbId     = "root";
	String dbPw     = "Rladnddnjs97!";
	
	Connection con = null;
	Statement stmt = null;  

	Ex_db()
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
		
	String getPrice(String title)
	{
		String price = "";
		try
		{
			String sql = "select * from bbq where menu = '" + title + "'";
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
	
	public static void main(String args[]) 
	{
		new Ex_db();
	}

}

