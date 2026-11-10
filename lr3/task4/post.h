#ifndef POST_H
#define POST_H

#include "mystring.h"

/* Structs */
typedef struct {
    String *city;
    String *street;
    int house;
    String *building;
    int apartment;
    String *postalCode;
} Address;

typedef struct {
    Address *address;
    float weight;
    String *id;
    String *arriveTime;
    String *deliveryTime;
} Mail;

typedef struct {
    Address *address;
    Mail **mail;
    int count;
    int cap;
} Post;

/* Init */
/* Можно переделать и сделать лучше и реализовать */
/* паттерн Builder: AddressBuilder, MailBuilder   */
status_t newAddress(
    String *city,
    String *street,
    int     house,
    String *building,
    int     apartment,
    String *postalCode,
    Address **out
);
status_t newMail(
    Address *address,
    float weight,
    String *id,
    String *arriveTime,
    String *deliveryTime,
    Mail **out
);
status_t newPost(Address *adr, Post **out);

/* Free */
void freeAddress(Address *adr);
void    freeMail(Mail *m);
void    freePost(Post *p);

/* Mail */
int compareBySiteMail(const void *a, const void *b);
int compareByTimeMail(const void *a, const void *b);

/* Post */
status_t     addMail(Post *p, Mail *m);
status_t    findById(Post *p, String *id, Mail **out);
status_t  removeMail(Post *p, Mail *m);
status_t getMailList(
    Post *p, 
    String *time, 
    Mail ***out,
    int *size
);

/* String */
status_t toLineMail(Mail *m, char *out, int size);
status_t toFullMail(Mail *m, char *out, int size);

#endif