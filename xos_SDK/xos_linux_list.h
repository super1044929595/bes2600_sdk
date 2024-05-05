#ifndef __XOS_LINUX_LIST_H__
#define __XOS_LINUX_LIST_H__

#ifdef __cplusplus
	extern "C"{
#endif
	
	// 双向链表节点
	struct list_head {
		struct list_head *next, *prev;
	};
	
	// 初始化节点：设置name节点的前继节点和后继节点都是指向name本身。
#define LIST_HEAD_INIT(name) { &(name), &(name) }
	
	// 定义表头(节点)：新建双向链表表头name，并设置name的前继节点和后继节点都是指向name本身。
#define LIST_HEAD(name) \
		struct list_head name = LIST_HEAD_INIT(name)
	

	
	// 获取"MEMBER成员"在"结构体TYPE"中的位置偏移
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
	
	// 根据"结构体(type)变量"中的"域成员变量(member)的指针(ptr)"来获取指向整个结构体变量的指针
#define container_of(ptr, type, member) ({          \
		const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
		(type *)( (char *)__mptr - offsetof(type,member) );})
	
	// 遍历双向链表
#define list_for_each(pos, head) \
		for (pos = (head)->next; pos != (head); pos = pos->next)
	
#define list_for_each_safe(pos, n, head) \
		for (pos = (head)->next, n = pos->next; pos != (head); \
			pos = n, n = pos->next)
	
#define list_entry(ptr, type, member) \
		container_of(ptr, type, member)
	
#ifdef __cplusplus
}
#endif
#endif
