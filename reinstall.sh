set -e
make
rmmod helko || echo "module was not installed"
insmod helko.ko
ip l set up dev yogonet0
ip a a 192.168.77.24/24 dev yogonet0