#ifndef _MYLIST_H
#define _MYLIST_H


struct list_head{
	struct list_head *prev,  *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name)}

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)


static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->prev = list;
	list->next = list;
}

static inline void __list_add(struct list_head *new,
			struct list_head *prev,
			struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{

	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	
	__list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *list)
{
	
	__list_del(list->prev, list->next);
}

static inline int list_empty(const struct list_head *head)
{
	
	return head->next == head;
}

#define offsetof(type, member) ((size_t) &((type *)0)->member)

#define container_of(ptr, type, member) ( {\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);\
	(type *)( (char *)__mptr - offsetof(type,member) );})


/*
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
*/



#define list_entry(ptr, type, member)\
	container_of(ptr, type, member)



/**
 *list_first_entry - get the first element from a list
 *@ptr:	the list head to take the element from.
 *@type:	the type of the struct this is embedded in.
 *@member:	the name of the list_struct within the struct.       
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member)\
	list_entry((ptr)->next, type, member)


#define list_first_or_null(ptr, type, member)\
	(!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)

#define list_for_each(pos, head)\
	for( pos = (head)->next; pos != head; pos = pos->next)

#define list_for_each_entry(pos, head, member)\
	for ( pos = list_entry((head)->next, typeof(*pos), member);\
	&pos->member != (head);\
	pos = list_entry(pos->member.next, typeof(*pos) , member))

#endif


