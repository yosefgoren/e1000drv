#include <linux/module.h> 
#include <linux/printk.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

static struct net_device *yogo_netdev;

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
    pr_info("Starting yogo netdev module\n"); 
    yogo_netdev = alloc_netdev(0, "yogonet%d", NET_NAME_UNKNOWN, setup_yogo_netdev);
    if (!yogo_netdev) {
        return -ENOMEM;
    }

    return register_netdev(yogo_netdev);
} 
 
void cleanup_module(void) 
{
    pr_info("Closing yogo netdev module\n"); 
    
    unregister_netdev(yogo_netdev);
    free_netdev(yogo_netdev); 
} 
 
MODULE_LICENSE("GPL");