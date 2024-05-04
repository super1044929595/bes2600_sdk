#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct _xOS_List_t{
	const char* name;
	int         id;
	struct _xOS_List_t* pre;
	struct _xOS_List_t* next;
}xOS_List_t;
xOS_List_t xos_ListHead = { NULL,-999,NULL,NULL };

int xOS_ListInsert(int id,const char* name)
{
	xOS_List_t *_templist = xos_ListHead.next;
	xOS_List_t* _templistpre = _templist;
	if (_templist != NULL) {
		do {
			_templistpre = _templist;
			_templist = _templist->next;
		} while (_templist!=NULL);
	}

	_templist =(xOS_List_t*)malloc(sizeof(xOS_List_t));
	if (_templist == NULL) {
		printf("xOS_ListInsert malloc error");
		return 1;
	}
	if (xos_ListHead.next == NULL) {
		xos_ListHead.next = _templist;
	}
	if (_templistpre != NULL) {
		_templistpre->next = _templist;
	}
	_templist->id = id;
	_templist->name = (char*)malloc(strlen(name));
	_templist->next = NULL;
	memcpy((char*)_templist->name,name, strlen(name));
	if (_templist->name==NULL) {
		printf("xOS_ListInsert malloc error");
	}
	printf("\r\n  +++++++++++++++++++++++++++++++++++++++++ ");
	printf("\r\n    xOS_ListInsert [ID: %d],[name:%s]", id, name);
	printf("\r\n  +++++++++++++++++++++++++++++++++++++++++ \n");
	return 0;
}

int xOS_ListDelete(int id ,const char *name)
{
	xOS_List_t* _templist = xos_ListHead.next;
	xOS_List_t* _templistpre = &xos_ListHead;
	while (_templist)
	{
		if( _templist->id == id ) {
			_templistpre->next = _templist->next;
			printf("\r\n  +---------------------------------------+ ");
			printf("\r\n	xOS_ListDelete --------> [ID: %d]", id);
			printf("\r\n  +---------------------------------------+ \n");
			free(_templist);
			break;
		}
		if (_templist) {
			if ((name != NULL) && (strcmp(_templist->name, name) == 0)){	
				_templistpre->next = _templist->next;
				printf("\r\n  +---------------------------------------+ ");
				printf("\r\n	xOS_ListDelete --------> [name: %s]",name);
				printf("\r\n  +---------------------------------------+ ");
				free(_templist);
				break;
			}
		}
		_templistpre = _templist;
		_templist = _templist->next;
	}
	return 0;
}

int xOS_ListSearch(int id, const char* name)
{
	char _record = 0;
	xOS_List_t* _templist = xos_ListHead.next;
	while (_templist!=NULL) {

		if ( _templist->id == id ) {
			   printf("\r\n xOS_ListSearch --------> [ID: %d],[name:%s] \n",id,name);
			   _record = 1;
			   break;
		}
		if (name != NULL) {
			if (strcmp(_templist->name, name) == 0)
			{
				printf("\r\n xOS_ListSearch --------> [ID: %d],[name:%s] \n", id, name);
				_record = 1;
				break;
			}
		}
	
		_templist = _templist->next;
	}
	if (_record == 0) {
		printf("\r\n xOS_ListSearch --------> no id");
	}
	return 0;
}

int main() {
	xOS_ListInsert(0, "first");
	xOS_ListInsert(1, "second");
	xOS_ListInsert(2, "third");
	xOS_ListSearch(1, NULL);
	xOS_ListSearch(0, "third");

	xOS_ListDelete(0,NULL);
	xOS_ListSearch(1, NULL);
	xOS_ListSearch(0, NULL);
	printf("\r\n\n\n");
}