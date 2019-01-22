<?php
/*==============================================*/
/*	Archivo PHP para el proyecto IoTWS
/*	Por Daniel Nedosseikine para MakersUPV
/* 	Enero 2019
/*==============================================*/

	/*Incluimos el fichero "dbconnect.php"*/
	include __DIR__ . "/dbconnect.php";

	/*Establecemos la zona horaria y guardamos la fecha en una variable*/
	date_default_timezone_set('Europe/Madrid');

	/*Inicializamos la variable "$data"*/
	$data = [];

	/*Generamos una petición SQL que solicite las últimas 144 entradas*/
	$query = "SELECT time, temperature, humidity, voltage
						FROM weatherlog
						order by time desc limit 144";

	/*Lanzamos la petición*/
	$result = mysqli_query($conn, $query);

	/*Si no obtenemos un resultado, mostramos el error producido*/
	if(!$result) {
		 echo "Error: " . mysqli_error($conn);
	}

	/*Mientras que existan filas de la respuesta por leer...*/
	while($row = mysqli_fetch_array($result, MYSQLI_ASSOC)){
		/*Guardamos en valor de voltage de voltaje*/
		$voltage = floatval($row['voltage']);

		/*Insertamos el valore de tiempo, temperatura y humedad en el vector "$data"*/
		array_push($data,["t" => (strtotime($row['time'])*1000), "temp" => floatval($row['temperature']), "hum" => floatval($row['humidity'])]);

		/*Medimos la longitud de la variable "$data"*/
		$size = sizeof($data,0);

		/*Si la longitud es superior a 1 y existe una diferencia superior a 20 minutos entre el valor de tiempo actual y el anterior...*/
		if($size>1 && $data[$size-1]["t"] - $data[$size-2]["t"] > 1200000){

			/*Insertamos un punto nulo entre el valor actual y el anterior*/
			array_splice($data,($size-1),0,[["t" => $data[$size-2]["t"]+1000, "temp" => null, "hum" => null]]);
		}
	}

	/*Insertamos el valore de voltaje como última entrada de la variable "$data"*/
	array_push($data,["v" => floatval($voltage)]);

	/*Codificamos el vector "$data" en formato JSON y lo envíamos como respuesta*/
	echo json_encode($data);

	/*Vaciamos la variable "$result"*/
	mysqli_free_result($result);

	/*Cerramos la conexión con la base de datos*/
	mysqli_close($conn);
?>
