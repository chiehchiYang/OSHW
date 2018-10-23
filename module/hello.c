#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>

#define NETLINK_USER 31
static int int_pid = 0;
struct sock *nl_sk = NULL;
void sendnlmsg(char *msg, int pid)
{
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh;
    int msg_size;
    int res;
    //msg = (char *)nlmsg_data(nlh); //return message = hello

    msg_size = strlen(msg);

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */

    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    //struct sk_buff *skb_out;

    char *msg = kmalloc(1024, GFP_KERNEL); // = "Hello from kernel";
    msg[0] = '\0';
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid; /*pid of sending process */

    char number[1024];
    struct task_struct *task, *children;

    for_each_process(task)
    {
        if (task->mm == NULL)
        {
            //{ //>=0
            sprintf(number, "%s(%d)\n", task->comm, task->pid);
            strcat(msg, number);

            for_each_process(children)
            {
                if (children->real_parent->pid == task->pid)
                {
                    sprintf(number, "\t\t\t\t%s(%d)\n", task->comm, task->pid);
                    strcat(msg, number);
                }
            }
        }
    }
    sendnlmsg(msg, pid);
    /*

            //process_name(pid)p->comm p->pid
            strcat(msg, rightPa);
            
            sprintf(number, "%s\n", p->comm);
            printk(KERN_INFO "Netlink received msg payload:%s\n", p->comm);
 
    */
    //sendnlmsg(msg, pid);
}
void dfs(struct task_struct *task)
{
    struct task_struct *child;
    struct list_head *list;

    //printk(KERN_INFO "Name: %-20s state: %ld\tprocess id: %d\n", task->comm, task->state, task->pid);
    /*
    list_for_each(list, &task->children)
    {
        child = list_entry(list, struct task_struct, sibling);
        dfs(child);
    }
    */
}

static int __init hello_init(void)
{

    dfs(&init_task);

    printk("Entering: %s\n", __FUNCTION__);

    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
