#!/usr/bin/env python
import rospy
from race.msg import drive_param
from race.msg import pid_input

kp = 14.0 
kd = 0.09 * 10 
servo_offset = 0
prev_error = 0.0
vel_input = 10075

pub = rospy.Publisher('drive_parameters', drive_param, queue_size=10)




def control(data):
	global prev_error
	global vel_input
	global kp
	global kd
	global kp
	global kd
	
	#vel_input = vel_input + 1
	msg = drive_param()
	msg.velocity = vel_input

	## Your code goes here
	# 1. Scale the error
	# 2. Apply the PID equation on error
	# 3. Make sure the error is within bounds
	## END
	
	

	error = data.pid_error * kp
	deriv = kd * (error - prev_error)
	
	
        angle = error + deriv

        if angle > 10:
            angle = 10
        elif angle < -10:
            angle = -10

        prev_error = error


	msg.angle = angle

	print(msg.angle)
	print("             ")
	print(msg.velocity)
	pub.publish(msg)

if __name__ == '__main__':
	#global kp
	#global kd
	#global vel_input
	print("Listening to error for PID")
	#kp = input("Enter Kp Value: ")
	#kd = input("Enter Kd Value: ")
	#vel_input = input("Enter Velocity: ")
	rospy.init_node('pid_controller', anonymous=True)
	rospy.Subscriber("error", pid_input, control)
	rospy.spin()
