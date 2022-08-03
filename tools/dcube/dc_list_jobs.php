<?php

$base = $argv[6] . "/api/";
$key = "?key=" . $argv[1];

$options = array(
    'http' => array(
    'method'  => 'GET',
    'header'=>  "Content-Type: application/json\r\n"
  )
);

$context  = stream_context_create($options);

$params = "";
# --days=*
if ($argv[2]) {
  echo "Filter " . $argv[2] . " days of jobs..." . PHP_EOL;
  $act = "metric/adv";
  $stop = time();
  $start = strtotime('-' . $argv[2] . ' day', $stop);
  $params = "&stop=" . $stop . "&start=" . $start;
}
# -n ""
if ($argv[3]) {
  echo "Filter jobs with name: " . $argv[3] . PHP_EOL;
  $act = "metric/adv";
  $params = "&name=" . $argv[3];
}
# --last=*
if ($argv[5]) {
  echo "Filter last " . $argv[5] . " jobs..." . PHP_EOL;
  $act = "metric/adv/" . $argv[5];
}
if(!$argv[2] && !$argv[3] && !$argv[5]) {
  echo "No filter. Getting ALL jobs..." . PHP_EOL;
  $act = "metric";
}
$url = $base . $act . $key . $params;
echo "=====================" . PHP_EOL;
echo $url . PHP_EOL;
echo "=====================" . PHP_EOL;
$result = file_get_contents($url, false, $context);
$response = json_decode($result, true);

print_r($response);

/*---------------------------------------------------------------------------*/
# Convert to CSV
echo "Fetched " . count($response) . " jobs..." . PHP_EOL;
echo "Write results to CSV... VERBOSE=" . $argv[3] . PHP_EOL;
# -v Verbose
if($argv[4]) {
  $formatted = format_data($response, $act);
  $csv = print_r(str_putcsv($formatted, 1), true);
  # Create a reduced version of this for a readable CSV we can print to bash
  $formatted_readable = array();
  foreach ($formatted as $row) {
    $row_readable = array(
        'id' => $row['id'],
        'name' => $row['name'],
        'description' => $row['description'],
        'proto' => $row['protocol'],
        'jam' => $row['jamming'],
        'len' => $row['message_length'],
        'period' => $row['periodicity'],
        'eval' => $row['evaluated'],
        'fin' => $row['finished']
    );
    $formatted_readable[] = $row_readable;
  }
  $csv_readable = print_r(str_putcsv($formatted_readable, 1), true);
  file_put_contents('dcube_jobs_list_readable.csv', $csv_readable);
} else {
  $formatted = format_job_id($response);
  $csv = print_r(str_putcsv($formatted, 0), true);
}

# Write to file
file_put_contents('dcube_jobs_list.csv', $csv);

/*---------------------------------------------------------------------------*/
/* Array to CSV */
/*---------------------------------------------------------------------------*/
function str_putcsv($data, $headers) {
  # Generate CSV data from array
  $fh = fopen('php://temp', 'rw'); # don't create a file, attempt
                                   # to use memory instead
  if ($headers) {
    // print_r(array_keys(current($data)));
    # write out the headers
    fputcsv($fh, array_keys(current($data)));
  }

  # write out the data
  foreach ( $data as $row ) {
    if(is_array($row)) {
      fputcsv($fh, array_values($row));
    } else {
      fputcsv($fh, array('id', $row));
    }
  }
  rewind($fh);
  $csv = stream_get_contents($fh);
  fclose($fh);

  return $csv;
}

/*---------------------------------------------------------------------------*/
function arr_get($array, $key, $default = ""){
    return isset($array[$key]) ? $array[$key] : $default;
}

/*---------------------------------------------------------------------------*/
function format_data($array, $act) {
  $result = array();
  foreach ( $array as $row ) {
    unset($row['job']['result']);
    foreach ($row['job'] as $key => $value) {
        if ($value == false) {
            $row['job'][$key] = 0;
        }
    }
    if (!array_key_exists('periodicity', $row['job'])) {
      $row['job']['periodicity'] = 0;
    }
    foreach ($row['job'] as $el) {
      $el = arr_get($row['job'], $el);
    }
    array_push($result, $row['job']);
  }
  return $result;
}

/*---------------------------------------------------------------------------*/
function format_job_id($array) {
  $result = array();
  foreach ($array as $row) {
    $el = array();
    $el['id'] = $row['job']['id'];
    array_push($result, $el);
  }
  // print_r($array);
  return $result;
}
