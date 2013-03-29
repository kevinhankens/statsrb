<?php

$limit = 50000;

$ns_list = array(
  'kevin',
  'melissa',
  'benjamin',
  'stinker',
  'pete',
);

for ($i = 0; $i < $limit; $i++) {
  $ts = time();
  $ns_i = rand(0, (count($ns_list) - 1));
  $ns = $ns_list[$ns_i];
  $v = rand(0,100);
  file_get_contents("http://localhost:9292/PUT/$ts/$ns/$v");
}

echo $i;
