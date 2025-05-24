## Identifying Hardware
As a strarting point, I need to know what NIC I am working with.
If I run `lsmod | grep e1000` I see an entry with this name.
Also, I can see an intresting `lspci` entry:
```
00:03.0 Ethernet controller: Intel Corporation 82540EM Gigabit Ethernet Controller (rev 02)
```
From an online search it seems there really is a physical NIC with this name.
So either:
1. this is access to the physical NIC on my PC, or
2. a virtual NIC with this name is somehow exposed to my VM.

Let's see what my windows (host) device manager sees...
On the device manager I see one Intel WiFi adapter and one VirtualBox Ethernet adapter.
So it's looking like option 2. is the case.

If I run `lspci -v -s 00:03.0` I can also confirm that `Kernel driver in use: e1000`.

Was also able to find a datasheet for `82540EM` and saved it here.

So I connected another virtual NIC of type `82540EM`, and it appears on `00:08.0` as `enp0s8`.

## Unbinding Starndard Driver
So currently the `e1000` driver is managing my NIC on BDF `00:08.0`, so how can I make it leave this hardware alone?
I did:
```bash
cd /sys/bus/pci/devices/0000:00:08.0/driver
echo 0000:00:08.0 > unbind
```
Now when I run `lspci` on it I don't see any associated drivers.
But I can still see `e1000` in list of kernel modules.

I looked at it a bit and the datasheet was not detailed enough in terms of the functional interface offered to the CPU, so I found a (somewhat generic e1000) software manual which seems more detailed. It is an official document from intel and it says that it applies to the `82540EM` hardware specifically.

## Registering a network device
So I asked GPT for an example of this, specifically a linux module source which registers a new network device that does nothing. And the example essentially compiles.

After building it, and installing the module, I can see in `ip a` an entry for `yogonet0` which means it actually works.

## Requesting and Implementing network device ioctls
So it looks like in my net device ops I can add a field `.ndo_do_ioctl`.
But what fd should actually receive this ioctl from userspace?

So far It's looking like either opening the relevant device in sysfs or opening a socket does not cause the ioctl to arrive at my kernel code.

I will try to folow the impl of the network device registration in the core kernel.

Starting from `net/core/dev.c: register_netdev`...
After some digging I found the following in the docstring of the definition of the `struct net_device_ops`:
```c
/* ...
 * int (*ndo_do_ioctl)(struct net_device *dev, struct ifreq *ifr, int cmd);
 *	Old-style ioctl entry point. This is used internally by the
 *	ieee802154 subsystem but is no longer called by the device
 *	ioctl handler.
 */
```

So I will be leaving this idea alone, and avoid ioctls.