#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iterator>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;
const int maxH = 180;
const int maxValue = 255;
const String windowCaptureName = "Video Capture";
const String windowTreshold = "Object Detection";
int lowH = 0, lowS = 0, lowV = 220;
int highH = maxH, highS = maxValue, highV = maxValue;

class Contour;
class Hull;
class FingerDetect;

static void lowHThreshTrackbar(int, void*);
static void highHTreshTackbar(int, void*);
static void lowSThreshTrackbar(int, void*);
static void highSThreshTrackbar(int, void*);
static void lowVThreshTrackbar(int, void*);
static void highVThreshTrackbar(int, void*);
static void trackbarBuild();



class Shot
{
private:
    Mat frame;
    String name;
public:
    Shot() { }
    Shot(VideoCapture& cap, const String& winName)
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
    void detectObject(Scalar lowValues, Scalar highValues)
    {
        inRange(this->frame, lowValues, highValues, this->frame);
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
    void setName(const String& winName)
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
    friend Contour;
    friend Hull;
    friend FingerDetect;
};

class Contour
{
private:
    vector<Point> lines;
    vector<Vec4i> defects;
    double area = 0;
public:
    friend FingerDetect;
    friend Hull;
    Contour() {};
    virtual void find(Shot& shot)
    {
        vector<Vec4i> contoursInfo;
        vector<vector<Point>> contours;
        findContours(shot.frame, contours, contoursInfo, RETR_EXTERNAL, CHAIN_APPROX_NONE);
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
        if(largestCountourIndex != 0)
            this->lines = contours[largestCountourIndex];
    }
    virtual void draw(Shot& shot)
    {
        polylines(shot.frame, this->lines, true, Scalar(0, 0, 255));
    }
    virtual void clear()
    {
        this->lines.clear();
    }
    size_t size()
    {
        return this->lines.size();
    }
  
};

class Hull : private Contour
{
private:
    vector<int> hullInts;
    Rect boundRect;
public:
    virtual void find(Contour& contour)
    {
        if (contour.size() <= 0)
            return;
        convexHull(Mat(contour.lines), this->lines, true);
        convexHull(Mat(contour.lines), hullInts, false);
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
        polylines(shot.frame, lines, true, Scalar(255, 0, 0));
        rectangle(shot.frame, boundRect, Scalar(255, 255, 0), 2, 8, 0);
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
    size_t fingers=0;
    bool active = false;
    Point position;
public:
    friend Hull;
    void findTheFingers(Hull& hull,Contour& contour, Shot& shot)
    {
        if (hull.size() == 0 || contour.size() == 0 || hull.defects.size() == 0)
            return;

        vector<Vec4i>::iterator i;
        i = hull.defects.begin();
        Point tmpPos;
        while (i != hull.defects.end())
        {
            Vec4i& v = (*i);
            Point startPoint(contour.lines[v[0]]);
            Point endPoint(contour.lines[v[1]]);
            Point farPoint(contour.lines[v[2]]);
            float depth = static_cast<float>(v[3] / 256);
            if (depth > 20 && depth < 160)
            {
                line(shot.frame, startPoint, farPoint, CV_RGB(0, 255, 0), 2);
                line(shot.frame, endPoint, farPoint, CV_RGB(0, 255, 0), 2);
                circle(shot.frame, startPoint, 8, Scalar(100, 0, 255), 2);
                tmpPos = startPoint;
                fingers++;
            }
            i++;
        }
        if (fingers > 0)
        {
            active = true;
            if (fingers == 1)
                position = tmpPos;
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
    void zeroing()
    {
        fingers = 0;
        active = false;
    }
};

int main()
{
    VideoCapture cap(0);
    namedWindow(windowCaptureName);
    namedWindow(windowTreshold);
    trackbarBuild();
    Shot frame, frame_HSV, Hsv;
    frame.setName(windowCaptureName);
    Hsv.setName(windowTreshold);
    FingerDetect hand;
    vector<Point> paint;
    while (true)
    {
        frame.SetFrame(cap);
        if (frame.empty())
            break;
        frame.convertToHsv(Hsv);
        Hsv.detectObject(Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV));
        Contour contour;
        contour.find(Hsv);
        Hull hull;
        hull.find(contour);
        hand.findTheFingers(hull, contour,frame);
        /*hull.draw(frame);*/
       /* contour.draw(frame);*/
        if (hand.getFingersCount() == 1)
            paint.push_back(hand.getPos());
        else if (hand.getFingersCount() == 3)
            paint.clear();
        for (int i = 0; i < paint.size();i++)
            circle(frame.getFrame(), paint[i], 6, Scalar(255, 255, 255),-1);
        frame.flip();
        Hsv.flip();
        String text = "Fingers: ";
        text += to_string(hand.getFingersCount());
        putText(frame.getFrame(), text, Point(20, 20), 1, 1, Scalar(255, 255, 255));
        frame.show();
        Hsv.show();
        contour.clear();
        hull.clear();
        hand.zeroing();
        char key = (char)waitKey(30);
        if (key == 'q' || key == 27)
            break;
    }
    return 0;
}


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

static void trackbarBuild()
{
    createTrackbar("Low H", windowTreshold, &lowH, maxH, lowHThreshTrackbar);
    createTrackbar("High H", windowTreshold, &highH, maxH, highHTreshTackbar);
    createTrackbar("Low S", windowTreshold, &lowS, maxValue, lowSThreshTrackbar);
    createTrackbar("High S", windowTreshold, &highS, maxValue, highSThreshTrackbar);
    createTrackbar("Low V", windowTreshold, &lowV, maxValue, lowVThreshTrackbar);
    createTrackbar("High V", windowTreshold, &highV, maxValue, highVThreshTrackbar);
}