#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"
#include <memory>
#include <chrono>

std::string src = "nvarguscamerasrc sensor-id=0 ! \
	video/x-raw(memory:NVMM), width=(int)640, height=(int)360, \
    format=(string)NV12 ! nvvidconv flip-method=0 ! video/x-raw, \
    width=(int)640, height=(int)360, format=(string)BGRx ! \
	videoconvert ! video/x-raw, format=(string)BGR ! appsink";

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camPub_Node");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();
    auto mypub = node->create_publisher<sensor_msgs::msg::CompressedImage>("image/compressed", qos_profile);
    
    std_msgs::msg::Header hdr; //메시지 헤더
    sensor_msgs::msg::CompressedImage::SharedPtr msg;
    rclcpp::WallRate loop_rate(40.0);

    //gstreamer 영상
    cv::VideoCapture cap(src, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) {
        RCLCPP_ERROR(node->get_logger(), "Could not open video!");
        rclcpp::shutdown();
        return -1;
    }

    //test 영상
    /*cv::VideoCapture test_cap("/home/jetson/simulation/test.mp4");
    if (!test_cap.isOpened()) { 
        RCLCPP_WARN(node->get_logger(), "Could not open video!");
        return -1;
    }*/

    cv::Mat frame;
    while(rclcpp::ok())
    {
        cap >> frame;
        //test_cap >> frame;
        if (frame.empty()) { 
            RCLCPP_ERROR(node->get_logger(), "frame empty");
            break;
        }
        msg = cv_bridge::CvImage(hdr, "bgr8", frame).toCompressedImageMsg(); //cv::Mat 객체(frame) -> CvImage 객체로 변환
        mypub->publish(*msg); //msg는 객체 => std::shared_ptr<sensor_msgs::msg::CompressedImage> ('line 22'와 비교)
        loop_rate.sleep();
    }
    rclcpp::shutdown();
    return 0;
}