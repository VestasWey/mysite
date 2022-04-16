<?php

	// get方式提取参数 --------------------------------------------------------------
	//echo "_GET -> " . $_GET["p0"] . "<br>";
	//echo "_GET -> " . $_GET["p1"] . "<br>";
	
	// post方式提取数据 --------------------------------------------------------------
	if ($_SERVER["REQUEST_METHOD"] == "POST")
	{
		//// enctype="multipart/form-data" 的时候php://input 是无效的
		$post_data = file_get_contents('php://input');
		echo "php://input<br>";
		echo $post_data . "<br>";

        echo "_REQUEST<br>";
        var_dump($_REQUEST);

        echo "_GET<br>";
        var_dump($_GET);

		echo "_POST<br>";
		var_dump($_POST);

		echo "_FILES<br>";
		var_dump($_FILES);
	}

    if($_FILES)
    {
        if ((($_FILES["file"]["type"] == "image/gif") ||
                ($_FILES["file"]["type"] == "image/jpeg") ||
                ($_FILES["file"]["type"] == "image/pjpeg") ||
                ($_FILES["file"]["type"] == "image/png")) &&
            ($_FILES["file"]["size"] < 200000))
        {
            if ($_FILES["file"]["error"] != 0)
            {
                echo "Return Code: " . $_FILES["file"]["error"] . "<br />";
            }
            else
            {
                if (file_exists("upload/" . $_FILES["file"]["name"])) {
                    echo $_FILES["file"]["name"] . " already exists. <br>";
                }
                else
                {
                    move_uploaded_file($_FILES["file"]["tmp_name"],
                        "upload/" . $_FILES["file"]["name"]);
                    echo "Stored in: " . "upload/" . $_FILES["file"]["name"] . "<br>" ;
                }
            }
        }
        else {
            echo " 文件无效!";
        }
    }
	
	echo "<br><br>  " . time() . "  client request details --------------------------------------------------------------<br><br>";
	var_dump($_SERVER);
?>