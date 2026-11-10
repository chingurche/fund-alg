#include "post.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

bool isPostalCode(String *str) {
    if (!str) return false;
    if (str->length != 6+1) return false;
    for (int i = 0; i < 6; i++) {
        if (!isdigit((unsigned char)str->data[i])) 
            return false;
    }
    return true;
}

status_t newAddress(
    String *city,
    String *street,
    int house,
    String *building,
    int apartment,
    String *postalCode,
    Address **out) 
{
    if (!out) return STATUS_ERR_NULL_ARG;
    if (isEmptyString(city) || 
        isEmptyString(street) ||
        !isPostalCode(postalCode) ||
        house <= 0 ||
        apartment <= 0)
        return STATUS_ERR_INVALID;

    Address *adr = malloc(sizeof *adr);
    if (!adr) return STATUS_ERR_ALLOC;

    adr->city = city;
    adr->street = street;
    adr->house = house;
    adr->building = building;
    adr->apartment = apartment;
    adr->postalCode = postalCode;

    *out = adr;
    return STATUS_OK;
}

bool isId(String *str) {
    if (!str) return false;
    if (str->length != 14+1) return false;
    for (int i = 0; i < 14; i++) {
        if (!isdigit((unsigned char)str->data[i])) 
            return false;
    }
    return true;
}

bool isTime(String *str) {
    if (!str) return false;
    int v[6];
    if (sscanf(
            str->data, 
            "%d:%d:%d %d:%d:%d",
            &v[0], &v[1], &v[2],
            &v[3], &v[4], &v[5]
        ) != 6)
        return false;
    if (v[0] < 1 || v[0] > 31) return false;
    if (v[1] < 1 || v[1] > 12) return false;
    if (v[2] < 1000 || v[2] > 9999) return false;
    if (v[3] < 0 || v[3] > 23) return false;
    if (v[4] < 0 || v[4] > 59) return false;
    if (v[5] < 0 || v[5] > 59) return false;
    
    return true;
}

status_t newMail(
    Address *address,
    float weight,
    String *id,
    String *arriveTime,
    String *deliveryTime,
    Mail **out)
{
    if (!out) return STATUS_ERR_NULL_ARG;
    if (!address || 
        weight < 0 ||
        !isId(id) ||
        !isTime(arriveTime) ||
        !isTime(deliveryTime))
        return STATUS_ERR_INVALID;
    
    Mail *mail = malloc(sizeof *mail);
    if (!mail) return STATUS_ERR_ALLOC;

    mail->address = address;
    mail->weight = weight;
    mail->id = id;
    mail->arriveTime = arriveTime;
    mail->deliveryTime = deliveryTime;

    *out = mail;
    return STATUS_OK;
}

status_t newPost(Address *adr, Post **out) {
    if (!adr || !out) return STATUS_ERR_NULL_ARG;
    
    Post *post = malloc(sizeof *post);
    if (!post) return STATUS_ERR_ALLOC;

    post->count = 0;
    static const int startcap = 16;
    post->cap = startcap;

    post->mail = calloc(
        startcap, sizeof(*(post->mail))
    );
    if (!post->mail) {
        free(post);
        return STATUS_ERR_ALLOC;
    }

    *out = post;
    return STATUS_OK;
}

void freeAddress(Address *adr) {
    freeString(adr->city);
    freeString(adr->street);
    adr->house = 0;
    freeString(adr->building);
    adr->apartment = 0;
    freeString(adr->postalCode);
    free(adr);
}

void freeMail(Mail *m) {
    freeAddress(m->address);
    m->weight = 0;
    freeString(m->id);
    freeString(m->arriveTime);
    freeString(m->deliveryTime);
    free(m);
}

void freePost(Post *p) {
    freeAddress(p->address);
    for (int i = 0; i < p->count; i++)
        freeMail(p->mail[i]);
    free(p->mail);
    p->count = 0;
    p->cap = 0;
    free(p);
}

int compareBySiteMail(const void *a, const void *b) {
    const Mail *m1 = *(const Mail **)a;
    const Mail *m2 = *(const Mail **)b;

    int pc = compareString(
        m1->address->postalCode, 
        m2->address->postalCode
    );
    if (pc != 0) return pc;

    int id = compareString(m1->id, m2->id);
    return id;
}

int compareByTimeMail(const void *a, const void *b) {
    const Mail *m1 = *(const Mail **)a;
    const Mail *m2 = *(const Mail **)b;

    int d1, mth1, y1, h1, min1, s1;
    int d2, mth2, y2, h2, min2, s2;

    sscanf(
        m1->arriveTime->data, "%d:%d:%d %d:%d:%d", 
        &d1, &mth1, &y1, &h1, &min1, &s1);
    sscanf(
        m2->arriveTime->data, "%d:%d:%d %d:%d:%d", 
        &d2, &mth2, &y2, &h2, &min2, &s2);

    if (y1 != y2) return y1 - y2;
    if (mth1 != mth2) return mth1 - mth2;
    if (d1 != d2) return d1 - d2;
    if (h1 != h2) return h1 - h2;
    if (min1 != min2) return min1 - min2;

    return s1 - s2;
}

