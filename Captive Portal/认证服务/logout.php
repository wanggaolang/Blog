<?php
 $myip = $_SERVER["REMOTE_ADDR"];
 #将此IP从IP集中删除
 exec("ipset del goodip $myip");
 session_start();
 unset($_SESSION['check']);
 header("Refresh:1;url=index.html");
 die("正在跳转登录界面...");
?>