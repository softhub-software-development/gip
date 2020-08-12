<?php
  $uri = $_SERVER['REQUEST_URI'];
  $query = parse_url($uri, PHP_URL_QUERY);
  $ip = $_SERVER['REMOTE_ADDR'];
  $cmd = $_REQUEST['cmd'];
  if (empty($cmd))
    $cmd = 'location';
  $internal = 'mw';
  $options = '&a=' . $ip;
  $engine = 'http://' . $internal . ':10101';
  $request = $engine . '?cmd=' . $cmd . '&' . $query . $options;
  echo file_get_contents($request);
?>

