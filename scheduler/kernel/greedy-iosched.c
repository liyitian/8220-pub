/*
 * elevator greedy
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

// modified from no-op to add additional queue and prev. position
struct greedy_data {
	struct list_head higher;
    struct list_head lower;
    sector_t prev_pos;
};

// i think this is okay?
static void greedy_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

// TODO: this is where we must set prev_pos i think, line below maybe helpful
static int greedy_dispatch(struct request_queue *q, int force)
{
    int up_dist;
    int down_dist;
    struct request* rq;
	struct greedy_data *gd = q->elevator->elevator_data;    
    // guess upper as target list
    struct list_head target = gd->upper;
    if (!list_empty(&gd->upper))
    {
        // upper exists, does lower?
        if (!list_empty(&gd->lower))
        {
            up_dist = blk_rq_pos(gd->higher) - gd->prev_pos;
            down_dist = gd->prev_pos - blk_rq_pos(gd->lower);
            // only if lower is farther away do we change rq
            if (up_dist > down_dist)
            {
                target = gd->lower;
            }
        }
        // in any but innermost case, we are head
    }
    // no upper, so if lower exists we choose it
    else if (!list_empty(&gd->lower))
    {
        target = gd->lower;
    }
    else
    {
        return 0;
    }
    struct request* rq = list_first_entry(target, struct request, queuelist);
    list_del_init(&rq->queuelist);
    //FIXME: need queue lock: do we already have it?
    // Add to tail of list, so that we know where we stand
    elv_dispatch_add_tail(q, rq);
    gd->prev_pos = blk_rq_pos(rq) + blk_rq_sectors(rq)
    return 1;
}


static void greedy_add_request(struct request_queue *q, struct request *rq)
{
	struct greedy_data *gd = q->elevator->elevator_data;
    // check if we know prev_position, and if lower. Otherwise default to 
    // upper list
    if (gd->prev_pos && gd->prev_pos > blk_rq_pos(rq))
    {
        add_to_lower(rq);   
    }
    // no prev position, or position is higher
    else
    {
        add_to_higher(rq);
    }

}

// adds to upper list
static void add_to_higher(struct request* rq)
{
	struct greedy_data *gd = q->elevator->elevator_data;
    struct list_head* pos;
    list_for_each(pos, gd->higher)
    {
        // smaller now, so take this guy's spot
        if (blk_rq_pos(pos) > blk_rq_pos(rq))
        {
            break;    
        }
    }
    // if list empty, pos == head so same result
	list_add(&rq->queuelist, &gd->pos);
}

// adds to lower list
static void add_to_lower(struct request* rq)
{
	struct greedy_data *gd = q->elevator->elevator_data;
    struct list_head* pos;
    list_for_each(pos, gd->lower)
    {
        // we are now bigger than the thing below us, so fit here
        if (blk_rq_pos(pos) < blk_rq_pos(rq))
        {
            break;    
        }
    }
    // should be list_add to take place of current node
    // if list empty, pos == head so same result
	list_add(&rq->queuelist, &gd->pos);
}

// TODO: idk if we need to implement this or other 'helpful' elv functions
/*
static struct request *
noop_former_request(struct request_queue *q, struct request *rq)
{
	struct noop_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
noop_latter_request(struct request_queue *q, struct request *rq)
{
	struct noop_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}
*/

// I think this is it?
static int greedy_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct greedy_data *gd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	gd = kmalloc_node(sizeof(*gd), GFP_KERNEL, q->node);
	if (!gd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = gd;

	INIT_LIST_HEAD(&gd->higher);
	INIT_LIST_HEAD(&gd->lower);
    gd->prev_pos = 0;

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

// I think this okay
static void greedy_exit_queue(struct elevator_queue *e)
{
	struct greedy_data *gd = e->elevator_data;

	BUG_ON(!list_empty(&gd->higher));
	BUG_ON(!list_empty(&gd->lower));
	kfree(gd);
}

static struct elevator_type elevator_greedy = {
	.ops = {
		.elevator_merge_req_fn		= greedy_merged_requests,
		.elevator_dispatch_fn		= greedy_dispatch,
		.elevator_add_req_fn		= greedy_add_request,
		//.elevator_former_req_fn		= noop_former_request,
		//.elevator_latter_req_fn		= noop_latter_request,
		.elevator_init_fn		= greedy_init_queue,
		.elevator_exit_fn		= greedy_exit_queue,
	},
	.elevator_name = "greedy",
	.elevator_owner = THIS_MODULE,
};

static int __init greedy_init(void)
{
	return elv_register(&elevator_greedy);
}

static void __exit greedy_exit(void)
{
	elv_unregister(&elevator_greedy);
}

module_init(greedy_init);
module_exit(greedy_exit);


MODULE_AUTHOR("TA");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Greedy IO scheduler");