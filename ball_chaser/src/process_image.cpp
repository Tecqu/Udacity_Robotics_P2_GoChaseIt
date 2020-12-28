#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
      ROS_INFO_STREAM("moving the robot...");
    // TODO: Request a service and pass the velocities to it to drive the robot
  ball_chaser::DriveToTarget srv;
  srv.request.linear_x = lin_x;
  srv.request.angular_z = ang_z;
  
   // Call the  ball_chaser::DriveToTarget (see drive_bot.cpp) service and pass the requested Twist
  if (!client.call(srv))
      ROS_ERROR("Failed to call service DriveToTarget");
  
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

  int white_pixel = 255;
  int WPix_lateral_pos = 0;
  bool WPix_found = false;
  
  ROS_INFO_STREAM("Processing the image... \n");
  ROS_INFO_STREAM(printf("img.height: %d, img.width: %d, img.step: %d \n", img.height, img.width, img.step));
  
  // TODO: Loop through each pixel in the image and check if there's a bright white one
  
  //Data representation of one pixel [ R_px1, G_px1, B_px1, R_px2, G_px2, B_px2, ... ]
  for(int i = 0; i < img.height; i++){ // go through rows
  	for(int j = 0; j < img.step; j = j+3)
  	{ // go through columns in a 3-step; *3 as consecutive order of R, G, B pixels
      		if(img.data[i*img.step + j  ] == white_pixel && 
      		   img.data[i*img.step + j+1] == white_pixel && 
      		   img.data[i*img.step + j+2] == white_pixel){
        		WPix_lateral_pos = j/3;
        		WPix_found = true;
      			break; //white pixel found -> continue identifying which part of the image contains the white pixel
      		}
    	}
    
    if(WPix_found == true)
      break; // if white pixel was found stop further looping through next rows
  }

	ROS_INFO_STREAM(printf("Lateral Pos of white ball: %d \n", WPix_lateral_pos));
  float lin_x = 0;
  float ang_z = 0;
  
  if(WPix_found == true){
      // Then, identify if this pixel falls in the left, mid, or right side of the image
    // <1/3 of img width -> left
    // <2/3 of img width -> middle
    // >2/3 of img width -> right
    
    if(WPix_lateral_pos > 0 && WPix_lateral_pos < img.width/3){
      ang_z = 0.5; //turn left
      ROS_INFO_STREAM("turn left \n");
    }
    else if(WPix_lateral_pos > (img.width*2/3) && WPix_lateral_pos < img.width){
      ang_z = -0.5;//turn right
      ROS_INFO_STREAM("turn right \n");
    }
    else if(WPix_lateral_pos >= img.width/3 && WPix_lateral_pos <= (img.width*2/3)){
      lin_x = 0.5;
      ROS_INFO_STREAM("move straight \n");
    }
    else{
      ROS_INFO_STREAM("do nothing \n");
    }   
  }
  // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
	// -> lin_x and ang_z will not be set, if WPix_found == false
  drive_robot(lin_x, ang_z);
     ROS_INFO_STREAM("\n");
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
