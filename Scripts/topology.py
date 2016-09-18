#!/usr/bin/python
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel
from mininet.node import RemoteController
from functools import partial
from time import sleep
from mininet.cli import CLI
from mininet.link import Link

class SingleSwitchTopo(Topo):
    "Single switch connected to n hosts."
    def build(self, n=2):
        switch_1 = self.addSwitch('switch1');
	switch_2 = self.addSwitch('switch2');
        switch_3 = self.addSwitch('switch3');
	switch_4 = self.addSwitch('switch4');

        sff1 = self.addHost('sff1', privateDirs=['/home/secu/Hackathon/SFF'], ip='10.0.0.100');
        self.addLink(sff1, switch_2);
      
 
        firewall = self.addHost('firewall', privateDirs=['/home/secu/Hackathon/NSF'], ip='10.0.0.200');
        self.addLink(firewall, switch_1);

	# Source Nodes
	for h in range(n / 2):
            host = self.addHost('h%s' % (h + 1), privateDirs=['/home/secu/Hackathon/PacketGeneration'])
            self.addLink(host, switch_3);

	#Destination Nodes
        for h in range(n / 2, n):
            host = self.addHost('h%s' % (h + 1), privateDirs=['/home/secu/Hackathon/PacketReceive'])
            self.addLink(host, switch_4);

        self.addLink(switch_1, switch_2);
        self.addLink(switch_2, switch_3);
        self.addLink(switch_2, switch_4);

def simpleTest():
    "Create and test a simple network"

    topo = SingleSwitchTopo(n=4)
    net = Mininet(topo, controller=partial(RemoteController, ip='192.168.30.131', port=6633))
    net.start();
 
    net.pingAll();
    # Inintalize components
    sff = net.get('sff1');
    packetGenerator = net.get('h1');
    destination = net.get('h3');
    firewall = net.get('firewall');

    #sff.cmd('./packetSinffer > /tmp/sff.out &');
    sff.cmd('../bin/sff sff1-eth0 > /tmp/sff.out &');
    #sff.cmd('./sff sff1' + ' ' + sff.IP() + ' > /tmp/sff.out &');

    firewall.cmd('../bin/firewall firewall-eth0 > /tmp/firewall.out &');

    # In order to check flow rule
    destination.cmd('../bin/ipPacketReceiver ', destination.name, destination.IP(), ' > /tmp/destination.out &');

    # Wait server
    sleep(3);

    # Start Packet Generation
    packetGenerator.cmd('while true; do ../bin/ipPacketGenerator ', packetGenerator.IP(), destination.IP(), '; sleep 1; done > /tmp/generator.out &');

    # Wait For a While
    sleep(5);

    # Clear all program
    packetGenerator.cmd('kill %while');
    #sff.cmd('echo -n end', 'nc -4u -w1', sff.IP(),'8000');
    #sff.cmd('wait', sffProcessID);
    
    CLI(net)
    # Stop Simulation
    net.stop()

if __name__ == '__main__':
    "Tell mininet to print useful information"
    setLogLevel('info')
    simpleTest()
