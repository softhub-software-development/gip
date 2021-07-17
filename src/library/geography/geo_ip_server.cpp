
//
//  geo_ip_server.cpp
//
//  Created by Christian Lehner on 12/29/19.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "geo_ip_server.h"
#include "geo_module.h"
#include <iomanip>

using namespace SOFTHUB::BASE;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

namespace SOFTHUB {
namespace GEOGRAPHY {

//
// class Geo_ip_server
//

Geo_ip_server::Geo_ip_server() :
    database(new Geo_ip_file_database()),
    access_log_listener(new Geo_access_log_listener(this))
{
}

void Geo_ip_server::configure(IConfig* config)
{
    this->config = config;
    database->configure(config);
    Http_server::configure(config);
    this->set_user_agent("Sofhub-Geo-IP/1.0.0");
}

bool Geo_ip_server::initialize()
{
    int port = config->get_parameter("geo-ip-port", GEO_IP_SERVER_PORT);
    Address* address = Address::create(port);
    this->set_server_address(address);
    if (!Http_server::initialize())
        return false;
    service_control_event();
    Hal_module::module.instance->run(access_log_listener);
    return true;
}

void Geo_ip_server::finalize()
{
    access_log_listener->stop();
    Http_server::finalize();
}

void Geo_ip_server::service_control_event()
{
}

void Geo_ip_server::serve_page(const Http_service_request* sreq, Http_service_response* sres)
{
    const Http_request_header* header = sreq->get_header();
    const string& version = header->get_version();
    const string& method_name = header->get_method_name();
    const string& path = header->get_path();
    const Url_parameter_map& parameter_map = sreq->get_parameter_map();
    const string& cmd = parameter_map.get("cmd");
    const string& cmd_str = cmd.empty() ? "<empty>" : cmd;
    clog << "serve page " << cmd_str << " " << version << " " << method_name << " " << path << endl;
    try {
        if (cmd == "") {
            serve_default_page(sreq, sres);
        } else if (cmd == "echo") {
            serve_echo(sreq, sres);
        } else if (cmd == "location") {
            serve_location(sreq, sres);
        } else if (cmd == "traffic") {
            serve_traffic(sreq, sres);
        } else if (cmd == "data") {
            serve_access_data(sreq, sres);
        } else {
            serve_error_page("invalid command", sres);
        }
    } catch (Exception& ex) {
        serve_error_page(ex.get_message(), sres);
    } catch (...) {
        serve_error_page("fatal error", sres);
    }
}

void Geo_ip_server::serve_default_page_content(ostream& stream)
{
    stream << "<!doctype html>" << endl;
    stream << "<html><head>" << endl;
    stream << "  <meta charset='UTF-8'>" << endl;
    stream << "</head><body>" << endl;
    stream << "  Waiting for Geo IP requests." << endl;
    stream << "</body></html>" << endl;
}

void Geo_ip_server::serve_default_page(const Http_service_request* sreq, Http_service_response* sres)
{
    stringstream content_stream;
    serve_default_page_content(content_stream);
    const string& content = content_stream.str();
    serve_content(content, "text/html", sres);
}

void Geo_ip_server::serve_echo(const Http_service_request* sreq, Http_service_response* sres)
{
    const Url_parameter_map& parameter_map = sreq->get_parameter_map();
    const string& message = parameter_map.get("message");
    clog << "echo request for \"" << message << "\"" << endl;
    serve_content(message, "text/html", sres);
}

void Geo_ip_server::serve_location(const Http_service_request* sreq, Http_service_response* sres)
{
    const Url_parameter_map& parameter_map = sreq->get_parameter_map();
    const string& ip = parameter_map.get("ip");
    Address_ref addr = Address::create_from_dns_name(ip, 0);
    clog << "location request for " << ip << " " << (addr ? addr->to_string(false) : "-") << endl;
    Geo_ip_entry_ref entry = addr ? database->find_in_filesystem(addr) : unknown_ip_entry;
    stringstream content_stream;
    content_stream << "<head>" << endl;
    output_header(content_stream);
    content_stream << "</head>" << endl;
    content_stream << "<body>" << endl;
#if !(defined _DEBUG && defined PLATFORM_MAC)
    const string& doc_root = get_document_root();
    const string& inc = File_path::concat(doc_root, "gip/common.html");
    const string& common = Configuration::read_file(inc);
    content_stream << common;
#endif
    output_location(entry, sreq, content_stream);
    content_stream << "</body>" << endl;
    const string& content = content_stream.str();
    serve_content(content, "text/html", sres);
    const string& location = entry->to_string();
    const string& user = sreq->get_user_ip();
    cacc << user << " \"" << ip << "\" \"" << (location.empty() ? "-" : location) << "\"" << endl;
}

void Geo_ip_server::serve_traffic(const Http_service_request* sreq, Http_service_response* sres)
{
    File_observer* observer = access_log_listener->get_observer();
    observer->refresh();
    const Url_parameter_map& parameter_map = sreq->get_parameter_map();
    const string& zoom_param = parameter_map.get("zoom");
    const Geo_coordinates& coordinates = Geo_coordinates::parse("0,0");
    const Geo_latitude& latitude = coordinates.get_latitude();
    const Geo_longitude& longitude = coordinates.get_longitude();
    stringstream content_stream;
    content_stream << "<head>" << endl;
    output_header(content_stream);
    content_stream << "</head>" << endl;
    content_stream << "<body>" << endl;
#if defined _DEBUG && defined PLATFORM_MAC
    const string& lat = latitude.to_string(decimal);
    const string& lon = longitude.to_string(decimal);
    content_stream << lon << ", " << lat << " zoom: " << zoom_param << endl;
#else
    const string& doc_root = get_document_root();
    const string& inc = File_path::concat(doc_root, "gip/common.html");
    const string& common = Configuration::read_file(inc);
    content_stream << common;
    float zoom = atof(zoom_param.c_str());
    output_script(content_stream, zoom ? zoom : 2.25);
#endif
    content_stream << "</body>" << endl;
    const string& content = content_stream.str();
    serve_content(content, "text/html", sres);
}

void Geo_ip_server::serve_access_data(const Http_service_request* sreq, Http_service_response* sres)
{
    stringstream stream;
    stream << "{ \"data\": [";
    const Geo_locations& locations = access_log_listener->get_locations();
    Geo_locations::const_iterator it = locations.begin();
    Geo_locations::const_iterator tail = locations.end();
    if (it != tail) {
        const Geo_locations::value_type& pair = *it++;
        output_access_data_element(pair, stream);
        while (it != tail) {
            const Geo_locations::value_type& pair = *it++;
            stream << ",";
            output_access_data_element(pair, stream);
        }
    }
    stream << "]}" << endl;
    const string& content = stream.str();
    serve_content(content, "application/json", sres);
}

void Geo_ip_server::serve_content(const string& content, const string& content_type, Http_service_response* sres)
{
    stringstream stream;
    serve_header("200 OK", content_type, content.length(), stream);
    stream << endl;
    stream << content;
    sres->set_content(stream.str());
}

void Geo_ip_server::serve_error_page(const string& msg, Http_service_response* sres)
{
    Lock::Block lock(mutex);
    Http_server::serve_error_page(msg, sres);
    clog << msg << endl;
}

void Geo_ip_server::output_header(ostream& stream)
{
    stream << "  <title>softhub geo-ip</title>" << endl;
    stream << "  <link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>" << endl;
    stream << "  <meta name='viewport' content='width=device-width, initial-scale=0.5'>" << endl;
    stream << "  <meta charset charset=UTF-8>" << endl;
}

void Geo_ip_server::output_script(ostream& stream, float zoom)
{
    const string& lib = "https://cdnjs.cloudflare.com/ajax/libs/openlayers/2.11/lib/OpenLayers.js";
    float clon = 0, clat = 0;
    int refresh_in_seconds = config->get_parameter("geo-map-refresh", 60);
    stream << "  <div id='mapdiv'></div>" << endl;
    stream << "  <script src='" << lib << "'></script>" << endl;
    stream << "  <script>" << endl << endl;
    stream << "    var map = new OpenLayers.Map('mapdiv');" << endl;
    stream << "    map.addLayer(new OpenLayers.Layer.OSM());" << endl;
    stream << "    var epsg4326 = new OpenLayers.Projection('EPSG:4326');" << endl;
    stream << "    var projectTo = map.getProjectionObject();" << endl;
    stream << "    var lonLat = new OpenLayers.LonLat(" << clon << ", " << clat << ").transform(epsg4326, projectTo);" << endl;
    stream << "    map.setCenter(lonLat, " << zoom << ");" << endl;    // TODO: map.getView().getZoom();
    stream << "    var vectorLayer = new OpenLayers.Layer.Vector('Overlay');" << endl;
    stream << "    map.addLayer(vectorLayer);" << endl;
    stream << "    var controls = {selector: new OpenLayers.Control.SelectFeature(vectorLayer, {onSelect: createPopup, onUnselect: destroyPopup})};" << endl;
    stream << "    map.addControl(controls['selector']);" << endl;
    stream << "    controls['selector'].activate();" << endl;
    stream << "    createAnnotations();" << endl;
    stream << "    setInterval(createAnnotations, " << refresh_in_seconds * 1000 << ");" << endl;
    stream << endl;
    stream << "    function createAnnotations() {" << endl;
    stream << "      var xhttp = new XMLHttpRequest();" << endl;
    stream << "      xhttp.addEventListener(\"readystatechange\", function() {" << endl;
    stream << "        if (this.readyState === 4) {" << endl;
    stream << "          if (xhttp.status === 200) {" << endl;
    stream << "            createAnnotationsAsync(xhttp.responseText);" << endl;
    stream << "          } else {" << endl;
    stream << "            console.error(xhttp.statusText);" << endl;
    stream << "          }" << endl;
    stream << "        }" << endl;
    stream << "      });" << endl;
    stream << "      xhttp.open('GET', '/gip/geo-ip.php?cmd=data', true);" << endl;
    stream << "      xhttp.send();" << endl;
    stream << "    }" << endl;
    stream << endl;
    stream << "    function initAnnotations() {" << endl;
    stream << "      vectorLayer.destroyFeatures();" << endl;
    stream << "      while (map.popups.length) {" << endl;
    stream << "        map.removePopup(map.popups[0]);" << endl;
    stream << "      }" << endl;
    stream << "    }" << endl;
    stream << endl;
    stream << "    function createAnnotationsAsync(responseText) {" << endl;
    stream << "      var anno = JSON.parse(responseText);" << endl;
    stream << "      var data = anno.data;" << endl;
    stream << "      initAnnotations();" << endl;
    stream << "      for (var i = 0; i < data.length; i++) {" << endl;
    stream << "        var obj = data[i];" << endl;
    stream << "        var desc = obj.ip.concat(' ').concat(obj.desc);" << endl;
    stream << "        var img = 'img/' + obj.img;" << endl;
    stream << "        createAnnotation(obj.lon, obj.lat, desc, img);" << endl;
    stream << "      }" << endl;
    stream << "    }" << endl;
    stream << endl;
    stream << "    function createAnnotation(lon, lat, desc, img) {" << endl;
    stream << "      var pt = new OpenLayers.Geometry.Point(lon, lat).transform(epsg4326, projectTo)" << endl;
    stream << "      var desc = {description: desc}" << endl;
    stream << "      var exgr = {externalGraphic: img, graphicHeight: 25, graphicWidth: 21, graphicXOffset: -12, graphicYOffset: -25}" << endl;
    stream << "      var feature = new OpenLayers.Feature.Vector(pt, desc, exgr);" << endl;
    stream << "      vectorLayer.addFeatures(feature);" << endl;
    stream << "    }" << endl;
    stream << endl;
    stream << "    function createPopup(feature) {" << endl;
    stream << "      var center = feature.geometry.getBounds().getCenterLonLat();" << endl;
    stream << "      var content = '<div class=\\'markerContent\\'>' + feature.attributes.description + '</div>';" << endl;
    stream << "      var unselector = function() {controls['selector'].unselectAll();};" << endl;
    stream << "      feature.popup = new OpenLayers.Popup.FramedCloud('pop', center, null, content, null, true, unselector);" << endl;
    stream << "      map.addPopup(feature.popup);" << endl;
    stream << "    }" << endl;
    stream << endl;
    stream << "    function destroyPopup(feature) {" << endl;
    stream << "      feature.popup.destroy();" << endl;
    stream << "      feature.popup = null;" << endl;
    stream << "    }" << endl << endl;
    stream << "  </script>" << endl;
    clog << "output_script: " << clon << ", " << clat << " zoom: " << zoom << endl;
}

void Geo_ip_server::output_access_data_element(const Geo_locations::value_type& pair, ostream& stream)
{
    const string& ip = pair.first->to_string(false);
    const Geo_log_data* data = pair.second;
    const Geo_ip_entry* entry = data->get_ip_entry();
    const Geo_coordinates& coords = entry->get_coordinates();
    float lon = coords.get_longitude().to_degrees<float>();
    float lat = coords.get_latitude().to_degrees<float>();
    int accesses = data->get_accesses();
    const string& desc = String_util::escape(entry->get_city(), '\'');
    const string& img = data->get_img();
    clog << "output_access_data_element: " << img << " for " << ip << endl;
    stream << "{" << endl;
    stream << "  \"ip\": \"" << ip << "\"," << endl;
    stream << "  \"lon\": " << lon << "," << endl;
    stream << "  \"lat\": " << lat << "," << endl;
    stream << "  \"desc\": \"" << desc << " (" << accesses << ")\"," << endl;
    stream << "  \"img\": \"" << img << "\"" << endl;
    stream << "}";
}

void Geo_ip_server::output_location(const Geo_ip_entry* entry, const Http_service_request* sreq, ostream& stream)
{
    const Geo_coordinates& coordinates = entry->get_coordinates();
    const Geo_latitude& latitude = coordinates.get_latitude();
    const Geo_longitude& longitude = coordinates.get_longitude();
    const string& lat = latitude.to_string(decimal);
    const string& lon = longitude.to_string(decimal);
#if defined _DEBUG && defined PLATFORM_MAC
    stream << lon << " " << lat << endl;
#else
    Geo_detail detail(coordinates, 10);
    const Url_parameter_map& parameter_map = sreq->get_parameter_map();
    const string& ip = parameter_map.get("ip");
    const string& box = detail.to_string(decimal);
    const string& osm = config->get_parameter("geo-server-url", GEO_SERVER_URL);
    const string& location = entry->to_string();
    const string& msg = location.empty() ? "not found" : location;
    stream << "  <form name='geo_ip' method='get' action='geo-ip.php'>" << endl;
    stream << "    <input type='hidden' id='cmd' name='cmd' value='location'>" << endl;
    stream << "    <label for='ip'>" << endl;
    stream << "      IP: <input id='ip' name='ip' value='" << ip << "' autofocus>" << endl;
    stream << "    </label><input type='submit'> " << msg << endl;
    stream << "    <br/>";
    stream << "    <iframe width='100%' height='90%' frameborder='0' scrolling='no' marginheight='0' marginwidth='0'" << endl;
    stream << "      src='" << osm << "?bbox=" << box << "&marker=" << lat << "," << lon << "&layer=mapnik'" << endl;
    stream << "      style='border: 1px solid black'>";
    stream << "    </iframe>" << endl;
    stream << "    <br/><small>" << endl;
    stream << "      <a href='" << osm << "?mlat=" << lat << "&mlon=" << lon << "&zoom=10&layers=M'>View Larger Map</a>" << endl;
    stream << "    </small>" << endl;
    stream << "  </form>" << endl;
#endif
}

void Geo_ip_server::output_route(const Geo_ip_entry* entry, const Http_service_request* sreq, ostream& stream)
{
}

Geo_ip_entry_ref Geo_ip_server::unknown_ip_entry = new Geo_ip_entry();

//
// class Geo_log_data
//

Geo_log_data::Geo_log_data(const Address* address, const Geo_ip_entry* ip_entry) :
    address(address), ip_entry(ip_entry), accesses(0), robot(false), download(false)
{
}

void Geo_log_data::set_link(const std::string& link)
{
    this->link = link;
}

void Geo_log_data::set_referer(const std::string& referer)
{
    this->referer = referer;
}

void Geo_log_data::set_client(const std::string& client)
{
    this->client = client;
}

void Geo_log_data::check_link(const String_vector& sv)
{
    string tmp = link;
    Strings::to_lower(tmp);
    download |= Strings::match_string_ext(tmp, sv);
}

void Geo_log_data::check_client(const String_vector& sv)
{
    string tmp = client;
    Strings::to_lower(tmp);
    robot |= Strings::match_string(tmp, sv);
}

string Geo_log_data::get_img() const
{
    return robot ? "robot.png" : (download ? "download.png" : "client.png");
}

void Geo_log_data::classify(BASE::IConfig* config)
{
    String_vector downloads;
    const string& dstr = config->get_parameter("geo-downloads", ".bin .zip .dmg");
    clog << "geo-downloads: " << dstr << endl;
    String_util::split(dstr, downloads);
    check_link(downloads);
    String_vector bots;
    const string& bstr = config->get_parameter("geo-bots", "bot spider crawl grab");
    clog << "geo-bots: " << bstr << endl;
    String_util::split(bstr, bots);
    check_client(bots);
    increment_accesses();
}

//
// class Geo_log_listener
//

Geo_log_listener::Geo_log_listener(Geo_ip_server* server) :
    server(server), done(false)
{
}

void Geo_log_listener::run()
{
    IConfig* config = server->get_config();
    const string& log = config->get_parameter("geo-log", "/var/log/apache2/access.log");
    File_observer* observer = get_observer();
    observer->tail(log, true);
}

void Geo_log_listener::fail(const exception& ex)
{
}

void Geo_log_listener::stop()
{
    done = true;
}

void Geo_log_listener::add_location(const Address* addr, Geo_log_data* data)
{
    locations.insert(addr, data);
}

void Geo_log_listener::clear_locations()
{
    locations.clear();
}

Geo_log_data* Geo_log_listener::store(const Address* addr, Geo_ip_entry* entry)
{
    Geo_locations::const_iterator it = locations.find(addr);
    Geo_log_data_ref data;
    if (it == locations.end()) {
        data = new Geo_log_data(addr, entry);
        add_location(addr, data);
    } else {
        data = it->second;
    }
    return data;
}

//
// class Geo_access_log_listener
//

Geo_access_log_listener::Geo_access_log_listener(Geo_ip_server* server) :
    Geo_log_listener(server), observer(new File_observer(new Geo_access_log_consumer(this)))
{
}

void Geo_access_log_listener::run()
{
    IConfig* config = server->get_config();
    const string& log = config->get_parameter("geo-log", "/var/log/apache2/access.log");
    observer->tail(log, true);
}

//
// class Geo_log_consumer
//

Geo_log_consumer::Geo_log_consumer(Geo_log_listener* listener) : listener(listener)
{
}

void Geo_log_consumer::consumer_reset()
{
    const Geo_locations& locations = listener->get_locations();
    clog << "consumer reset: " << locations.size() << endl;
    listener->clear_locations();
}

//
// class Geo_access_log_consumer
//

Geo_access_log_consumer::Geo_access_log_consumer(Geo_log_listener* listener) : Geo_log_consumer(listener)
{
}

bool Geo_access_log_consumer::consumer_process(const String_vector& columns)
{
    size_t ncols = columns.size();
    if (ncols < 1)
        return false;
    const string& ip = columns[0];
    clog << "consumer process: " << ip << endl;
    Address_const_ref addr = Address::create_from_dns_name(ip, 0);
    if (!addr)
        return true;
    Geo_ip_server* server = listener->get_server();
    Geo_ip_file_database* database = server->get_ip_database();
    Geo_ip_entry_ref entry = database->find_in_filesystem(addr);
    if (!entry)
        return true;
    Geo_log_data_ref data = listener->store(addr, entry);
    if (ncols > 4)
        data->set_link(columns[4]);
    if (ncols > 7)
        data->set_referer(columns[7]);
    if (ncols > 8)
        data->set_client(columns[8]);
    Geo_ip_server* service = listener->get_server();
    IConfig* config = service->get_config();
    data->classify(config);
    return true;
}

}}
