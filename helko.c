#include <linux/module.h> 
#include <linux/printk.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <linux/pci.h>

#define HW_VENDOR_ID (0x8086)
#define HW_DEVICE_ID (0x100e)

static struct pci_device_id nic_card_ids[] = {
    { PCI_DEVICE(HW_VENDOR_ID, HW_DEVICE_ID) },
    { }
};
MODULE_DEVICE_TABLE(pci, nic_card_ids);

static int hw_probe(struct pci_dev* dev, const struct pci_device_id* id) {
    u16 device_id = 0;
    u32 output = 0;
    void* ptr_bar0 = NULL;
    u64 addr = 0;
    int res = 0;

    pr_info("yogo probing device...\n");
    
    res = pci_read_config_word(dev, 0x2, &device_id);
    if (res != 0) {
        pr_info("Failed to read device ID from configspace\n");
        return -1;
    }
    if (device_id == HW_DEVICE_ID) {
        pr_info("Confirmed correct DID\n");
    } else {
        pr_info("Unable to coinfirm correct DID\n");
    }

    res = pci_resource_len(dev, 0);
    if(res < 0) {
        pr_info("failed to read BAR0 len due to %d\n", res);
        return res;
    }
    pr_info("BAR0 len is %d\n", res);
    addr = pci_resource_start(dev, 0);
    pr_info("bar0 is mapped to: %llx\n", addr);
    res = pcim_enable_device(dev);
    if(res < 0) {
        pr_info("failed to enavle pcim device due to %d\n", res);
        return -1;
    }
    
    res = pcim_iomap_regions(dev, BIT(0), KBUILD_MODNAME);
    if(res < 0) {
        pr_info("failed to iomap region due to %d\n", res);
        return -1;
    }
    
    ptr_bar0 = pcim_iomap_table(dev)[0];
    if(ptr_bar0 == NULL) {
        pr_info("failed to get bar0 address\n");
        return -1;
    }

    output = ioread32(ptr_bar0 + 0x2);
    pr_info("Got output: %x\n", output); //0x4000
    
    return 0;
}

static void hw_remove(struct pci_dev* dev) {
    pr_info("yogo removing device...\n");
}

static struct pci_driver hw_driver = {
    .name = "yogo_e1000_driver",
    .id_table = nic_card_ids,
    .probe = hw_probe,
    .remove = hw_remove
};


static struct net_device *yogo_netdev = NULL;

static void gen_packet(void) {
    struct sk_buff *skb = NULL;
    int pkt_len = 1000; // length of your received data
    u8 data[1000] = {0};    // pointer to received data
    struct net_device* dev = yogo_netdev;

    skb = netdev_alloc_skb(dev, pkt_len);
    if (!skb) {
        return;
    }

    skb_put_data(skb, data, pkt_len);
    skb->protocol = eth_type_trans(skb, dev);
    skb->ip_summed = CHECKSUM_UNNECESSARY; // or CHECKSUM_NONE depending on your HW

    netif_rx(skb); // hand off to the network stack
    dev->stats.rx_packets++;
    dev->stats.rx_bytes += pkt_len;
}

static int yogo_open(struct net_device* dev) {
    netif_start_queue(dev);
    return 0;
}

static int yogo_close(struct net_device* dev) {
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t yogo_start_xmit(struct sk_buff *skb, struct net_device* dev) {
    pr_info("got a new pkt to xmit\n");
    dev_kfree_skb(skb);
    gen_packet();
    return NETDEV_TX_OK;
}

static const struct net_device_ops yogo_netdev_ops = {
    .ndo_open = yogo_open,
    .ndo_stop = yogo_close,
    .ndo_start_xmit = yogo_start_xmit,
};

static void setup_yogo_netdev(struct net_device* dev) {
    ether_setup(dev);
    dev->netdev_ops = &yogo_netdev_ops;
    // dev->flags |= IFF_NOARP;
    memset(dev->dev_addr, 0, ETH_ALEN);
}

int init_module(void) 
{ 
    int res = 0;
    
    // PCI side:
    pr_info("Starting yogo netdev module\n"); 
    
    res = pci_register_driver(&hw_driver);
    if (res != 0) {
        return res;
    }

    // Networking Side:
    yogo_netdev = alloc_netdev(0, "yogonet%d", NET_NAME_UNKNOWN, setup_yogo_netdev);
    if (!yogo_netdev) {
        return -ENOMEM;
    }

    return register_netdev(yogo_netdev);
} 
 
void cleanup_module(void) 
{
    pr_info("Closing yogo netdev module\n"); 
    
    pci_unregister_driver(&hw_driver);
    if(yogo_netdev != NULL) {
        unregister_netdev(yogo_netdev);
        free_netdev(yogo_netdev); 
        yogo_netdev = NULL;
    }
} 
 
MODULE_LICENSE("GPL");