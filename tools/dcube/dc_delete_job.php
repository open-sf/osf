<?php

// api key
$base = $argv[3] . "/api/";
$key = "?key=" . $argv[1];

$act = "queue/" . $argv[2];

$options = array(
    'http' => array(
    'method'  => 'DELETE',
    'header'=>  "Content-Type: application/json\r\n"
  )
);

$context  = stream_context_create($options);
$result = file_get_contents($base . $act . $key, false, $context);
$response = json_decode($result, true);

echo "Job deleted, got response:" . PHP_EOL;
echo "DELETED job with ID: " . $response['id'] . PHP_EOL;
return $response['id'];
