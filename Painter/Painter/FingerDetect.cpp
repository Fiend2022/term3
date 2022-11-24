#include "FingerDetect.hpp"
#include "opencv2/highgui.hpp"



static void lowHThreshTrackbar(int, void*)
{
    lowH = min(highH - 1, lowH);
    setTrackbarPos("Low H", windowTreshold, lowH);
}

static void highHTreshTackbar(int, void*)
{
    highH = max(highH, lowH + 1);
    setTrackbarPos("High H", windowTreshold, highH);
}

static void lowSThreshTrackbar(int, void*)
{
    lowS = min(highS - 1, lowS);
    setTrackbarPos("Low S", windowTreshold, lowS);
}

static void highSThreshTrackbar(int, void*)
{
    highS = max(highS, lowS + 1);
    setTrackbarPos("High S", windowTreshold, highS);
}

static void lowVThreshTrackbar(int, void*)
{
    lowV = min(highV - 1, lowV);
    setTrackbarPos("Low V", windowTreshold, lowV);
}

static void highVThreshTrackbar(int, void*)
{
    highV = max(highV, lowV + 1);
    setTrackbarPos("High V", windowTreshold, highV);
}

void trackbarBuild()
{
    createTrackbar("Low H", windowTreshold, &lowH, maxH, lowHThreshTrackbar);
    createTrackbar("High H", windowTreshold, &highH, maxH, highHTreshTackbar);
    createTrackbar("Low S", windowTreshold, &lowS, maxValue, lowSThreshTrackbar);
    createTrackbar("High S", windowTreshold, &highS, maxValue, highSThreshTrackbar);
    createTrackbar("Low V", windowTreshold, &lowV, maxValue, lowVThreshTrackbar);
    createTrackbar("High V", windowTreshold, &highV, maxValue, highVThreshTrackbar);
}

