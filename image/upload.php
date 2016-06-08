<?php
if($_FILES["file"]["size"] < 10000000) {
	if ($_FILES["file"]["error"] > 0) {
		echo "Return Code: " . $_FILES["file"]["error"] . "<br />";
	}
	else {
		$error = 0;
		if("rtu_a118" == $_FILES["file"]["name"]) {
			move_uploaded_file($_FILES["file"]["tmp_name"],
				"upload/" . $_FILES["file"]["name"]);
			system("killall -9 rtu_a118");
			system("chmod +x upload/rtu_a118");
			system("cp upload/rtu_a118 /bin");
			system("rtu_a118 1>/dev/null 2>&1 &");
		} else if("image.tar" == $_FILES["file"]["name"]) {
			move_uploaded_file($_FILES["file"]["tmp_name"],
				"upload/" . $_FILES["file"]["name"]);
			system("tar xf upload/image.tar -C ~");
			system("cd ~/image;./initsys;rm -rf ~/image;rm -rf /var/www/upload/image.tar");
		} else if(0 == strncmp("map", $_FILES["file"]["name"], strlen("map"))) {
			move_uploaded_file($_FILES["file"]["tmp_name"],
				"upload/" . $_FILES["file"]["name"]);
			system("cp upload/" . $_FILES["file"]["name"] . " /usr/share");
		} else if(0 == strncmp("config", $_FILES["file"]["name"], strlen("config"))) {
			move_uploaded_file($_FILES["file"]["tmp_name"],
				"upload/" . $_FILES["file"]["name"]);
			system("cp upload/" . $_FILES["file"]["name"] . " /usr/share");
		} else if(0 == strncmp("upload", $_FILES["file"]["name"], strlen("upload"))) {
			move_uploaded_file($_FILES["file"]["tmp_name"],
				"" . $_FILES["file"]["name"]);
		} else {
			$error = -1;
		}
		if(0 == $error) {
			echo "Upload: " . $_FILES["file"]["name"] . "<br />";
		    echo "Type: " . $_FILES["file"]["type"] . "<br />";
		    echo "Size: " . ($_FILES["file"]["size"] / 1024) . " Kb<br />";
		    echo "Temp file: " . $_FILES["file"]["tmp_name"] . "<br />";
			echo "Stored in: " . "upload/" . $_FILES["file"]["name"] . "<br />";
		} else {
			echo "file name error <br />";
		}
	}
} else {
	echo "file too big <br />";
}
?>