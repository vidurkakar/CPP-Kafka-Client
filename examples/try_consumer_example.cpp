#include <stdexcept>
#include <iostream>
#include <string>
#include <cmath>
#include <csignal>
#include "/usr/include/boost/program_options.hpp"
#include "cppkafka/consumer.h"
#include "cppkafka/configuration.h"
#include "flight_control_sample.cpp"
#include "/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common/dji_linux_environment.cpp"
#include "/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common/dji_linux_helpers.cpp"
#include "dji_telemetry.hpp"

#include "cppkafka/producer.h"
#include "cppkafka/configuration.h"


#include "flight_control_sample.hpp"
#include "producerCall2.cpp"

using namespace DJI::OSDK;
using namespace DJI::OSDK::Telemetry;

Telemetry::Status         status;
Telemetry::GlobalPosition globalPosition;
Telemetry::GPSInfo        gpsInfo;
Telemetry::RC             rc;
Telemetry::Vector3f       velocity;
Telemetry::Quaternion     quaternion;
Telemetry::Battery        battery;

using std::string;
using std::exception;
using std::cout;
using std::endl;

using cppkafka::Consumer;
using cppkafka::Configuration;
using cppkafka::Message;
using cppkafka::TopicPartitionList;


using cppkafka::Producer;
using cppkafka::Configuration;
using cppkafka::Topic;
using cppkafka::MessageBuilder;

namespace po = boost::program_options;

bool running = true;

