#pragma once
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"

#include <iterator>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
using namespace cv;
const int maxH = 180;
const int maxValue = 255;
const cv::String windowCaptureName = "Video Capture";
const cv::String windowTreshold = "Object Detection";
static int lowH = 0, lowS = 0, lowV = 220;
static int highH = maxH, highS = maxValue, highV = maxValue;
const long double PI = 3.14159265358979323846;

struct Line
{
    vector<Point> line;
    Scalar color;
};

enum colors
{
    green, red, blue
};

static Scalar Red(255, 0, 0);
static Scalar Green(0, 255, 0);
static Scalar Blue(0, 0, 255);
static Scalar White(255, 255, 255);

class Contour;
class Hull;
class FingerDetect;

void trackbarBuild();



class Shot
{
private:
    Mat frame;
    cv::String name;
public:
    Shot() { }
    Shot(VideoCapture& cap, const cv::String& winName)
    {
        cap.read(frame);
        name = winName;
    }
    Mat& getFrame()
    {
        return frame;
    }
    void convertToHsv(Shot& HsvOutput)
    {
        cvtColor(this->frame, HsvOutput.frame, COLOR_BGR2HSV);
    }
    void detectObject()
    {
        inRange(this->frame, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), this->frame);
        erode(this->frame, this->frame, MORPH_ELLIPSE);
        dilate(this->frame, this->frame, MORPH_ELLIPSE);
        GaussianBlur(this->frame, this->frame, Size(19, 19), 0.0, 0);
    }
    void operator=(Shot& shot)
    {
        shot.frame = this->frame;
    }
    bool empty()
    {
        return this->frame.empty();
    }
    void setName(const cv::String& winName)
    {
        this->name = winName;
    }
    void SetFrame(VideoCapture& cap)
    {
        cap >> this->frame;
    }
    void flip()
    {
        cv::flip(this->frame, this->frame, 1);
    }
    void show()
    {
        imshow(this->name, this->frame);
    }
};

class Contour
{
private:
    vector<Point> lines;
    vector<Vec4i> defects;
    double area = 0;
public:
    Contour() {};
    virtual void find(Shot& shot)
    {
        vector<Vec4i> contoursInfo;
        vector<vector<Point>> contours;
        findContours(shot.getFrame(), contours, contoursInfo, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        area = 0.0;
        double largestCountourSize = area;
        int largestCountourIndex = 0;
        for (size_t i = 0; i < contours.size();i++)
        {
            area = contourArea(contours[i]);
            if (largestCountourSize < area)
            {
                largestCountourSize = area;
                largestCountourIndex = i;
            }
        }
        if (largestCountourIndex != 0)
            this->lines = contours[largestCountourIndex];
    }
    virtual void draw(Shot& shot)
    {
        polylines(shot.getFrame(), this->lines, true, Scalar(0, 0, 255));
    }
    virtual void clear()
    {
        this->lines.clear();
    }
    size_t size()
    {
        return this->lines.size();
    }
    vector<Point> getLines()
    {
        return this->lines;
    }
    vector<Vec4i> getDefects()
    {
        return this->defects;
    }
    friend Hull;
    friend FingerDetect;
};

class Hull : public Contour
{
private:
    vector<int> hullInts;
    Rect boundRect;
public:
    virtual void find(Contour& contour)
    {
        if (contour.size() <= 0)
            return;
        convexHull(Mat(contour.getLines()), this->lines, true);
        convexHull(Mat(contour.getLines()), hullInts, false);
        boundRect = boundingRect(this->lines);
        if (hullInts.size() > 3)
            convexityDefects(Mat(contour.lines), hullInts, this->defects);
        else
            return;

    }
    virtual void draw(Shot& shot)
    {
        if (this->lines.size() <= 0)
            return;
        polylines(shot.getFrame(), lines, true, Red);
        rectangle(shot.getFrame(), boundRect, Scalar(255, 255, 0), 2, 8, 0);
    }
    virtual void clear()
    {
        lines.clear();
        hullInts.clear();
    }
    friend FingerDetect;
};

class FingerDetect
{
private:
    size_t fingers = 0;
    size_t previousFingersNum = 0;
    Point position;
    size_t countOfLines = 1;
    vector<Line> lines;
    Scalar currentColor = Green;
    Scalar prevColor;
    size_t enumColor;

public:
    friend Hull;
    FingerDetect()
    {
        lines.resize(1);
        lines[0].color = Green;
    }
    void findTheFingers(Hull& hull, Contour& contour, Shot& shot)
    {
        if (hull.size() == 0 || contour.size() == 0 || hull.defects.size() == 0)
            return;
        vector<Vec4i>::iterator i;
        i = hull.defects.begin();
        Point tmpPos;
        float depth;
        while (i != hull.defects.end())
        {
            Vec4i& v = (*i);
            Point startPoint(contour.lines[v[0]]);
            Point endPoint(contour.lines[v[1]]);
            Point farPoint(contour.lines[v[2]]);
            float a = sqrt(pow((endPoint.x - startPoint.x), 2) + pow((endPoint.y - startPoint.y), 2));
            float b = sqrt(pow((farPoint.x - startPoint.x), 2) + pow((farPoint.y - startPoint.y), 2));
            float c = sqrt(pow((endPoint.x - farPoint.x), 2) + pow((endPoint.y - farPoint.y), 2));
            depth = static_cast<float>(v[3] / 256);
            double angle = acos((pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c));
            if (angle <= PI / 2)
            {
                line(shot.getFrame(), startPoint, farPoint, White, 2);
                line(shot.getFrame(), endPoint, farPoint, White, 2);
                circle(shot.getFrame(), startPoint, 8, Scalar(100, 0, 255), 2);
                tmpPos = startPoint;
                fingers++;
            }
            if (fingers == 0 && i != hull.defects.begin())
                if (depth > 20 && depth < 160)
                {
                    line(shot.getFrame(), startPoint, farPoint, White, 2);
                    line(shot.getFrame(), endPoint, farPoint, White, 2);
                    circle(shot.getFrame(), startPoint, 8, Scalar(100, 0, 255), 2);
                    tmpPos = startPoint;
                    fingers++;
                    lines[countOfLines - 1].line.push_back(tmpPos);
                }
            i++;
        }
    }
    void whatColor()
    {
        if (fingers == 2 && previousFingersNum != 2)
        {
            enumColor++;
            enumColor %= 3;
            if (enumColor == green)
                currentColor = Green;
            else if (enumColor == red)
                currentColor = Red;
            else if (enumColor == blue)
                currentColor = Blue;
        }
        else
            currentColor = prevColor;
    }
    void drawing(Shot& shot)
    {
        if (fingers == 1 && previousFingersNum != 1)
        {
            lines.push_back({ vector<Point>(),this->currentColor });
            countOfLines++;
        }

        for (size_t j = 0; j < lines.size();j++)
        {
            if (lines[j].line.size() != 0)
                for (size_t i = 0; i < lines[j].line.size() - 1;i++)
                    line(shot.getFrame(), lines[j].line[i], lines[j].line[i + 1], lines[j].color, 4, LINE_AA);
        }
    }
    size_t getFingersCount()
    {
        return fingers;
    }
    Point getPos()
    {
        return position;
    }
    size_t getPastFingersCount()
    {
        return previousFingersNum;
    }
    void zeroing()
    {
        if (fingers == 3)
        {
            for (int i = 0; i < lines.size(); i++)
                lines[i].line.clear();
            lines.clear();
            lines.resize(1);
            countOfLines = 1;
        }
        previousFingersNum = fingers;
        prevColor = currentColor;
        fingers = 0;
    }
};