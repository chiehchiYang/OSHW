#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#define NETLINK_USER 26

struct sock *nl_sk = NULL;
extern struct net init_net;

module_init(pstree_init);
module_exit(pstree_exit);
int send_usrmsg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    int ret;

    /* allocked sk_buff  */
    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if (!nl_skb)
    {
        printk("netlink alloc failure\n");
        return -1;
    }

    /* 设置netlink消息头部 */
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_USER, len, 0);
    if (nlh == NULL)
    {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }
    //copy message to send
    memcpy(nlmsg_data(nlh), pbuf, len); //( ,msg, msg_size)
    ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);
    if (res < 0)
        printk("Error while sending bak to user\n");

    return ret;
}
static void netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
    char *kmsg = "hello users!!!"; //

    if (skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(skb);
        umsg = NLMSG_DATA(nlh);
        if (umsg)
        {
            printk("kernel recv from user: %s\n", umsg);
            send_usrmsg(kmsg, strlen(kmsg));
        }
    }
}

static int pstree_init(void)
{
    /* create netlink socket */
    struct netlink_kernel_cfg cfg = {
        .input = netlink_rcv_msg,
    };
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (nl_sk == NULL)
    {
        printk("Error, creating socket!\n");
        return -1;
    }
    printk("pstree_init\n");
    return 0;
}
static void pstree_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}
MODULE_LICENSE("GPL");