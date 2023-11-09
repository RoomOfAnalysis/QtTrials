#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <chrono>

int main()
{
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) // if not success, exit program
    {
        printf("not able to open the cam\n");
        return -1;
    }
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    cap.set(cv::CAP_PROP_FPS, 60);

    printf("after setting fps : %f\n", cap.get(cv::CAP_PROP_FPS));

    cv::Mat frame;
    std::chrono::steady_clock::time_point now;
    while (true)
    {
        now = std::chrono::steady_clock::now();
        if (!cap.read(frame) || frame.empty())
        {
            printf("not able to read a frame from video stream\n");
            break;
        }
        // FIXME: fps is still not stable at 60fps after setting... (50-80)
        printf(
            "fps : %f\n",
            1000.0 /
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - now).count());
        cv::imshow("live", frame);

        if (cv::waitKey(1) == 27)
        {
            printf("esc key is pressed by user\n");
            break;
        }
    }

    return 0;
}