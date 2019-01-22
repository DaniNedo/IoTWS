<?php
/*==============================================*/
/*	Archivo PHP para el proyecto IoTWS
/*	Por Daniel Nedosseikine para MakersUPV
/* 	Enero 2019
/*==============================================*/

	/*Incluimos el fichero "dbconnect.php"*/
	include 'dbconnect.php';

	/*Establecemos la zona horaria y guardamos la fecha en una variable*/
	date_default_timezone_set("Europe/Madrid");
	$date = date("Y-m-d G:i:s");

	/*Definimos las variables en las que guardamos los datos de la petición TCP IP*/
	$temperature = $_GET['temperature'];
	$humidity = $_GET['humidity'];
	$voltage = $_GET['voltage'];
	$sendmail = $_GET['sendMail'];

	/*Generamos una petición SQL que inserta los datos en la base de datos*/
	$query = "INSERT INTO weatherlog (`time`,`temperature`,`humidity`,`voltage`) VALUES ('$date','$temperature','$humidity','$voltage')";

	/*Lanzamos la petición*/
	$result = mysqli_query($conn, $query);

	/*Si no obtenemos un resultado, mostramos el error producido*/
	if(!$result) {
		 die("Error: " . mysqli_error($conn));
	}

	/*Cerramos la conexión con la base de datos*/
	mysqli_close($conn);

	/*Si la variable sendmail vale 1, enviamos un correo de alerta*/
	if($sendmail == 1){
		$to = $config['email'];
		$subject = "Alerta de esp8266!";
		$msg = "La batería de su dispostivo esp8266 está muy baja ($voltage V).\nConsidere cambiar la batería o su dispositivo se apagará :(";

		mail($to,$subject,$msg);
	}
?>
