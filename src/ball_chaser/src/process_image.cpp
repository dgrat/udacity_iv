#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <string>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_me(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Driving the robot");

    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested motor commands
    if (!client.call(srv)) {
        ROS_ERROR("Failed to call service command_robot");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img) {
    const int white_pixel = 255;
    const int left  = 1 * img.width / 3.f;
    const int right = 2 * img.width / 3.f;

    size_t num_l = 0;
    size_t num_r = 0;
    size_t num_m = 0;

    ROS_INFO_STREAM(std::to_string(img.height) + "; " + std::to_string(img.width) + "; " + std::to_string(img.step));

    for(int y = 0; y < img.height; y++) 
    for(int x = 0; x < img.width; x++) {
        size_t id = y*img.width+x;
        int r = img.data[id*3+0];
        int g = img.data[id*3+1];
        int b = img.data[id*3+2];
        bool white = (r+g+b) == 3*white_pixel;

        if (!white) continue;

        if (x <= left) { num_l++; continue; }
        if (x >= right) { num_r++; continue; }
        if (x > left &&  x < right) num_m++;
    }

    if(num_l > num_r && num_l > num_m) {
        drive_me(0.1, +0.5);
        ROS_INFO_STREAM("Driving the robot left");
    }
    else if(num_r > num_l && num_r > num_m) {
        drive_me(0.1, -0.5);
        ROS_INFO_STREAM("Driving the robot right");
    }
    else if(num_m > num_r && num_m > num_l) {
        drive_me(0.1, 0);
        ROS_INFO_STREAM("Driving the robot straight");
    }
    else {
        drive_me(0.0, +0.5);
        ROS_INFO_STREAM("Turn the robot");
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
