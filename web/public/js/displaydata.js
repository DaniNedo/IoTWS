/*==============================================*/
/*	Archivo JavaScript para el proyecto IoTWS	*/
/*	Por Daniel Nedosseikine para MakersUPV		*/
/* 	Mayo 2018									*/
/*==============================================*/

//Cuando el documento esté listo ejecutamos el siguiente script
document.onready = function () {
	//Creamos dos vectores que almacenen los puntos que aparecen en las gráficas
	var tempPoints = [];
	var humPoints = [];

	//Creamos tres variables que almacenen el voltaje y las medias de temperatura y humedad
	var voltage;
	var avgTemp;
	var avgHum;


	//Creamos una función que se llame getData
	function getData(){
		//Iniciamos una petición de AJAX
		$.ajax({
			//Indicamos la url del fichero php al cual hacemos la petición
			url: "../php/readdata.php" ,
			//En caso de que la petición sea exitosa ejecutamos la siguiente funcion pasandole el parametro "result"
			success : function(result) {
				//Ponemos la temperatura y humedad medias a cero
				avgTemp = 0;
				avgHum = 0;

				//Creamos una variable que almacene los datos obtenidos al parsear "result"
				var data = jQuery.parseJSON(result);

				//Asignamos a la variable "voltage" el valor del último parámetro del array "data"
				voltage = data[data.length-1].v;
				//Recorremos todos los elementos de "data" salvo el último
				for (var i = 0; i < data.length-1; i++) {
					//Insertamos el punto actual con formato var[temperatura,tiempo] al final del vector "tempPoints"
					tempPoints.push({
						x: data[i].t,
						y: data[i].temp
					});
					//Insertamos el punto actual con formato var[humedad,tiempo] al final del vector "humPoints"
					humPoints.push({
						x: data[i].t,
						y: data[i].hum
					});

					//Sumamos la temperatura y la humedad a la correspondiente variable media
					avgTemp += data[i].temp;
					avgHum += data[i].hum;
				}

				//Llamamos a la función de reenderizado de la gráfica de temeperatura
				tempChart.render();
				//Llamamos a la función de reenderizado de la gráfica de humedad
				humChart.render();

				//Dividimos "avgXXX" entre el número de puntos para obtener la media
				avgTemp /= (data.length-1);
				avgHum /= (data.length-1);

				//Aproximamos al segundo decimal
				voltage = voltage.toFixed(2);
				avgTemp = avgTemp.toFixed(2);
				avgHum = avgHum.toFixed(2);

				//Actualizamos los elementos dinámicos de la página web
				document.getElementById("voltage").innerHTML = "El voltaje es de: " + voltage + "V";
				document.getElementById("temp").innerHTML = "La temperatura media es de: " + avgTemp + "ºC";
				document.getElementById("hum").innerHTML = "La humedad media es de: " + avgHum + "%";
			}
		});
	}

	//Invocamos a la función "getData"
	getData();

	//Creamos la gráfica de temperatura
	var tempChart = new CanvasJS.Chart("tempChart", {
		//Pequeña animación al cargar los datos
		animationEnabled: true,
		//Modificamos los parámetros del título
		title:{
			//Seleccionamos la fuente
			fontFamily: "arial",
			//Indicamos el contenido del título
			text: "Temperatura"
		},
		//Leyenda del eje Y
		axisY: {
			title: "(ºC)"
		},
		//Damos formato a los datos
		data: [{
			//Tipo de línea: spline. Puntos unidos con una línea suave
			type: "spline",
			//Tamaño de los puntos 5
			markerSize: 5,
			//Formato en el que se muestra la hora: horas:minutos
			xValueFormatString: "hh:mm",
			//Tipo de dato para el eje X
			xValueType: "dateTime",
			//Vector del cual se obtienen los puntos a mostrar
			dataPoints: tempPoints
		}]
	});

	//Creamos la gráfica de humedad
	var humChart = new CanvasJS.Chart("humChart", {
		//Pequeña animación al cargar los datos
		animationEnabled: true,
		//Modificamos los parámetros del título
		title:{
			//Seleccionamos la fuente
			fontFamily: "arial",
			//Indicamos el contenido del título
			text: "Humedad"
		},
		//Leyenda del eje Y
		axisY: {
			title: "(%)"
		},
		//Damos formato a los datos
		data: [{
			//TIpode de línea: splie. Puntos unidos con una línea suave
			type: "spline",
			//Tamaño de los puntos 5
			markerSize: 5,
			//Formato en el que se muestra la hora: horas:minutos
			xValueFormatString: "hh:mm",
			//Tipode de datos para el eje X
			xValueType: "dateTime",
			//Vector del cual se obtienen los puntos a mostrar
			dataPoints: humPoints
		}]
	});
};
