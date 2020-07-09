<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<title>Temperature logger</title>
	<link href="examples.css" rel="stylesheet" type="text/css">
	<script language="javascript" type="text/javascript" src="jquery/jquery.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.canvaswrapper.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.colorhelpers.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.saturated.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.browser.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.drawSeries.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.time.js"></script>	
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.uiConstants.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.legend.js"></script>	
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.axislabels.js"></script>
	<script language="javascript" type="text/javascript" src="jquery/jquery.flot.tickrotor.js"></script>	

    <style>
    .x1Label {
        fill: black;
        font-size: 20px;
        font-family: Tahoma, Geneva, sans-serif;
    }
    .y1Label {
        fill: blue;
        font-size: 20px;
        font-family: Tahoma, Geneva, sans-serif;
    }
    .y2Label {
        fill: green;
        font-size: 20px;
        font-family: Tahoma, Geneva, sans-serif;
    }
	.y3Label {
        fill: red;
        font-size: 20px;
        font-family: Tahoma, Geneva, sans-serif;
    }
    </style>
	
	
	<script type="text/javascript">
    
$(function() {

	function percentFormatter(v, axis) {
	    return v.toFixed(2) + "%";
	}

    function pressureFormatter(v, axis) {
	    return v.toFixed(2) + "mBar";
	}

	function temperatureFormatter(v, axis) {
	    return v.toFixed(2) + "°C";
	}

	var options = {
	    series: {
		lines: {
		    show: true,
		    lineWidth: 2
		},
		points: {
		    show: false
		},
	    },
	    xaxis: {
				//autoScale: "none",
                color: "black",
                position: 'bottom',
                axisLabel: 'Time',
				//rotateTicks: 90,
				mode: "time",
				minTickSize: [30, "minute"],
				min: (new Date(2020, 6, 8, 1, 0, 0)),
				max: (new Date(2020, 6, 9, 1, 0, 0)),
				timeformat: "%Y/%m/%d %H:%M",
				timeBase: "seconds"
	    },
	    yaxes: [ {
                      min: 0,
                      color: "black",
                      tickFormatter: temperatureFormatter,
                      position: 'left', 
                      axisLabel: 'Temperature'
                    }, 
                    {
                      	color: "black",
						alignTicksWithAxis: 1,
						position: "right",
						tickFormatter: pressureFormatter,
						axisLabel: 'Barometric Pressure', 
						show: true,  
						showTicks: true, 
						gridLines: false
					}, 
                    {
                      	color: "black",
						alignTicksWithAxis: 1,
						position: "right",
						tickFormatter: percentFormatter,
						axisLabel: 'Humidity', 
						show: true,  
						showTicks: true, 
						gridLines: false
					}],
            legend: {
                position: "ne",
                show: true
            },
            colors: [ "blue", "green", "red" ]
	}

	function dataUpdate() {

		data = [];

		$.plot("#placeholder", data, options);

		function fetchData() {

			function onDataReceived(series) {

				data = [];
				data.push(series["temperature"]);
				data.push(series["pressure"]);
				data.push(series["humidity"]);

				$.plot("#placeholder", data, options);
			}

			$.ajax({
<?php
	$date = "";
	if ($_SERVER["REQUEST_METHOD"] == "GET") 
	{
		if(isset($_GET['date'])) 
		{
			$date = test_input($_GET["date"]);
			if ($date != "")
			{
				echo "\t\t\t\turl: 'temperature.json.php?date=" . $date . "',\n";
			}
		}
		else
			echo "url: 'temperature.json.php',";
	}

	function test_input($data) 
	{
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
?>
				type: "GET",
				dataType: "json",
				cache: false,
				success: onDataReceived
			});

			setTimeout(fetchData, 1000);
		}

		setTimeout(fetchData, 1000);
	}

	$(document).ready(function() {
	    dataUpdate();
	})

});

	</script>
</head>
<body>
<?php

include('account.php');

$servername = "localhost";
$dbname = "temperature";

echo "<a href=/>All</a>&nbsp;";

$db = new mysqli($servername, $username, $password, $dbname);
$res = $db->query("SELECT cast(timestamp AS DATE) AS date FROM data GROUP BY cast(timestamp AS DATE)");

//while ($row = $res->fetchArray()) {
while($row = $res->fetch_assoc()) 
{
	echo "<a href=/?date=" . $row['date'] . ">" . $row['date'] . "</a>&nbsp;";
}
?>

	<div id="header">
		<h2>Temperature logger</h2>
	</div>

	<div id="content">

		<div class="demo-container">
			<div id="placeholder" class="demo-placeholder"></div>
		</div>

	</div>

</body>
</html>
