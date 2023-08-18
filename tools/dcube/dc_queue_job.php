<?php

$base = $argv[17] . "/api/";
$key = "?key=" . $argv[1];

$act = "queue/create_job";

$job = array(
    "protocol"        => intval($argv[2]),
    "layout"          => $argv[3], 
    "periodicity"     => intval($argv[4]),
    "message_length"  => intval($argv[5]),
    "patching"        => intval($argv[6]),
    "name"            => $argv[7],
    "description"     => $argv[8],
    "duration"        => intval($argv[9]),
    "logs"            => intval($argv[10]),
    // "jamming"         => $argv[11], // jamming is a STR in Graz
    "jamming"         => intval($argv[11]),
    "priority"        => boolval($argv[12]),
    "file"            => base64_encode(file_get_contents($argv[13]))
);
if($argv[14]) {
  $job["temp_profile"] = $argv[14];
}
$overrides = array();
if($argv[15]) {
  $overrides["start"] = intval($argv[15]);
}
if($argv[16]) {
  $overrides["delta"] = intval($argv[16]);
}
if(sizeof($overrides)) {
  $job["config_overrides"] = $overrides;
}

$payload = json_encode($job);

$options = array(
    'http' => array(
    'method'  => 'POST',
    'content' => $payload,
    'header'=>  "Content-Type: application/json\r\n"
  )
);

$context  = stream_context_create($options);
$result = file_get_contents($base . $act . $key, false, $context);
$response = json_decode($result, true);

echo "Job created, got response:" . PHP_EOL;
var_dump($result);
echo "Job ID: " . $response['id'] . PHP_EOL;

return $response['id'];
