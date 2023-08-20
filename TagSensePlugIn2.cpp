#include "StdAfx.h"
#include "TagSensePlugIn2.h"
#include "EuroScopePlugIn.h"
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include <cctype>

#define MY_PLUGIN_NAME      "TagSense"
#define MY_PLUGIN_VERSION   "0.0.1-a"
#define MY_PLUGIN_DEVELOPER "Vicente Rendo"
#define MY_PLUGIN_COPYRIGHT "GPL v3"
#define CONFIG_FILE "TagSenseConfig.txt"

using namespace std;
using namespace EuroScopePlugIn;

const   int     TAG_ITEM_HOLDING_NAME = 1;
const   int     TAG_ITEM_HOLDING_TIME = 2;
const   int     TAG_ITEM_REMAINING_TIME = 3;

const   int     TAG_FUNC_HOLDING_EDIT = 1;   // for the TAGs
const   int     TAG_FUNC_HOLDING_WAIT = 2;   // for the TAGs

const   int     TAG_FUNC_HOLDING_EDITOR = 10;  // when editing the point name
const   int     TAG_FUNC_HOLDING_WAIT_LIST = 11;  // for the popup list elements
const   int     TAG_FUNC_HOLDING_WAIT_CLEAR = 12;  // cnacel the wait by the popup

bool STATE = true;
string SERVER_ADDR = "127.0.0.1";
string ORIGIN_PREFIX = "";

