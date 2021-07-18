# gip

Geo IP server and command line tool.

Gip is an Apache log file analyzer for Linux, in particular for small platforms like the Raspberry Pi.
When launched without command line arguments or using the lauch script gip.sh, gip runs as a server process.
Given an argument like "gip google.com" or a numeric IP address, it prints the country and city the address is registered in.

![gip-map](https://user-images.githubusercontent.com/61689577/126057090-4bec81f0-4dd8-4680-aabf-3b65fa3b3d55.png)

To compile the source code, follow these steps:

(1) install Flex and Bison<br/>
    apt-get -y install flex bison<br/>

(2) go to the build/Linux/gip directory and run "make"

(3) configure the ip dataset

Gip comes with a tiny ip dataset file with only a few IP ranges for testing. In order to use gip, download the
dataset from the link below and replace geo-ip.csv with the full database. Remove the directory named "geo-ip" if
it has already been generated and run the gip command again. If the "geo-ip" directory is not found, it is regenerated
from the "geo-ip.csv" dataset, which may take a few minutes. Be sure, the column headings of your dataset matches 
those of the small default dataset file.

By default, the dataset is located at build/Linux/gip/geo-ip. To change the location create /var/tmp/gip/default.conf
and add the line geo-ip=/mnt/b/dat or where ever your want to put the data directory. Put the dataset file
"geo-ip.csv" into this directory, run the gip command and after a minute or two (~10 minutes on a raspberry pi), you'll
find the "geo-ip" data directory next to the csv file.

IMPORTANT: The "geo-ip" directory holds a lot of sub directories and files, therefore the filesystem must be configured
with a high number of inodes. Make sure to check the log files at "/var/tmp/gip/log/*.log" for error messages. Issue
a df -i to check if the filesystem ran out of inodes.

To run the gip server, lauch gip without parameters or use the gip.sh script to lauch the server.
Use your browser to access "http://localhost:10101/gip".

See the supplied php scripts to tunnel through your firewall. TODO: refine this.

Download the geo ip dataset "IP2Location LITE IP-COUNTRY-REGION-CITY-LATITUDE-LONGITUDE-ZIPCODE-TIMEZONE"
from here: https://lite.ip2location.com/ip2location-lite see "Free Databases", DB11-LITE.

Here is a reference installation: https://softhub.com/gip/geo-ip.php?cmd=traffic

Example gip commands:

<h3>Print help:</h3>
<p># gip -h</p>
usage: gip [-C] [-c] [-g] [-h] [-tT] [ip-address-or-domain ...]<br/>
       gip invoked with no arguments starts the gip server<br/>
       -C print country<br/>
       -c print city<br/>
       -g print geo coordinates only<br/>
       -d print domain name if possible<br/>
       -h print this help information<br/>
       -t trace route omitting duplicates<br/>
       -T trace route<br/>
<br/>
<h3>Print the ip's geo location of 91.65.9.3:</h3>
<p># gip 91.65.9.3</p>
91.65.9.3 "Berlin, Berlin, Germany"
<br/>
<h3>Print only the country:</h3>
<p># gip -C softhub.com</p>
softhub.com, Germany
<br/>
<h3>Print the route:</h3>
<p># gip -t google.com</p>
91.65.9.254 Berlin, Berlin, Germany<br/>
145.254.3.66 Eschborn, Hessen, Germany<br/>
72.14.194.138 Mountain View, California, United States<br/>
<br/>
<h3>Print the geo coordinates:</h3>
<p># gip -g softhub.com</p>
52.5244 13.4105
<br/>
