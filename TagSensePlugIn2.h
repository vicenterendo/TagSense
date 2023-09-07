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

    //void OnFunctionCall(int FunctionId, const char* ItemString, POINT Pt, RECT Area);
    void multithread(void (CTagSensePlugIn::* f)());

    void SendFP(CFlightPlan FlightPlan);

    void IterateFPs();

    void SendFPs(vector<CFlightPlan> fps_total);

    bool OnCompileCommand(const char* sCommandLine);

    void loadConfig();
};