<?php
/*==============================================*/
/*	Archivo PHP para el proyecto IoTWS
/*	Por Daniel Nedosseikine para MakersUPV
/* 	Enero 2019								
/*==============================================*/

	/*Llamamos al archivo config.ini proporcionando la ruta necesaria y parseamos los datos en una variable*/
    $config = parse_ini_file(__DIR__ . '/../../private/config.ini');

	/*Establecemos la conexión con la base de datos*/
	$conn = new mysqli($config['servername'],$config['username'],$config['password'],$config['dbname']);

	/*Si se pordujo un error mostramos la información referente al mismo*/
	if (mysqli_connect_errno()) {
		echo "Failed to connect to MySQL: " . mysqli_connect_error();
	}
?>
