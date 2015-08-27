#include <stdio.h>

#define offset(TYPE, MEMBER)  ( (size_t)  &(((TYPE*)0)->MEMBER))
#define containerOf(ptr, type, member) ({ \
                const typeof(((type*)0)->member) *__mptr = (ptr); \
                (type*)((char*)__mptr - offset(type, member)); })
#define listEntry(ptr, type, member) \
    containerOf(ptr, type, member)

#define list_for_each(pos, head) \
    for(pos = (head)->prev; pos != (head); pos = pos->prev)
struct ListNode {
    struct ListNode* prev;
    struct ListNode* next;
};


static inline void initListNode(struct ListNode *head){
    head->prev = head;
    head->next = head;
}

static inline void addTailListNode(struct ListNode *head, struct ListNode *item){
    item->next  = head;
    item->prev  = head->prev;
    head->prev->next = item;
    head->prev = item;

};

static inline void addHeadListNode(struct ListNode *head, struct ListNode *item){
    item->next = head->next;
    item->prev = head->prev;
    head->next->prev = item;
    head->next = item;

};

static inline void removeListNode(struct ListNode* item){
    item->prev->next = item->next;
    item->next->prev = item->prev;
};

void dump(const char* data, int length){
    printf("\n");
    int i = 0;
    for(i = 0; i < length; i++){
        printf("%02x", (data[i] & 0xFF));
        if((i + 1) % 2 == 0) printf(" ");
        if((i + 1) % 16 == 0)printf("\n");
    }
    printf("\n");
}
