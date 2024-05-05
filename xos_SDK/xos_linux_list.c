#include "xos_linux_list.h"
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>


static void xos_ListTestDemo(void);

// 初始化节点：将list节点的前继节点和后继节点都是指向list本身。
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

// 添加节点：将new插入到prev和next之间。
static inline void __list_add(struct list_head *new,
				  struct list_head *prev,
				  struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

// 添加new节点：将new添加到head之后，是new称为head的后继节点。
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

// 添加new节点：将new添加到head之前，即将new添加到双链表的末尾。
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

// 从双链表中删除entry节点。
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

// 从双链表中删除entry节点。
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

// 从双链表中删除entry节点。
static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

// 从双链表中删除entry节点，并将entry节点的前继节点和后继节点都指向entry本身。
static inline void list_del_init(struct list_head *entry)
{
	__list_del_entry(entry);
	INIT_LIST_HEAD(entry);
}

// 用new节点取代old节点
static inline void list_replace(struct list_head *old,
				struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

// 双链表是否为空
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static void xos_ListTestDemo(void)
{
	struct person 
	{ 
		int age; 
		char name[20];
		struct list_head list; //注意是变量 不是指针
	};

	struct person *pperson; 
	struct person person_head; 
	struct list_head *pos, *next; 
	int i;

	// 初始化双链表的表头 
	INIT_LIST_HEAD(&person_head.list); 
	
	// 添加节点
	for (i=0; i<5; i++)
	{
		pperson = (struct person*)malloc(sizeof(struct person));
		pperson->age = (i+1)*10;
		sprintf(pperson->name, "%d", i+1);
		// 将节点链接到链表的末尾 
		// 如果想把节点链接到链表的表头后面，则使用 list_add
		list_add_tail(&(pperson->list), &(person_head.list));
	}

	// 遍历链表
	printf("==== 1st iterator d-link ====\n"); 
	list_for_each(pos, &person_head.list) 
	{ 
		pperson = list_entry(pos, struct person, list); 
		printf("name:%-2s, age:%d\n", pperson->name, pperson->age); 
	} 
	
	// 删除节点age为20的节点
	printf("==== delete node(age:20) ====\n");
	list_for_each_safe(pos, next, &person_head.list)
	{
		pperson = list_entry(pos, struct person, list);
		if(pperson->age == 20)
		{
			list_del_init(pos);
			free(pperson);
		}
	}

	// 再次遍历链表
	printf("==== 2nd iterator d-link ====\n");
	list_for_each(pos, &person_head.list)
	{
		pperson = list_entry(pos, struct person, list);
		printf("name:%-2s, age:%d\n", pperson->name, pperson->age);
	}
	
	// 释放资源
	list_for_each_safe(pos, next, &person_head.list)
	{
		pperson = list_entry(pos, struct person, list); 
		list_del_init(pos); 
		free(pperson); 
	}
		 
}

