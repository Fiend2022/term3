#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "SFML/Audio.hpp"
#include "SFML/Audio/SoundSource.hpp"
#include "FingerDetect.hpp"



int main()
{
    cv::VideoCapture cap(0);
    cv::namedWindow(windowCaptureName);
    cv::namedWindow(windowTreshold);
    trackbarBuild();
    Shot frame, frame_HSV, Hsv;
    frame.setName(windowCaptureName);
    Hsv.setName(windowTreshold);
    FingerDetect hand;
    sf::SoundBuffer buffer;
    buffer.loadFromFile("sound.wav");
    sf::Sound sound;
    sound.setBuffer(buffer);
    while (true)
    {
        frame.SetFrame(cap);
        if (frame.empty())
            break;
        frame.convertToHsv(Hsv);
        Hsv.detectObject();
        Contour contour;
        contour.find(Hsv);
        Hull hull;
        hull.find(contour);
        hand.findTheFingers(hull, contour,frame);
        hand.whatColor();
        hand.drawing(frame);    
        frame.flip();
        Hsv.flip();
        cv::String text = "Fingers: ";
        if (hand.getFingersCount() == 1 && (sound.getStatus() != sf::Sound::Playing))
            sound.play();
        text += to_string(hand.getFingersCount());
        putText(frame.getFrame(), text, Point(20, 20), 1, 1, White);
        frame.show();
        Hsv.show();
        contour.clear();
        hull.clear();
        hand.zeroing();
        char key = static_cast<char>(waitKey(30));
        if (key == 'q' || key == 27)
            break;
    }
    return 0;
}

