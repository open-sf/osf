<?php

function is_multi2($a) {
    foreach ($a as $v) {
        if (is_array($v)) return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/
/* Array to CSV */
/*---------------------------------------------------------------------------*/
function str_putcsv($data, $headers) {
  # Generate CSV data from array
  $fh = fopen('php://temp', 'rw'); # don't create a file, attempt
                                   # to use memory instead
  $multi = false;
  foreach ( $data as $row ) {
    if(is_array($row)) {
      $multi = true;
    }
  }
  if($multi) {
    if ($headers) {
      fputcsv($fh, array_keys(current($data)));
    }
    # write out the data
    foreach ( $data as $row ) {
            fputcsv($fh, $row);
    }
  } else {
    if ($headers) {
      fputcsv($fh, array_keys($data));
    }
    fputcsv($fh, $data);
  }
  rewind($fh);
  $csv = stream_get_contents($fh);
  fclose($fh);

  return $csv;
}

/*---------------------------------------------------------------------------*/
/* Array to CSV */
/*---------------------------------------------------------------------------*/
function combine_data($scenario, $queue) {
  $result = array();
  foreach ( $scenario as $row ) {
    $row['id'] = $queue['id'];
    $row['name'] = $queue['name'];
    $row['proto'] = $queue['protocol'];
    $row['description'] = $queue['description'];
    $row['jamming'] = $queue['jamming'];
    $row['layout'] = $queue['layout'];
    $row['len'] = $queue['message_length'];
    // $row['periodicity'] = $queue['periodicity'];
    array_push($result, $row);
  }
  return $result;
}

/*---------------------------------------------------------------------------*/
/* Main */
/*---------------------------------------------------------------------------*/
// api key
$base = $argv[7] . "/api/";
$key = "?key=" . $argv[1];

$options = array(
    'http' => array(
    'method'  => 'GET',
    'header'=>  "Content-Type: application/json\r\n"
  )
);
$context  = stream_context_create($options);

$params = "";
if ($argv[5]) {
  echo "Filter " . $argv[5] . " days of jobs..." . PHP_EOL;
  $act = "metric";
  $stop = time();
  $start = strtotime('-' . $argv[5] . ' day', $stop);
  $params = $params . "&stop=" . $stop . "&start=" . $start;
}
if ($argv[6]) {
  echo "Filter jobs with name: " . $argv[6] . PHP_EOL;
  $params = $params . "&name=" . $argv[6];
}

/*---------------------------------------------------------------------------*/
$act = "/scenario/" . $argv[2];
$result = file_get_contents($base . $act . $key . $params, false, $context);
$scenario = json_decode($result, true);

// print_r($scenario);

# Simplify the key for CSV headers
$scenario = array_map(function($scenario) {
    return array(
        'src' => $scenario['Source node(s)'],
        'dest' => $scenario['Destination node(s)'],
        'gpio' => $scenario['GPIO pin'],
        'reliability' => $scenario['Reliability [%]'],
        'sent' => $scenario['Messages sent to source node'],
        'received' => $scenario['Messages received on sink node'],
        'correct' => $scenario['Correct messages'],
        'missed' => $scenario['Missed messages'],
        'superfluous' => $scenario['Superflous messages'],
        'causality' => $scenario['Messages with causality error'],
        'lat_combined' => $scenario['Latency combined [us]'],
        'lat_mean' => $scenario['Latency mean [us]'],
        'lat_median' => $scenario['Latency median [us]'],
        'lat_90' => $scenario['Latency 90 Percentile [us]'],
        'lat_95' => $scenario['Latency 95 Percentile [us]'],
        'lat_99' => $scenario['Latency 99 Percentile [us]'],
        'energy_total' => $scenario['Total Energy [J]'],
        'energy_setup' => $scenario['Energy during setup time [J]'],
    );
}, $scenario);


/*---------------------------------------------------------------------------*/
$act = "/queue/" . $argv[2];
$result = file_get_contents($base . $act . $key . $params, false, $context);
$queue = json_decode($result, true);

// print_r($queue);

// /*---------------------------------------------------------------------------*/
// $act = "/metric/" . $argv[1];
// $result = file_get_contents($base . $act . $key . $params, false, $context);
// $metric = json_decode($result, true);

// print_r($metric);

/*---------------------------------------------------------------------------*/
# Combine data
$result = combine_data($scenario, $queue);


# If we are MP2P then the last element of the results will show the combined
# results, which will have a list of destination nodes separated by '*'. Look
# for this and if you find it then discard the rest.
if ((count($result) > 1) && is_array($result[0])) {
    $dest = $result[count($result)-1]['dest'];
    if(strpos($dest, '*') !== false) {
      $result = $result[count($result)-1];
    }
} else {
  $result = $result[0];
}



# Create a reduced version of this for a readable CSV we can print to bash
$result_readable = array(
    'id' => $result['id'],
    'name' => $result['name'],
    'reliability' => $result['reliability'],
    'latency' => round(floatval($result['lat_combined'])/1000, 2),
    'energy' => round($result['energy_total'], 2),
    'jamming' => $result['jamming'],
    'layout' => $result['layout'],
    'len' => $result['len'],
    'description' => $result['description'],
);

$result_plotting = array(
    'id' => $result['id'],
    'name' => $result['name'],
    'layout' => $result['layout'],
    'jamming' => $result['jamming'],
    'len' => $result['len'],
    'reliability' => $result['reliability'],
    'sent' => $result['sent'],
    'received' => $result['received'],
    'correct' => $result['correct'],
    'missed' => $result['missed'],
    'superfluous' => $result['superfluous'],
    'causality' => $result['causality'],
    'lat_combined' => $result['lat_combined'],
    'lat_mean' => $result['lat_mean'],
    'lat_median' => $result['lat_median'],
    'lat_90' => $result['lat_90'],
    'lat_95' => $result['lat_95'],
    'lat_99' => $result['lat_99'],
    'energy_total' => $result['energy_total'],
    'energy_setup' => $result['energy_setup'],
    'description' => $result['description'],
);

/*---------------------------------------------------------------------------*/
# Convert to CSV
echo "Write results to CSV... APPEND=" . $argv[3] ." WRITE_HEADERS=" . $argv[4] . PHP_EOL;
$csv_plotting = print_r(str_putcsv($result_plotting, $argv[4]), true);
$csv_readable = print_r(str_putcsv($result_readable, $argv[4]), true);
# Write to file
if ($argv[3]) {
  file_put_contents('dcube_results_plotting.csv', $csv_plotting, FILE_APPEND);
  file_put_contents('dcube_results_readable.csv', $csv_readable, FILE_APPEND);
} else {
  file_put_contents('dcube_results_plotting.csv', $csv_plotting);
  file_put_contents('dcube_results_readable.csv', $csv_readable);
}

# TODO: Simple headers
// fputcsv($output, array('id','name','description'));

# TODO: Only for "/queue/"
// echo "Job ID: " . $response['id'] . PHP_EOL;
