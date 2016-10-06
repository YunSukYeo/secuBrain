ip addr del 192.168.100.100/24 brd + dev sff0 label sff0:0
ip link delete sff0 type dummy
rmmod dummy