status_t addMail(Post *p, Mail *m) {
    if (!p || !m) return STATUS_ERR_NULL_ARG;

    for (int i = 0; i < p->count; i++)
        if (equalString(p->mail[i]->id, m->id))
            return STATUS_ERR_ALR_EXIST;

    if (p->count + 1 > p->cap) {
        Mail **tmp = realloc(
            p->mail, 
            p->cap * 2 * sizeof(*tmp)
        );
        if (!tmp) return STATUS_ERR_ALLOC;

        p->cap *= 2;
        free(p->mail);
        p->mail = tmp;
    }

    int i = p->count - 1;
    while (i >= 0 && compareBySiteMail(&m, &p->mail[i]) < 0) {
        p->mail[i + 1] = p->mail[i];
        i--;
    }
    p->mail[i + 1] = m;
    p->count++;

    return STATUS_OK;
}

status_t findById(Post *p, String *id, Mail **out) {
    if (!p || !id || !out) 
        return STATUS_ERR_NULL_ARG;

    for (int i = 0; i < p->count; i++) {
        if (equalString(p->mail[i]->id, id)) {
            *out = p->mail[i];
            return STATUS_OK;
        }
    }
    return STATUS_ERR_NOT_FOUND;
}

status_t removeMail(Post *p, Mail *m) {
    if (!p || !m) return STATUS_ERR_NULL_ARG;

    int founded = -1;
    for (int i = 0; i < p->count; i++) {
        if (m == p->mail[i]) {
            founded = i;
            break;
        }
    }
    if (founded == -1) return STATUS_ERR_NOT_FOUND;

    
    freeMail(m);
    for (int i = founded; i < p->count - 1; i++) {
        p->mail[i] = p->mail[i+1];
    }
    p->count--; // мусорное значение просто остается

    return STATUS_OK;
}

bool isTimeExpired(String *deliveryTime, const int target[6]) {
    int d, mth, y, h, min, s;
    if (sscanf(deliveryTime->data, "%d:%d:%d %d:%d:%d", 
               &d, &mth, &y, &h, &min, &s) != 6) {
        return false;
    }

    if (y != target[2])     return y < target[2];
    if (mth != target[1])   return mth < target[1];
    if (d != target[0])     return d < target[0];
    if (h != target[3])     return h < target[3];
    if (min != target[4])   return min < target[4];
    
    return s <= target[5];
}

status_t getMailList(
    Post *p, 
    String *time, // опционально
    Mail ***out,
    int *size) 
{
    if (!p || !out || !size) 
        return STATUS_ERR_NULL_ARG;

    *out = NULL;
    *size = 0;

    int target[6] = {0};
    
    if (time != NULL) {
        if (sscanf(time->data, "%d:%d:%d %d:%d:%d",
                   &target[0], &target[1], &target[2],
                   &target[3], &target[4], &target[5]) != 6) {
            return STATUS_ERR_INVALID;
        }
    }

    int count = 0;
    for (int i = 0; i < p->count; i++) {
        if (time == NULL || 
            isTimeExpired(p->mail[i]->deliveryTime, target))
            count++;
    }

    if (count == 0) {
        return STATUS_OK;
    }

    Mail **result = malloc(count * sizeof(Mail *));
    if (!result) {
        return STATUS_ERR_ALLOC;
    }

    int idx = 0;
    for (int i = 0; i < p->count; i++) {
        if (time == NULL || 
            isTimeExpired(p->mail[i]->deliveryTime, target))
            result[idx++] = p->mail[i];
    }

    qsort(result, count, sizeof(Mail *), compareByTimeMail);

    *out = result;
    *size = count;

    return STATUS_OK;
}


status_t toLineMail(Mail *m, char *out, int size) {
    if (!m || !out) return STATUS_ERR_NULL_ARG;
    int len = snprintf(
        out, 
        size,
        "%14s: %19s %19s %7.3fкг\n",
        m->id->data,
        m->arriveTime->data,
        m->deliveryTime->data,
        m->weight
    );
    if (len >= size) return STATUS_ERR_OVERFLOW;

    return STATUS_OK;
}

status_t toFullMail(Mail *m, char *out, int size) {
    if (!m || !out) return STATUS_ERR_NULL_ARG;
    int len = snprintf(
        out, 
        size, 
        "\nАдрес получения\n"
        "Город:           %s\n"
        "Улица:           %s\n"
        "Номер дома:      %d\n"
        "Корпус:          %s\n"
        "Номер квартиры:  %d\n"
        "Индекс:          %s\n"
        "\nДанные посылки\n"
        "Вес посылки:     %fкг\n"
        "Идентификатор:   %s\n"
        "Время создания:  %s\n"
        "Время получения: %s\n", 
        m->address->city->data, 
        m->address->street->data, 
        m->address->house, 
        isEmptyString(m->address->building) ? 
        m->address->building->data :
        "нет информации", 
        m->address->apartment, 
        m->address->postalCode->data,
        m->weight,
        m->id->data,
        m->arriveTime->data,
        m->deliveryTime->data
    );
    if (len >= size) return STATUS_ERR_OVERFLOW;

    return STATUS_OK;
}