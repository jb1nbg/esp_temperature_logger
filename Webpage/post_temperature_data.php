<?php

include('account.php');

$servername = "localhost";
$dbname = "temperature";

// Keep this API Key value to be compatible with the ESP32 code provided in the project page. 
// If you change this value, the ESP32 sketch needs to match
$api_key_value = "temperature_logger";

$api_key= $TIMESTAMP = $TEMPERATURE = $PRESSURE = $HUMIDITY = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $TIMESTAMP = test_input($_POST["timestamp"]);
        $TEMPERATURE = test_input($_POST["temperature"]);
        $PRESSURE = test_input($_POST["pressure"]);
        $HUMIDITY = test_input($_POST["humidity"]);
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO data (host, timestamp, temperature, pressure, humidity) VALUES ('" . $_SERVER['REMOTE_ADDR'] . "', '" . $TIMESTAMP . "', '" . $TEMPERATURE . "', '" . $PRESSURE . "', '" . $HUMIDITY . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }

}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
 
