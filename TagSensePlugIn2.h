#pragma once

#include "EuroScopePlugIn.h"
#include <string>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

#define PLUGIN_NAME "TagSense"

using namespace std;
using namespace EuroScopePlugIn;

static std::vector<std::vector<CFlightPlan>> SplitVector(const std::vector<CFlightPlan>& inputVector, size_t splitLength) {
    std::vector<std::vector<CFlightPlan>> result;
    size_t startIndex = 0;

    while (startIndex < inputVector.size()) {
        size_t endIndex = startIndex + splitLength;
        if (endIndex > inputVector.size()) {
            endIndex = inputVector.size();
        }

        std::vector<CFlightPlan> subVector(inputVector.begin() + startIndex, inputVector.begin() + endIndex);
        result.push_back(subVector);

        startIndex = endIndex;
    }

    return result;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

class CTagSensePlugIn :
    public EuroScopePlugIn :: CPlugIn
{

public:

    CTagSensePlugIn ( void ) ;

    virtual ~CTagSensePlugIn ( void ) ;

    void sendMessage(string message);

    void OnTimer(int Counter);

    void OnGetTagItem (CFlightPlan FlightPlan,
        CRadarTarget RadarTarget,
        int ItemCode,
        int TagData,
        char sItemString[16],
        int* pColorCode,
        COLORREF* pRGB,
        double* pFontSize);

    //void OnFunctionCall(int FunctionId, const char* ItemString, POINT Pt, RECT Area);
    void multithread(void (CTagSensePlugIn::* f)());

    void SendFP(CFlightPlan FlightPlan) {
        CURL* curl;
        curl = curl_easy_init();
        CURLcode res;
        std::string readBuffer;
        const CFlightPlanControllerAssignedData assigned_data = FlightPlan.GetControllerAssignedData();
        const CFlightPlanData data = FlightPlan.GetFlightPlanData();
        const char* TSAT_TAG = assigned_data.GetFlightStripAnnotation(3);
        const char* callsign = FlightPlan.GetCallsign();
        const char* destination = data.GetDestination();
        const char* SID = data.GetSidName();
        const char* squawk = assigned_data.GetSquawk();
        const char* status = FlightPlan.GetGroundState();
        const char* STAR = data.GetStarName();
        string info = "";
        info = info + "?callsign=" + callsign;
        if (strlen(TSAT_TAG) > 4) {
            string TSAT(TSAT_TAG);
            TSAT = TSAT.substr(2, 4);
            info = info + "&tsat=" + TSAT;
        }
        info = info + "&destination=" + destination;
        info = info + "&squawk=" + squawk;
        info = info + "&sid=" + SID;
        info = info + "&star=" + STAR;
        info = info + "&status=" + status;
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, ("http://localhost:8080/" + info).c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
    }

    void IterateFPs();

    void SendFPs(vector<CFlightPlan> fps_total);

    void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);

    void OnRefresh(HDC hDC, int Phase);
};


/*void ACDMPlugin::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
    EuroScopePlugIn::CRadarTarget RadarTarget,
    int ItemCode,
    int TagData,
    char sItemString[16],
    int* pColorCode,
    COLORREF* pRGB,
    double* pFontSize)
{

    if (!FlightPlan.IsValid()) {
        return;
    }

    std::string adep = FlightPlan.GetFlightPlanData().GetOrigin();
    to_upper(adep);

    if (adep.compare("LPPT") == 0) {

        CheckEtd(FlightPlan);

    }


}*/