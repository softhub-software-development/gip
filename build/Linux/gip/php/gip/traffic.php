<html>
  <head>
    <title>softhub traffic</title>
    <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
    <meta name="viewport" content="width=device-width, initial-scale=0.7">
    <meta charset charset=UTF-8>
    <meta http-equiv="refresh" content="10">
  </head>
  <body>
    <?php echo file_get_contents("common.html"); ?>
    <?php
      $OSM = "https://www.openstreetmap.org/export/embed.html";
      $BOX = "-1,-1,1,1";
      $LAT = 0;
      $LON = 0;
      $SRC = $OSM . "?bbox=" . $BOX . "&marker=" . $LAT . "," . $LON . "&layer=mapnik";
      $DST = $OSM . "?mlat=" . $LAT . "&mlon=" . $LON . "&zoom=10&layers=M";
    ?>
    <h1><font color='blue'>&nbsp;softhub access log</font></h1>
    <form name='geo_ip' method='get' action='geo-ip.php'>
      <iframe width='100%' height='90%' frameborder='0' scrolling='no' marginheight='0' marginwidth='0'
        src=<?php echo $SRC ?> style='border: 1px solid black'>
      </iframe>
      <br/>
      <small>
        <a href=<?php echo $DST ?>>View Larger Map</a>
      </small>
    </form>
  </body>
</html>

