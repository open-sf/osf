<?php
$path = "~/logs/";
$files = array_diff(scandir($path), array('..', '.'));
foreach($files as $file) {
        $lines = file_get_contents($path . $file);
        preg_match('/Node ID\: ([0-9]+)/', $lines, $matches);
        $nodeid = $matches[1];
        if($nodeid == 0) {
          $nodeid = substr($file, strpos($file, '_') + 1,
                           strpos($file, '.') - strpos($file, '_') - 1);
        }
        preg_match('/Link-layer address\: (.+)/', $lines, $matches);
        $lladdr = $matches[1];

        echo "{ {$nodeid}, {{";
        $parts = explode(".", $lladdr);
        $i = 0;
        foreach($parts as $part) {
                $p0 = substr($part, 0, 2);
                $p1 = substr($part, 2, 2);
                echo "0x" . $p0 . ", 0x" . $p1;
                if($i != 3) echo ", "; else echo "}} }," . PHP_EOL;
                $i++;
        }
        // echo $nodeid . " " . $lladdr . PHP_EOL;
}
