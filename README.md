# Heterogeneous Multi-Robot Communication using Kafka

This repository contains the CPP libraries to run the Kafka client. This is based on [cppkakfa](https://github.com/mfontanini/cppkafka) library.

# Installation Instructions - 

### Device Node
1. **DJI OSDK** - Download and install the the modified DJI SDK from [here](https://github.com/vidurkakar/Onboard-SDK-x86).
Enter the following commands to build the OSDK repository-
```Shell
$ mkdir build
$ cd build..
$ cmake ..
$ make
```
2. **LIBRDKAFKA** - Download and install the libdrdkafka library from [here](https://github.com/edenhill/librdkafka)
Enter the following commands to build the librdkafka repository-
```Shell
$ ./configure --install-deps
$ make
$ sudo make install
```
3. **CPP KAFKA** - In order to compile _cppkafka_ you need:
    * _librdkafka >= 0.9.4_
    * _CMake_
    * A compiler with good C++ 11 support (e.g. gcc >= 4.8). This was tested successfully on _g++ 4.8.3_. 
    * The boost library.

     Now in this repository, go to  */examples/CMakeLists.txt*. Change the directory paths according to the path where you installed your DJI OSDK. Once the paths are successfully changed, enter the followings commands to compile - 
```Shell
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make examples
```


### Edge Node
Download Edge Node repository from [here](https://github.com/vidurkakar/Spring-Kafka-Edge-Node-String).

### Cloud Node
Download Cloud Node repository from [here](https://github.com/vidurkakar/Spring-Kafka-Cloud-Node-String).

### Kafka
Download the binary source of Kafka from [Apache Kafka's](https://kafka.apache.org/downloads) website. To run kafka on the main system enter the following commands - 
1. Start the Zookeeper - ```
./zookeeper-server-start.sh ../config/zookeeper.properties```
2. Start the Kafka Broker - ```
./kafka-server-start.sh ../config/server.properties```
3. Your *server.properties* will contain the details of the of the IP address and the port where your broker is published. In the file add- ```
advertised.listeners=PLAINTEXT://your_local_host_name:port```
Also number the *broker.id* and *num.partitions* accordingly.

# Running the Device Node - 
Go to ``` /build/examples``` and run the following command - 
```
$ ./try-consumer --brokers *ip_address:port* --topic *topic_name* --group-id *groud_id*
```

For example- 
 ```
$ ./try-consumer --brokers 192.168.1.250:9092 --topic deviceNodeReq2 --group-id helloworld44
```


Example GET Commands can be found on my Post profile [here](https://documenter.getpostman.com/view/7078648/S1LwynQh)

A GET command has the following format -

```
http://yourBrokerIP:currentNodePort/serviceNumber?request=DestinationNode#SourceNode#MessagePayload#ResponseNode
```
Note: Escape character %23 is used for **#**

In the payload for OSDK command, we write in the following format - 
```
OSDK:theCommand:InputValuesIfAny
```

Example OSDK Payload - 
```
OSDK:move:1:0:0:0
```
Note: Escape character %3A is used for **:**

Example GET Command - 

```sh
http://localhost:8060/service2?request=deviceNodeReq2%23EdgeNode1%23OSDK%3Atakeoff%23edgeNodeResp1
```

-------------------
 [CAS Lab, Virginia Tech](https://caslab.ece.vt.edu/)
