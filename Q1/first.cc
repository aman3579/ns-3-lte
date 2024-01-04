#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("First.cc"); 

int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  // std::string phyMode("DsssRate11mbps");
  // double txp = 7.5; 
  
  Time::SetResolution (Time::MS);  //setting time resolution in nanoseconds i.e. speed of network will be measured in nanoseconds
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (2);    //we are creating two nodes     0----------------0


//nodes, channel, netdevice, application and topology helper
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("6000ms"));

//NIC Card (Network Interface card)
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);



  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");    //ip addrerss and subnet mask

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

//there are totally 65536 ports in a Networking device
  UdpEchoServerHelper echoServer (3459);


//first node is refereed as nodes.Get(0) and the second node is nodes.Get(1)
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 3459);    //  getAddress(1) //means address of port 2 and port number is  is 9
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));   //client is installed on node(2)
  clientApps.Start (Seconds (2.0));    //client should start after the server is enabled. server is started at 1second so client should start after 1 sec. 0.3 0.4etc are not possible
  clientApps.Stop (Seconds (10.0));
  
  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  Simulator::Stop(Seconds(100.0));
  Simulator::Run();
  
   monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier =  DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
   std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
   
   for (std::map<FlowId, FlowMonitor::FlowStats> ::const_iterator iter = stats.begin(); iter!=stats.end();++iter)
   {
   Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
   NS_LOG_UNCOND("Flow ID: "<<iter->first<<"Src Addr" <<t.sourceAddress<< "Dest address"<<t.destinationAddress);
   
   NS_LOG_UNCOND("Number of sent packets =" <<iter->second.txPackets );
   NS_LOG_UNCOND("Number of received packets =" <<iter->second.rxPackets );
   NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
   NS_LOG_UNCOND("Delay = "<< iter->second.delaySum);
   }
   
   
  Simulator::Destroy ();
  return 0;
  
}
