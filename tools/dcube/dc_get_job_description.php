<?php

// api key
$base = $argv[3] . "/api/";
$key = "?key=" . $argv[1];
$act = "queue/" . $argv[2];

$options = array(
    'http' => array(
    'method'  => 'GET',
    'header'=>  "Content-Type: application/json\r\n"
  )
);

$context  = stream_context_create($options);
$result = @file_get_contents($base . $act . $key, false, $context);


if($result === FALSE) {
  // NB: We read the stdout in the bash to get the description, so we can't
  //     echo the error here.
  // $error = error_get_last();
  // echo "HTTP request failed. Error was: " . $error['message'];
  echo "";
} else {
  $response = json_decode($result, true);
  $description = str_replace(",","",$response['description']);
  echo $description;
}
