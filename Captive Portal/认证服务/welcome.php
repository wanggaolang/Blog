<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<title>首页</title>
</head>
<body>
	<?php
	session_start();
	if(!isset($_SESSION['check']) || $_SESSION['check'] != true){
		echo '没登录就想看？';
		header("Refresh:3;url=index.html");
		die;
	}
    ?>
	<h1>欢迎来到首页</h1><br>
	<a href="logout.php"><b>下线</b></a>
</body>
</html>