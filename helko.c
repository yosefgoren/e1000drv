/* 
 * hello-1.c - The simplest kernel module. 
 */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/printk.h> /* Needed for pr_info() */ 

#include <linux/netdevice.h>

static struct net_device *yogo_netdev;


static int yogo_open(struct net_device* dev) {
    netif_start_queue(dev);
    return 0;
}

static int yogo_close(struct net_device* dev) {
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t yogo_start_xmit(struct sk_buff *skb, struct net_device* dev) {
    dev_kfree_skb(skb);
    return NETDEV_TX_OK;
}


static const struct net_device_ops yogo_netdev_ops = {
    .ndo_open = yogo_open,
    .ndo_stop = yogo_close,
    .ndo_start_xmit = yogo_start_xmit
};

static void setup_yogo_netdev(struct net_device* dev) {
    ether_setup(dev);
    dev->netdev_ops = &yogo_netdev_ops;
    dev->flags |= IFF_NOARP;
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