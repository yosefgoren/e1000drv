export BDF="0000:00:08.0"

set -e
make
rmmod helko || echo "module was not installed"
insmod helko.ko
ip l set up dev yogonet0
ip a a 192.168.77.24/24 dev yogonet0

# Not needed usually but might be useful:
# echo $BDF > /sys/bus/pci/drivers/yogo_e1000_driver/bind
lspci -s $BDF -v