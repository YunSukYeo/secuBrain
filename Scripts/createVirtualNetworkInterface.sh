modprobe dummy
lsmod | grep dummy
sudo ip link set name sff0 dev dummy0

ip link show sff0
ip addr add 192.168.100.100/24 brd + dev sff0 label sff0:0

ip a | grep -w inet
