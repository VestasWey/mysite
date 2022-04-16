<?php

    setrawcookie("name", "testUser");
    $ary = array(
        "user" => "wh",
        "nick" => "bean",
        "info" => array(
            "mail" => "www@qq.com",
            "phone" => "12131231231"
            )
        );
    setrawcookie("params", "details", time() + 60);
    var_dump($_COOKIE);

	//require "index.php";

	//echo "Hello World!";
	
	// json解析 --------------------------------------------------------------
	//$arr = array(
	//	'name' => '魏艳辉',
	//	'nick' => '为梦翱翔',
	//	'contact' => array(
	//		'email' => 'zhuoweida@163.com',
	//		'website' => 'http://zhuoweida.blog.tianya.cn',
	//	)
	//);
	//$json_string = json_encode($arr);
	//echo "$json_string";
	
	// get方式提取参数 --------------------------------------------------------------
	//echo "_GET -> " . $_GET["p0"] . "<br>";
	//echo "_GET -> " . $_GET["p1"] . "<br>";
	
	// post方式提取数据 --------------------------------------------------------------
	//if ($_SERVER["REQUEST_METHOD"] == "POST") 
	//{
	//	$post_data = file_get_contents('php://input');
	//	// always_populate_raw_post_data=On
	//	//$post_data = $GLOBALS['HTTP_RAW_POST_DATA'];
	//	echo "raw post data<br>";
	//	echo $post_data . "<br>";
	//	
	//	echo "json_decode data<br>";
	//	$josn_data = json_decode(trim($post_data), true);
	//	var_dump($josn_data) . "<br>";
	//	
	//	echo $josn_data["name"] . "<br>";
	//	echo $josn_data["nick"] . "<br>";
	//}
	
	// 函数定义、for使用 --------------------------------------------------------------
	//$p0 = "p0";
	//$p1 = "p1";	
	//function test_fn()
	//{
	//	//echo $GLOBALS['p0'] . "<br>";
	//	
	//	//global $p1;		
	//	//echo $p1;
	//	
	//	static $p2 = 22;
	//	$p2++;
	//	echo $p2 . "<br>";
	//}
	//for($i = 0; $i < 2; $i++)
	//{
	//	test_fn();
	//}
	
	// foreach使用
	//$vct = array("f", "s", "t");
	//foreach($vct as $item)
	//{
	//	echo $item . "<br>";
	//}
	
	// 接口、类 --------------------------------------------------------------
	//abstract class IInterface
	//{
	//	function __construct($name)
	//	{
	//		$this->name = $name;
	//		
	//		static $id = 0;
	//		$this->id_ = ++$id;
	//		
	//		echo $this->id_ . " IInterface::__construct<br>";
	//		
	//		register_shutdown_function(array($this, "before_destruct"));			
	//	}
	//	
	//	function __destruct()
	//	{
	//		echo $this->id_ . " IInterface::__destruct<br>";
	//	}
	//	
	//	function myname()
	//	{
	//		echo $this->name . "<br>";
	//	}
	//	
	//	function myid()
	//	{
	//		echo "id = " . $this->id_ . "<br>";
	//	}
	//	
	//	function before_destruct()
	//	{
	//		echo $this->id_ . " IInterface::before_destruct<br>";
	//	}
	//	
	//	var $name;
	//	var $id_;
	//}
	//class CImpl extends IInterface
	//{
	//	function __construct($name = "no_name")
	//	{
	//		parent::__construct($name);
	//	}
	//	
	//	function __destruct()
	//	{			
	//		echo $this->id_ . " CImpl::__destruct<br>";
	//		
	//		parent::__destruct();
	//	}		
	//}
	//for($i = 0; $i < 2; $i++)
	//{
	//	$impl = new CImpl();
	//	$impl->myid();
	//}
	
	// 数组定义 --------------------------------------------------------------
	//$cars = array("Volvo", 121, 3.14, 5.96e2, 0xf1, 07);
	//echo var_dump($cars) . "<br>";
	//$x = 1;
	//var_dump($x);
	
	//// mysql->pdo --------------------------------------------------------------
	//class actor
	//{
	//	public $id;
	//	public $name;
	//	public $date;
	//	
	//	public function to_string()
	//	{
	//		return $this->id . "\t" . $this->name . "\t" . $this->date;
	//	}
	//}
	////$pdo = new PDO("mysql:host=localhost;dbname=sakila", "Usopp", "Wh520mysql");
	//$pdo = new PDO("mysql:host=localhost", "Usopp", "Wh520mysql");
	//// create ---------
	//$result = $pdo->exec("drop database if exists php_test_db;");
	//$result = $pdo->exec("create database if not exists php_test_db default charset utf8;");
	//$result = $pdo->exec("use php_test_db;");
	//echo "create database " . var_dump($pdo->errorinfo()) . "<br>";
	//$result = $pdo->exec("drop table if exists user_info;");
	//$sql = "create table if not exists user_info(
	//	id int auto_increment primary key,
	//	name varchar(32) not null,
	//	date timestamp
	//)ENGINE=InnoDB";
	//$count = $pdo->exec($sql);
	//echo "create table " . var_dump($pdo->errorinfo()) . "<br>";
	//// insert ---------
	//$stmt = $pdo->prepare("insert into user_info (name) values (:name)");
	//$usopp = "usopp";
	//$stmt->bindParam(":name", $usopp, PDO::PARAM_STR);
	//$stmt->execute();
	//echo "insert table " . var_dump($stmt->errorInfo()) . "<br>";	
	//// select --------------
	//// 直接执行语句 =======
	//$stmt = $pdo->query("select * from user_info limit 0, 10");
	//// fetch array =======
	////$result = $stmt->fetchAll();
	////var_dump($result);
	//
	////while ($row = $stmt->fetch()) {
	////	echo "row data: ";
	////	var_dump($row);
	////	echo "<br>";
	////}
	//
	//// fetch class =======
	//$stmt->setFetchMode(PDO::FETCH_CLASS, "actor");
	//while ($user = $stmt->fetch()) {  
	//	echo "user: " . $user->to_string() . "<br>";
	//}
	//// update ---------
	//$stmt = $pdo->prepare("update user_info set name = :name where id = 1;");
	//$usopp = "usopp---";
	//$stmt->bindParam(":name", $usopp, PDO::PARAM_STR);
	//$stmt->execute();
	//echo "update table " . var_dump($stmt->errorInfo()) . "<br>";
	//// delete & begintrans ---------
	//$pdo->beginTransaction();
	//$count = $pdo->exec("delete from user_info where id = 1;");
	//$count = $pdo->exec("delete from user_info where id_ = 1;");
	//$pdo->rollBack();
	//echo "delete & begintrans data " . var_dump($pdo->errorInfo()) . "<br>";
	
	
	echo "<br><br>  " . time() . "  client request details --------------------------------------------------------------<br><br>";
	echo "REQUEST_METHOD -> " . $_SERVER['REQUEST_METHOD'] . "<br>";
	echo "QUERY_STRING -> " . $_SERVER['QUERY_STRING'] . "<br>";
	echo "SERVER_NAME -> " . $_SERVER['SERVER_NAME'] . "<br>";
	echo "REMOTE_PORT -> " . $_SERVER['REMOTE_PORT'] . "<br>";
	echo "PHP_SELF -> " . $_SERVER['PHP_SELF'] . "<br>";
	echo "argv -> " . $_SERVER['argv'] . "<br>";
	echo "argc -> " . $_SERVER['argc'] . "<br>";
	echo "SERVER_SOFTWARE -> " . $_SERVER['SERVER_SOFTWARE'] . "<br>";
	echo "SERVER_PROTOCOL -> " . $_SERVER['SERVER_PROTOCOL'] . "<br>";
	echo "HTTP_ACCEPT -> " . $_SERVER['HTTP_ACCEPT'] . "<br>";
	echo "HTTP_ACCEPT_CHARSET -> " . $_SERVER['HTTP_ACCEPT_CHARSET'] . "<br>";
	echo "HTTP_ACCEPT_ENCODING -> " . $_SERVER['HTTP_ACCEPT_ENCODING'] . "<br>";
	echo "HTTP_ACCEPT_LANGUAGE -> " . $_SERVER['HTTP_ACCEPT_LANGUAGE'] . "<br>";
	echo "HTTP_USER_AGENT -> " . $_SERVER['HTTP_USER_AGENT'] . "<br>";
	echo "SERVER_PORT -> " . $_SERVER['SERVER_PORT'] . "<br>";
	echo "SCRIPT_NAME -> " . $_SERVER['SCRIPT_NAME'] . "<br>";
	echo "HTTP_HOST -> " . $_SERVER['HTTP_HOST'] . "<br>";
	echo "REMOTE_ADDR -> " . $_SERVER['REMOTE_ADDR'] . "<br>";
	echo "CONTENT_TYPE -> " . $_SERVER['CONTENT_TYPE'] . "<br>";
	echo "CONTENT_LENGTH -> " . $_SERVER['CONTENT_LENGTH'] . "<br>";	
?>