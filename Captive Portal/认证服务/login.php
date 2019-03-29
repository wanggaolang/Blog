<?php
	$username = $_POST["username"];
	$password = $_POST["password"];
	#mysql中的wifi数据库admin表
	$dsn = 'mysql:host=127.0.0.1;dbname=wifi;port=3306;charset=utf8';
	$user = 'root';
	$pass = 'mysql';
	$pdo = new PDO($dsn,$user,$pass);	
	$sql = "select * from admin where username = '$username'";
	$pdo_statement = $pdo->query($sql);
	$row = $pdo_statement->fetch();
	if($row['password'] == $password && $username!='' && $password!=''){
		//设置session
		session_start();
        $_SESSION['username'] = $username;
        $_SESSION['ip'] = $_SERVER["REMOTE_ADDR"];
        $_SESSION['check'] = true;
        exec("ipset add goodip $_SESSION['ip']");
        header("Refresh:1;url=welcome.php");
	}else{
		echo '请重新登录';
		header("Refresh:1;url=index.html");
	}
	
?>