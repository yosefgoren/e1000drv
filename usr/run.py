import socket as sc


s = sc.socket(sc.AF_PACKET, sc.SOCK_RAW, sc.ntohs(3))
s.bind(("yogonet0", 0))
pak, info = s.recvfrom(2000)
print(pak)
print(info)