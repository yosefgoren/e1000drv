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

## Registering a linux network device