static bool startsWith(const char* pre, const char* str) {
    const size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

static std::string toUpper(const std::string& input) {
    std::string upperCase;

    for (char c : input) {
        upperCase += std::toupper(c);
    }

    return upperCase;
}

static std::vector<std::string> splitString(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::string token;
    std::istringstream iss(input);

    while (std::getline(iss, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}

void CTagSensePlugIn::loadConfig() {
    std::ifstream inputFile(CONFIG_FILE);
    if (!inputFile) {
        std::ofstream outputFile(CONFIG_FILE);
        if (outputFile) {
            outputFile << format("SERVER {} // ADDRESS OF THE REMOTE TAGSENSE SERVER\nPREFIX {} // ORIGIN ICAO CODE FILTER (LP = LPxx, E = Exxx, ...)", SERVER_ADDR, ORIGIN_PREFIX) << std::endl;
            outputFile.close();
        }
    }
    try {
        fstream configFile;
        configFile.open(CONFIG_FILE, ios::in);
        if (configFile.is_open()) {
            string tp;
            while (getline(configFile, tp)) {
                try {
                    const vector<string> split = splitString(tp, ' ');
                    if (split.size() < 2) continue;
                    const string param = toUpper(split.at(0));
                    const string value = split.at(1);
                    if (param == "SERVER") {
                        SERVER_ADDR = value;
                    }
                    else if (param == "PREFIX") {
                        ORIGIN_PREFIX = value;
                    }
                } catch (const exception e) {}
                
            }
            configFile.close();
        }
        sendMessage("Config file loaded.");
    }
    catch (const std::exception e) {
        sendMessage("Failed to load config file.");
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    const size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

CTagSensePlugIn::CTagSensePlugIn()
  : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
        MY_PLUGIN_NAME,
        MY_PLUGIN_VERSION,
        MY_PLUGIN_DEVELOPER,
        MY_PLUGIN_COPYRIGHT)
{
    loadConfig();
}

void CTagSensePlugIn::sendMessage(string message) {
    DisplayUserMessage(PLUGIN_NAME, "", message.c_str(), true, true, true, false, false);
};

void CTagSensePlugIn::OnTimer(int Counter) {
    if (Counter % 10 == 0 && STATE) multithread(&CTagSensePlugIn::IterateFPs);
}

void CTagSensePlugIn::multithread(void (CTagSensePlugIn::* f)()) {
    try {
        thread* mythread = new thread(f, this);
        mythread->detach();
    }
    catch (std::exception e) {
        cout << "Failed to multi-thread function";
    }
}

void CTagSensePlugIn::IterateFPs()
{
    CFlightPlan fp = FlightPlanSelectFirst();
    if (!fp.IsValid()) return;
    const char* firstCallsign = fp.GetCallsign();
    std::vector<CFlightPlan> FPs;
    while (true) {
        if (!fp.IsValid()) break;
        if (string(fp.GetFlightPlanData().GetOrigin()).substr(0, ORIGIN_PREFIX.length()) == ORIGIN_PREFIX) FPs.push_back(fp);
        fp = FlightPlanSelectNext(fp);
        if (fp.GetCallsign() == firstCallsign) break;
    }
    SendFPs(FPs);
};

void CTagSensePlugIn::SendFPs(vector<CFlightPlan> fps_total) {
    const std::vector<std::vector<CFlightPlan>> fps_subs = SplitVector(fps_total, 100);
    const int fps_subs_size = fps_subs.size();
    for (int i = 0; i < fps_subs_size; i++) {
        std::vector<CFlightPlan> fps = fps_subs.at(i);
        CURL* curl;
        curl = curl_easy_init();
        CURLcode res;
        std::string readBuffer;
        string json = "[";
        const int fps_size = fps.size();
        for (int _i = 0; _i < fps_size; _i++) {
            const CFlightPlan fp = fps.at(_i);
            const CFlightPlanControllerAssignedData assigned_data = fp.GetControllerAssignedData();
            const CFlightPlanData data = fp.GetFlightPlanData();
            const CRadarTarget radar_target = fp.GetCorrelatedRadarTarget();
            const char* TSAT_TAG = assigned_data.GetFlightStripAnnotation(3);
            const char* callsign = fp.GetCallsign();
            const char* origin = data.GetOrigin();
            const double distance_to_origin = fp.GetDistanceFromOrigin();
            const char* destination = data.GetDestination();
            const double distance_to_destination = fp.GetDistanceToDestination();
            const char* SID = data.GetSidName();
            const char* squawk = assigned_data.GetSquawk();
            const char* status = fp.GetGroundState();
            const char* STAR = data.GetStarName();
            const int pressure_altitude = radar_target.GetPosition().GetPressureAltitude();
            const int flight_level = radar_target.GetPosition().GetFlightLevel();
            string _json = "{";
            _json = _json + "\"callsign\":\"" + callsign + "\",";
            if (strlen(TSAT_TAG) > 4) {
                string TSAT(TSAT_TAG);
                TSAT = TSAT.substr(2, 4);
                _json = _json + "\"tsat\":\"" + TSAT + "\",";
            }
            else _json = _json + "\"tsat\": \"\",";
            _json = _json + "\"origin\":\"" + origin + "\",";
            _json = _json + "\"destination\":\"" + destination + "\",";
            _json = _json + "\"squawk\":\"" + squawk + "\",";
            _json = _json + "\"sid\":\"" + SID + "\",";
            _json = _json + "\"star\":\"" + STAR + "\",";
            _json = _json + "\"status\":\"" + status + "\",";
            _json = _json + "\"distance_to_origin\":\"" + to_string(distance_to_origin) + "\",";
            _json = _json + "\"distance_to_destination\":\"" + to_string(distance_to_destination) + "\",";
            _json = _json + "\"flight_level\":\"" + to_string(flight_level) + "\",";
            _json = _json + "\"pressure_altitude\":\"" + to_string(pressure_altitude) + "\"";
            json += _json + "},";
        }

        json.pop_back();
        json += "]";

        if (curl) {
            try {
                struct curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, "Content-Type: application/json");
                curl_easy_setopt(curl, CURLOPT_URL, std::format("http://{}/tag", SERVER_ADDR).c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback, json);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json.length());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                //curl_easy_setopt(c, CURLOPT_CAINFO, "/path/to/the/certificate.crt");
                //curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);

            }
            catch (std::exception e) {
                DisplayUserMessage(PLUGIN_NAME, "", e.what(), true, true, true, false, false);
            }
        }

    }
}

bool CTagSensePlugIn::OnCompileCommand(const char* sCommandLine) {
    if (startsWith(".tagsense stop", sCommandLine)) {
        STATE = false;
        sendMessage("UPDATES - OFF");
        return true;
    }
    else if (startsWith(".tagsense start", sCommandLine)) {
        STATE = true;
        sendMessage("UPDATES - ON");
        return true;
    }
    else if (startsWith(".tagsense server", sCommandLine) && splitString(string(sCommandLine), ' ').size() == 3) {
        SERVER_ADDR = splitString(string(sCommandLine), ' ').at(2);
        sendMessage(format("SERVER ADDRESS - {}", SERVER_ADDR));
        return true;
    }
    else if (startsWith(".tagsense reload", sCommandLine)) {
        loadConfig();
        return true;
    }
}


CTagSensePlugIn :: ~CTagSensePlugIn(void)
{
}