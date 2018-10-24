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
int for_count;
char msg[10000];

int find_level(struct task_struct *task)
{
    int counter = 0;

    if (task->parent->pid != 0)
    {
        counter++;
        if (task->parent->parent->pid != 0)
        {
            counter++;
            if (task->parent->parent->parent->pid != 0) //3
            {
                counter++;
                if (task->parent->parent->parent->parent->pid != 0)
                {
                    counter++;
                    if (task->parent->parent->parent->parent->parent->pid != 0)
                    {
                        counter++;
                        if (task->parent->parent->parent->parent->parent->parent->pid != 0) //6
                        {
                            counter++;
                            if (task->parent->parent->parent->parent->parent->parent->parent->pid != 0) //7
                            {
                                counter++;
                                if (task->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //8
                                {
                                    counter++;
                                    if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //9
                                    {
                                        counter++;
                                        if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //10
                                        {
                                            counter++;
                                            if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //11
                                            {
                                                counter++;
                                                if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //12
                                                {
                                                    counter++;
                                                    if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //13
                                                    {
                                                        counter++;
                                                        if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //14
                                                        {
                                                            counter++;
                                                            if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //15
                                                            {
                                                                counter++;
                                                                if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //16
                                                                {
                                                                    counter++;
                                                                    if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //17
                                                                    {
                                                                        counter++;
                                                                        if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //18
                                                                        {
                                                                            counter++;
                                                                            if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //19
                                                                            {
                                                                                counter++;
                                                                                if (task->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->parent->pid != 0) //20
                                                                                {
                                                                                    counter++;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return counter;
}
void dfs(struct task_struct *task)
{
    struct task_struct *child;
    struct list_head *list;
    int i;
    char tem[100];
    char *space = "    ";

    memset(tem, 0, sizeof(tem));

    int counter = find_level(task);

    for (i = 0; i < counter; i++)
    {
        strcat(msg, space);
    }
    sprintf(tem, "%s(%d)\n", task->comm, task->pid);
    strcat(msg, tem);
    list_for_each(list, &task->children)
    {
        child = list_entry(list, struct task_struct, sibling);
        dfs(child);
    }
}
void find_sibling(struct task_struct *task)
{
    struct task_struct *child;
    struct list_head *list2;
    char tem1[100];
    list_for_each(list2, &task->parent->children)
    {
        child = list_entry(list2, struct task_struct, sibling);
        if (child->pid != task->pid)
        {
            sprintf(tem1, "%s(%d)\n", child->comm, child->pid);
            strcat(msg, tem1);
        }
    }
}

void find_ancestor(struct task_struct *task)
{

    if (task->parent->pid == 0) //|| task->parent->pid == 2
    {
        char tem1[100];
        sprintf(tem1, "%s(%d)\n", task->comm, task->pid);
        strcat(msg, tem1);
    }
    else
    {

        find_ancestor(task->parent);
        char tem[100];
        char *space = "    ";
        int m = 0;
        int level = find_level(task);
        printk(KERN_INFO "%s(%d)", task->comm, task->pid);
        for (m = 0; m < level; m++)
        {
            strcat(msg, space);
        }
        sprintf(tem, "%s(%d)\n", task->comm, task->pid);
        strcat(msg, tem);
    }
}

void print_ancestor(struct task_struct *task)
{
    struct task_struct *test;

    find_ancestor(task);
}

void sendnlmsg(int pid)
{
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh;
    int msg_size;
    int res;

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
    memset(msg, 0, sizeof(msg));
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid; /*pid of sending process */

    //dfs(&init_task); //For entire pstree

    /*//For specific pid pstree
    struct task_struct *det_pid;
    pid_t dpid = 1111;
    det_pid = pid_task(find_vpid(dpid), PIDTYPE_PID);
    dfs(det_pid);
    */
    //struct task_struct;

    //For sibling //test pid :1562Xorg(1562) dbus-daemon(1570) gnome-session-b(1576)
    // list for each

    //struct task_struct *det_pid;
    // pid_t dpid = 1810;
    //det_pid = pid_task(find_vpid(dpid), PIDTYPE_PID);
    //find_sibling(det_pid);

    struct task_struct *det_pid;
    pid_t dpid = task_pid_nr(current);
    det_pid = pid_task(find_vpid(dpid), PIDTYPE_PID);
    print_ancestor(det_pid);

    sendnlmsg(pid);
}

static int __init hello_init(void)
{

    printk("Entering: %s\n", __FUNCTION__);
    //msg = kmalloc(1024, GFP_KERNEL);
    //msg[0] = '\0';
    //memset(msg, 0, sizeof(msg));
    //dfs(&init_task);
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
