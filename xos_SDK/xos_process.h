#ifndef __XOS_PROCESS_H__
#define __XOS_PROCESS_H__

#ifdef __cplusplus
	extern "C"{
#endif

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({            \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

struct list {
	int data;	/*有效数据*/
	struct list *next; /*指向下一个元素的指针*/
	struct list *prev; /*指向上一个元素的指针*/
};

	/*静态初始化*/
#define LIST_HEAD_INIT(name) { &(name), &(name) }
	
#define LIST_HEAD(name) \
		struct list_head name = LIST_HEAD_INIT(name)
		
#define list_for_each(pos, head) \
for (pos = (head)->next; pos != (head); pos = pos->next)
	/*动态初始化*/
	static inline void INIT_LIST_HEAD(struct list_head *list)
	{
		list->next = list;
		list->prev = list;
	}




#ifdef __cplusplus
	}
#endif

#endif
