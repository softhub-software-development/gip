<html>
  <head>
    <title>softhub geo-ip</title>
    <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
    <meta name="viewport" content="width=device-width, initial-scale=0.7">
    <meta charset charset=UTF-8>
  </head>
  <body>
    <?php echo file_get_contents("common.html"); ?>
    <h1><font color='blue'>&nbsp;softhub geo-ip</font></h1>
    <form name="geo_ip" method="get" action="geo-ip.php">
      <table class='w3-table w3-large'>
        <tr>
          <td>
            <label for="ip">IP: <input id="ip" name="ip" placeholder="Number or Domain" autofocus></label>
            <input type="submit">
          </td>
        </tr>
      </table>
    </form>
  </body>
</html>