int main(int argc, char* argv[]) {
    cout << "Hello TRY CPP Example" << endl;

    string brokers;
    string consumer_topic_name;
    string producer_topic_name="edgeNodeResp1";
    string group_id;
    int partition_value = 0;   //Added for producer.
    int responseMsgCount=0;
    string producerPayload;


    po::options_description options("Options");
    options.add_options()
        ("help,h",     "produce this help message")
        ("brokers,b",  po::value<string>(&brokers)->required(),
                       "the kafka broker list")
        ("topic,t",    po::value<string>(&consumer_topic_name)->required(),
                       "the topic in which to write to")
        ("group-id,g", po::value<string>(&group_id)->required(),
                       "the consumer group id")
        ;

    po::variables_map vm;

    try {
        po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
        po::notify(vm);
    }
    catch (exception& ex) {
        cout << "Error parsing options: " << ex.what() << endl;
        cout << endl;
        cout << options << endl;
        return 1;
    }

    // Stop processing on SIGINT
    signal(SIGINT, [](int) { running = false; });

    // Construct the Consumer configuration
    Configuration configConsumer = {
        { "metadata.broker.list", brokers },
        { "group.id", group_id },
        // Disable auto commit
        { "enable.auto.commit", false }
    };

    // Create the consumer
    Consumer consumer(configConsumer);

    // Construct the Producer configuration
    Configuration configProducer = {
        { "metadata.broker.list", brokers }
    };

    // Create the producer
    Producer producer(configProducer);

    // Print the assigned partitions on assignment
    consumer.set_assignment_callback([](const TopicPartitionList& partitions) {
        cout << "Got assigned: " << partitions << endl;
    });

    // Print the revoked partitions on revocation
    consumer.set_revocation_callback([](const TopicPartitionList& partitions) {
        cout << "Got revoked: " << partitions << endl;
    });

    // Subscribe to the topic
    consumer.subscribe({ consumer_topic_name });

    cout << "Consuming messages from topic " << consumer_topic_name << endl;

	// Initialize OSDK variables
    int functionTimeout = 1;
    int argc1=2;
    char* argv1[]= {"","UserConfig.txt"};
    cout << "Config File Path: " << argv1[1] << endl;
    // Setup OSDK.
	LinuxSetup linuxEnvironment(argc1, argv1);
	Vehicle*   vehicle = linuxEnvironment.getVehicle();

    // Now read lines and write them into kafka
    while (running) {




        // Try to consume a message
        Message msg = consumer.poll();
        if (msg) {
            // If we managed to get a message
            if (msg.get_error()) {
                // Ignore EOF notifications from rdkafka
                if (!msg.is_eof()) {
                    cout << "[+] Received error notification: " << msg.get_error() << endl;
                }
            }
            else {
		        // Print the key (if any)
		        if (msg.get_key()) {
		            cout << msg.get_key() << " -> ";
		        }
		        // Print the payload
		        cout << msg.get_payload() << endl;

			//------------Payload Processing----------

			//Split Main Payload
			string s, tmp;
			s=msg.get_payload();
			std::stringstream ss(s);
			std::vector<std::string> words;

			while(getline(ss, tmp, '#')){
	   		 words.push_back(tmp);
			}

			for(int i=0;i<words.size();i++)
			{
			cout << words.at(i) << endl;

			//cout << "Payload Start: " << words.at(2).substr(0,4) << endl;
			}


			// Test if Payload OSDK Command
			if (words.at(2).substr(0,4).compare("OSDK")==0)
			{
				cout << "OSDK command" << endl;
				string sCommand, tmpCommand;
				sCommand=words.at(2);
				std::stringstream ssCommand(sCommand);
				std::vector<std::string> wordsCommand;

				while(getline(ssCommand, tmpCommand, ':')){
		   		 wordsCommand.push_back(tmpCommand);
				}

				//Print Command Payload
				/*
				for(int i=1;i<wordsCommand.size();i++)
				{
				cout << "OSDK Payload: "<<wordsCommand.at(i) << endl;

				//cout << "Payload Start: " << words.at(2).substr(0,4) << endl;
				}
				*/


				//Decide which command to call

				string droneCommand = wordsCommand.at(1);
				string TAKEOFF="takeoff", LAND="land", MOVE="move",DRONESTATUS="status";
				cout << "Drone Command String: " << droneCommand << endl;


				if(droneCommand.compare(TAKEOFF)==0)
				{
					cout << " Drone Takeoff Command" << endl;
					vehicle->obtainCtrlAuthority(functionTimeout);
      				monitoredTakeoff(vehicle);                  //Call Takeoff Command

					//SEND ACK
					MessageBuilder builder(words.at(3));     // Create a message builder for this topic
					builder.partition(partition_value);   	  // Get the partition we want to write to.

					producerPayload=words.at(3)+"#"+"TAKEOFF Command ACK from "+consumer_topic_name+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
					builder.payload(producerPayload);
					responseMsgCount++;

					// Actually produce the message we've built
					producer.produce(builder);
				}
				else if(droneCommand.compare(MOVE)==0)

				{	if(wordsCommand.size()==6)
					{
                        cout << "Drone Move Command " << endl;
                        cout << "x: " << wordsCommand.at(2) << endl;
                        cout << "y: " << wordsCommand.at(3) << endl;
                        cout << "z: " << wordsCommand.at(4) << endl;
                        cout << "Yaw: " << wordsCommand.at(5) << endl;
                        vehicle->obtainCtrlAuthority(functionTimeout);

                        //Call Drone Move Command
                        moveByPositionOffset(vehicle, strtof((wordsCommand.at(2)).c_str(),0), strtof((wordsCommand.at(3)).c_str(),0), strtof((wordsCommand.at(4)).c_str(),0), strtof((wordsCommand.at(5)).c_str(),0));

                        //SEND ACK
                        MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                        builder.partition(partition_value);   	  // Get the partition we want to write to.

                        producerPayload=words.at(3)+"#"+"MOVE Command ACK from "+consumer_topic_name+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                        builder.payload(producerPayload);
                        responseMsgCount++;

                        // Actually produce ACK in topic
                        producer.produce(builder);
					}
					else
					{cout<< "Incorrect Move Command" << endl;}
				}

				else if(droneCommand.compare(LAND)==0)
				{
					cout << "Drone Land Command" << endl;
					vehicle->obtainCtrlAuthority(functionTimeout);
                    monitoredLanding(vehicle);              //Call Drone Land Command

					//SEND ACK
					MessageBuilder builder(words.at(3));     // Create a message builder for this topic
					builder.partition(partition_value);   	  // Get the partition we want to write to.

					producerPayload=words.at(3)+"#"+"LAND Command ACK from "+consumer_topic_name+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
					builder.payload(producerPayload);
					responseMsgCount++;

					// Actually produce ACK in topic
					producer.produce(builder);
				}
				else if(droneCommand.compare(DRONESTATUS)==0)
				{   //Check status in 2 position

                    if(wordsCommand.size()==3)
                    {
                        //GPS Position
                        if(wordsCommand.at(2).compare("position")==0)
                        {
                            globalPosition = vehicle->broadcast->getGlobalPosition();
                            globalPosition.latitude=globalPosition.latitude*(180/M_PI);
                            globalPosition.longitude=globalPosition.longitude*(180/M_PI);

                            string strLatitude=std::to_string(globalPosition.latitude);
                            string strLongitude=std::to_string(globalPosition.longitude);
                            string strAltitude=std::to_string(globalPosition.altitude);

                             cout << "Position in degree      (LLA)           = "
                                      << strLatitude << ", " << strLongitude
                                      << ", " << strAltitude << "\n";

                            //SEND ACK
                            MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                            builder.partition(partition_value);   	  // Get the partition we want to write to.

                            producerPayload=words.at(3)+"#"+"Position= [Latitiude: "+ strLatitude +", Longitude: "+ strLongitude + ", Altitude: "+ strAltitude+"]"+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                            builder.payload(producerPayload);
                            responseMsgCount++;

                            // Actually produce ACK in topic
                            producer.produce(builder);
                        }

                        //Drone Velocity
                        else if(wordsCommand.at(2).compare("velocity")==0)
                        {
                            velocity = vehicle->broadcast->getVelocity();

                            string strVelX=std::to_string(velocity.x);
                            string strVelY=std::to_string(velocity.y);
                            string strVelZ=std::to_string(velocity.z);

                            cout << "Velocity              (vx,vy,vz)      = " << strVelX
                                  << ", " << strVelY << ", " << strVelZ << "\n";

                            //SEND ACK
                            MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                            builder.partition(partition_value);   	  // Get the partition we want to write to.

                            producerPayload=words.at(3)+"#"+"Velocity= [Vx: "+ strVelX +", Vy: "+ strVelY + ", Vz: "+ strVelZ+"]"+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                            builder.payload(producerPayload);
                            responseMsgCount++;

                            // Actually produce ACK in topic
                            producer.produce(builder);
                        }

                        //RC Values
                        else if(wordsCommand.at(2).compare("getrc")==0)
                        {
                            rc = vehicle->broadcast->getRC();

                            string strRcRoll=std::to_string(rc.roll);
                            string strRcPitch=std::to_string(rc.pitch);
                            string strRcYaw=std::to_string(rc.yaw);
                            string strRcThrottle=std::to_string(rc.throttle);

                             cout << "RC Commands           (r/p/y/thr)     = " << strRcRoll << ", "
                                  << strRcPitch << ", " << strRcYaw << ", " << strRcThrottle << "\n";

                            //SEND ACK
                            MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                            builder.partition(partition_value);   	  // Get the partition we want to write to.

                            producerPayload=words.at(3)+"#"+"RC Commands(r/p/y/thr)= [Roll: "+ strRcRoll +", Pitch: "+ strRcPitch + ", Yaw: "+ strRcYaw+", Throttle: "+ strRcThrottle+"]"+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                            builder.payload(producerPayload);
                            responseMsgCount++;

                            // Actually produce ACK in topic
                            producer.produce(builder);
                        }

                        //Attitude Quaternion
                        else if(wordsCommand.at(2).compare("quaternion")==0)
                        {
                            quaternion = vehicle->broadcast->getQuaternion();

                            string strQuaternion_0=std::to_string(quaternion.q0);
                            string strQuaternion_1=std::to_string(quaternion.q1);
                            string strQuaternion_2=std::to_string(quaternion.q2);
                            string strQuaternion_3=std::to_string(quaternion.q3);

                            cout << "Attitude Quaternion   (w,x,y,z)       = " << strQuaternion_0
                                  << ", " << strQuaternion_1 << ", " << strQuaternion_2 << ", "
                                  << strQuaternion_3 << "\n";

                            //SEND ACK
                            MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                            builder.partition(partition_value);   	  // Get the partition we want to write to.

                            producerPayload=words.at(3)+"#"+"Attitude Quaternion (w,x,y,z)= [w: "+ strQuaternion_0 +", x: "+ strQuaternion_1 + ", y: "+ strQuaternion_2+", z: "+ strQuaternion_3+"]"+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                            builder.payload(producerPayload);
                            responseMsgCount++;

                           // Actually produce ACK in topic
                            producer.produce(builder);
                        }

                        //Battery Percentage
                        else if(wordsCommand.at(2).compare("battery")==0)
                        {
                            battery = vehicle->broadcast->getBatteryInfo();

                            string strBatteryPercent=std::to_string(battery.percentage);


                            cout << "Battery Percentage     = " << strBatteryPercent<< "\n";

                            //SEND ACK
                            MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                            builder.partition(partition_value);   	  // Get the partition we want to write to.

                            producerPayload=words.at(3)+"#"+"Battery Percentage = " + strBatteryPercent +"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                            builder.payload(producerPayload);
                            responseMsgCount++;

                            // Actually produce ACK in topic
                            producer.produce(builder);
                        }

                        else
                        {
                            cout << "Unkown Status Command";

                            //SEND ACK
                            MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                            builder.partition(partition_value);   	  // Get the partition we want to write to.

                            producerPayload=words.at(3)+"#"+ "Unkown Drone Status  Command" +"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                            builder.payload(producerPayload);
                            responseMsgCount++;

                            // Actually produce ACK in topic
                            producer.produce(builder);
                        }
                    }
                    else //General Status Print
                    {
                        cout << "Drone Status Command" << endl;

                        //Get All the flight stats
                        status         = vehicle->broadcast->getStatus();
                        globalPosition = vehicle->broadcast->getGlobalPosition();
                        rc             = vehicle->broadcast->getRC();
                        velocity       = vehicle->broadcast->getVelocity();
                        quaternion     = vehicle->broadcast->getQuaternion();

                        globalPosition.latitude=globalPosition.latitude*(180/M_PI);
                        globalPosition.longitude=globalPosition.longitude*(180/M_PI);

                        cout << "-------\n";
                        cout << "Flight Status                         = "
                                  << (unsigned)status.flight << "\n";
                        cout << "Position   Radian     (LLA)           = "
                                  << globalPosition.latitude << ", " << globalPosition.longitude
                                  << ", " << globalPosition.altitude << "\n";
                        cout << "RC Commands           (r/p/y/thr)     = " << rc.roll << ", "
                                  << rc.pitch << ", " << rc.yaw << ", " << rc.throttle << "\n";
                        cout << "Velocity              (vx,vy,vz)      = " << velocity.x
                                  << ", " << velocity.y << ", " << velocity.z << "\n";
                        cout << "Attitude Quaternion   (w,x,y,z)       = " << quaternion.q0
                                  << ", " << quaternion.q1 << ", " << quaternion.q2 << ", "
                                  << quaternion.q3 << "\n";
                        cout << "-------\n\n";


                        //SEND ACK
                        MessageBuilder builder(words.at(3));     // Create a message builder for this topic
                        builder.partition(partition_value);   	  // Get the partition we want to write to.

                        producerPayload=words.at(3)+"#"+"Status Command ACK from "+consumer_topic_name+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
                        builder.payload(producerPayload);
                        responseMsgCount++;

                        // Actually produce ACK in topic
                        producer.produce(builder);
					}
				}
				else
				{
                    cout << "Unkown OSDK Command" << endl;

					MessageBuilder builder(words.at(3));     // Create a message builder for this topic
					builder.partition(partition_value);   	  // Get the partition we want to write to.

					producerPayload=words.at(3)+"#"+"Unkown OSDK Command ACK from "+consumer_topic_name+"#"+consumer_topic_name+"#"+std::to_string(responseMsgCount);// Set the payload on this builder
					builder.payload(producerPayload);
					responseMsgCount++;

					// Actually produce ACK in topic
					producer.produce(builder);
				}
			}

		cout << "====================================================" << endl;


                // Now commit the message
                consumer.commit(msg);

            	}
        }
    }
}
