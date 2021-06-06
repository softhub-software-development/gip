
//
//  gip.cpp
//
//  Created by Christian Lehner on 02.03.20.
//  Copyright © 2020 Christian Lehner. All rights reserved.
//

#include "gip.h"
#include <geography/geo_geography.h>
#include <geography/geo_module.h>
#include <geography/geo_config.h>
#include <geography/geo_report.h>
#include <util/util.h>
#include <signal.h>

using namespace SOFTHUB;
using namespace SOFTHUB::BASE;
using namespace SOFTHUB::GEOGRAPHY;
using namespace SOFTHUB::GIP;
using namespace SOFTHUB::HAL;
using namespace SOFTHUB::NET;
using namespace SOFTHUB::UTIL;
using namespace std;

static void signal_handler(int signum);
static void report_ip(int argc, char** argv, Geo_config* config);
static void usage();

int main(int argc, char** argv)
{
    signal(SIGHUP, signal_handler);
    Geo_config_ref config(new Geo_config());
    Geo_config::define_app_data_dir("gip");
    const string& report_options = Geo_report_factory::all_options;
    const string& cmd_options = report_options + "h";
    config->read_parameters();
    Base_module::setup(argc, argv, cmd_options.c_str(), config);
    Gip_module::init_logging();
    Geo_module::module.init();
    Geo_module::module.instance->configure(config);
    if (argc >= 2) {
        if (config->get_bool_parameter("h")) {
            usage();
        } else {
            report_ip(argc, argv, config);
        }
    } else {
        int port = config->get_parameter("geo-ip-port", GEO_IP_SERVER_PORT);
        cout << "geo location server run on port " << port << endl;
        Geo_module::module.instance->run_service();
        Geo_module::module.dispose();
    }
    // Geo_module::module.dispose(); // TODO
    return 0;
}

static void report_ip(int argc, char** argv, Geo_config* config)
{
    Geo_report_factory factory;
    Geo_report_ref report = factory.create_report(config);
    bool reported = false;
    for (int i = 1; i < argc; i++) {
        const string& arg = argv[i];
        if (arg[0] != '-') {
            report->report_ip(arg);
            reported = true;
        }
    }
    if (!reported)
        usage();
}

void log_message(Log_level level, const string& msg)
{
    switch (level) {
    case ERR:
    default:
        std::cout << msg << std::endl;
        break;
    }
}

static void usage()
{
    cout << "usage: gip [-C] [-c] [-g] [-d] [-h] [-tT] [ip-address-or-domain ...]" << endl;
    cout << "       gip invoked with no arguments starts the gip server" << endl;
    cout << "       -C print country" << endl;
    cout << "       -c print city" << endl;
    cout << "       -g print geo coordinates only" << endl;
    cout << "       -d print domain name if possible" << endl;
    cout << "       -h print this help information" << endl;
    cout << "       -t trace route omitting duplicates" << endl;
    cout << "       -T trace route" << endl;
}

static void signal_handler(int signum)
{
    switch (signum) {
    case SIGHUP:
        cout << "SIGHUP" << endl;
        Geo_module::module.instance->terminate_service();
        break;
    case SIGINT:    // currently not caught
        cout << "SIGINT" << endl;
        break;
    case SIGPIPE:
        cout << "SIGPIPE" << endl;
        break;
    }
}
