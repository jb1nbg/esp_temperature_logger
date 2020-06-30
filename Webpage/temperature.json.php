<?php

  ini_set('display_errors', '0');
  ini_set('display_startup_errors', '0');
  error_reporting(0);

  include('account.php');

  $servername = "localhost";
  $dbname = "temperature";

  $temperature_data = array();
  $pressure_data = array();
  $humidity_data = array();

  $db = new mysqli($servername, $username, $password, $dbname);
  //$db = new SQLite3('measurement.db');
  
  /*
  $start_time = 0;
  $first = $db->query("SELECT * FROM data WHERE host = '192.168.178.30' ORDER BY id ASC LIMIT 1");
  //if ($row = $first->fetchArray()) {
  if ($row = $first->fetch_assoc()) {
    $start_time = strtotime($row['timestamp']);
  }
  */

  $filterDate = "";
  $date = "";
  if ($_SERVER["REQUEST_METHOD"] == "GET") 
  {
    if(isset($_GET['date'])) 
    {
      $date = test_input($_GET["date"]);
      if ($date != "")
      {
        $filterDate = "and cast(timestamp as date) = '" . $date . "'";
      }
    }
  }

  $res = $db->query("SELECT * FROM data WHERE host = '192.168.178.30' " . $filterDate);

  //while ($row = $res->fetchArray()) {
  while($row = $res->fetch_assoc()) 
  {
    /*
    $timestamp = strtotime($row['timestamp']);
    $diff = $timestamp - $start_time;
    */
    $diff = strtotime($row['timestamp']);
    array_push($temperature_data, array($diff, $row['temperature']));
    array_push($pressure_data, array($diff, $row['pressure']));
    array_push($humidity_data, array($diff, $row['humidity']));
  }

  $myObj->temperature = array("label" => "temperature", "data" => $temperature_data);
  $myObj->pressure = array("label" => "pressure", "data" => $pressure_data, "yaxis" => 2);
  $myObj->humidity = array("label" => "humidity", "data" => $humidity_data, "yaxis" => 3);

  $myJSON = json_encode($myObj);

  header("Content-Type: application/json");
  echo $myJSON;

  function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
  }

?>
