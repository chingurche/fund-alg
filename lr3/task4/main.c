#include "post.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ERRSTRING "Произошла ошибка при инициализации String.\n"
#define ERRINT    "Произошла ошибка при парсинге int.\n"
#define ERRFLOAT  "Произошла ошибка при парсинге float.\n"

String *getCurrentTimeString(void) {
    time_t rawtime;
    struct tm *timeinfo;
    char buf[32];
    time(&rawtime);

    timeinfo = localtime(&rawtime);

    strftime(buf, sizeof(buf), "%d:%m:%Y %H:%M:%S", timeinfo);

    String *str = NULL;
    newString(buf, &str);
    return str;
}

void readConsole(char *buf, size_t size) {
    if (fgets(buf, size, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
    } else {
        buf[0] = '\0';
    }
}

void handleDialog(Post *post) {
    status_t rc;
    static const size_t MAXLINE = 32;
    
    char line[MAXLINE];
    while (true) {
        printf("\n");
        printf("(1) Добавить информацию о посылке\n");
        printf("(2) Найти информацию о посылке\n");
        printf("(3) Удалить информацию о посылке\n");
        printf("(4) Поиск всех доставленных отправлений\n");
        printf("(5) Поиск всех отправлений, срок доставки " 
            "которых на текущий момент времени истёк\n");
        printf("(0) Выход\n");
        fgets(line, sizeof(line), stdin);
        if (line[0] == '0' || line[1] == '\0') break;
        switch (line[0]) {
            case '1': {
                printf("\n----------Форма записи адреса получения----------\n");
                String *city = NULL;
                String *street = NULL;
                int house;
                String *building = NULL;
                int apartment;
                String *postalCode = NULL;

                printf("Укажите город (непустая строка): ");
                readConsole(line, MAXLINE);
                if (newString(line, &city)) { 
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                printf("Укажите улицу (непустая строка): ");
                readConsole(line, MAXLINE);
                if (newString(line, &street)) { 
                    freeString(city);
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                printf("Укажите номер дома (натуральное число): ");
                readConsole(line, MAXLINE);
                if (sscanf(line, "%d", &house) != 1) {
                    freeString(city);
                    freeString(street);
                    fprintf(stderr, ERRINT);
                    break;
                }

                printf("Укажите корпус (строка): "); 
                readConsole(line, MAXLINE);
                if (newString(line, &building)) { 
                    freeString(city);
                    freeString(street);
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                printf("Укажите номер квартиры (натуральное число): ");
                readConsole(line, MAXLINE);
                if (sscanf(line, "%d", &apartment) != 1) {
                    freeString(city);
                    freeString(street);
                    freeString(building);
                    fprintf(stderr, ERRINT);
                    break;
                }

                printf("Укажите индекс получателя (строка из шести символов цифр): ");
                readConsole(line, MAXLINE);
                if (newString(line, &postalCode)) {
                    freeString(city);
                    freeString(street);
                    freeString(building); 
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                Address *address = NULL;
                rc = newAddress(
                    city, 
                    street, 
                    house, 
                    building, 
                    apartment, 
                    postalCode,
                    &address
                );
                if (rc) {
                    fprintf(stderr, "Произошла ошибка при инициализации Address: ");
                    switch (rc) {
                        case STATUS_ERR_INVALID:
                            fprintf(stderr, "Недействительные входные данные.\n");
                            break;
                        case STATUS_ERR_ALLOC:
                            fprintf(stderr, "Не удалось выделить память.\n");
                            break;
                        case STATUS_ERR_NULL_ARG:
                            fprintf(stderr, "Внутренняя ошибка.\n");
                            break;
                        default:
                            fprintf(stderr, "Неизвестная ошибка.\n");
                            break;
                    }
                    freeString(city);
                    freeString(street);
                    freeString(building);
                    freeString(postalCode);
                    break;
                }


                printf("\n----------Форма записи данных посылки----------\n");
                float weight;
                String *id = NULL;
                String *arriveTime = NULL;
                String *deliveryTime = NULL;

                printf("Укажите вес (неотрицательное вещественное число): "); 
                readConsole(line, MAXLINE); 
                if (sscanf(line, "%f", &weight) != 1) {
                    freeAddress(address);
                    fprintf(stderr, ERRFLOAT);
                    break;
                }

                printf("Укажите идентификатор (строка из 14 символов цифр): ");
                readConsole(line, MAXLINE);
                if (newString(line, &id)) { 
                    freeAddress(address);
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }
                
                printf("Укажите время отправки (dd:MM:yyyy hh:mm:ss): ");
                readConsole(line, MAXLINE);
                if (newString(line, &arriveTime)) { 
                    freeAddress(address);
                    freeString(id);
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }
                
                printf("Укажите время вручения (dd:MM:yyyy hh:mm:ss): ");
                readConsole(line, MAXLINE);
                if (newString(line, &deliveryTime)) { 
                    freeAddress(address);
                    freeString(id);
                    freeString(arriveTime);
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                Mail *mail = NULL;
                rc = newMail(
                    address,
                    weight, 
                    id, 
                    arriveTime, 
                    deliveryTime,
                    &mail
                );
                if (rc) {
                    fprintf(stderr, "Произошла ошибка при инициализации Mail: ");
                    switch (rc) {
                        case STATUS_ERR_INVALID:
                            fprintf(stderr, "Недействительные входные данные.\n");
                            break;
                        case STATUS_ERR_ALLOC:
                            fprintf(stderr, "Не удалось выделить память.\n");
                            break;
                        case STATUS_ERR_NULL_ARG:
                            fprintf(stderr, "Внутренняя ошибка.\n");
                            break;
                        default:
                            fprintf(stderr, "Неизвестная ошибка.\n");
                            break;
                    }
                    freeAddress(address);
                    freeString(id);
                    freeString(arriveTime);
                    freeString(deliveryTime);
                    break;
                }

                rc = addMail(post, mail);
                if (rc) {
                    switch (rc) {
                        case STATUS_ERR_ALR_EXIST:
                            fprintf(stderr, "Посылка с таким идентификатором уже существует.\n");
                            break;
                        case STATUS_ERR_ALLOC:
                            fprintf(stderr, "Не удалось выделить память.\n");
                            break;
                        case STATUS_ERR_NULL_ARG:
                            fprintf(stderr, "Внутренняя ошибка.\n");
                            break;
                        default:
                            fprintf(stderr, "Неизвестная ошибка.\n");
                            break;
                    }
                    freeMail(mail);
                    break;
                }
                printf("Запись добавлена.\n");
                break;
            }
            case '2': {
                String *id = NULL;

                printf("Укажите идентификатор: "); 
                readConsole(line, MAXLINE);
                if (newString(line, &id)) { 
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                Mail *mail = NULL;
                rc = findById(post, id, &mail);
                freeString(id);
                switch (rc) {
                    case STATUS_OK: break;
                    case STATUS_ERR_NOT_FOUND:
                        fprintf(stderr, "Посылка с таким идентификатором не существует.\n");
                        break;
                    case STATUS_ERR_NULL_ARG:
                        fprintf(stderr, "Внутренняя ошибка.\n");
                        break;
                    default:
                        fprintf(stderr, "Неизвестная ошибка.\n");
                        break;
                }

                char buf[512];
                rc = toFullMail(mail, buf, sizeof buf);
                if (rc) { 
                    fprintf(stderr, "Произошла ошибка при сериализации Mail.\n");
                    break;
                }
                printf("%s", buf);
                break;
            }
            case '3': {
                String *id = NULL;

                printf("Укажите идентификатор: "); 
                readConsole(line, MAXLINE);
                if (newString(line, &id)) { 
                    fprintf(stderr, ERRSTRING); 
                    break; 
                }

                Mail *mail = NULL;
                rc = findById(post, id, &mail);
                freeString(id);
                if (rc) {
                    switch (rc) {
                        case STATUS_ERR_NOT_FOUND:
                            fprintf(stderr, "Посылка с таким идентификатором не существует.\n");
                            break;
                        case STATUS_ERR_NULL_ARG:
                            fprintf(stderr, "Внутренняя ошибка.\n");
                            break;
                        default:
                            fprintf(stderr, "Неизвестная ошибка.\n");
                            break;
                    }
                    break;
                }

                rc = removeMail(post, mail);
                if (rc) {
                    fprintf(stderr, "Это как это?\n");
                    break;
                }
                printf("Запись успешно удалена.\n");
                break;
            }
            case '4': {
                Mail **mail = NULL;
                int size;
                rc = getMailList(post, NULL, &mail, &size);
                if (rc) {
                    switch (rc) {
                        case STATUS_ERR_INVALID:
                            fprintf(stderr, "Недействительное время.\n");
                            break;
                        case STATUS_ERR_ALLOC:
                                fprintf(stderr, "Не удалось выделить память.\n");
                            break;
                        case STATUS_ERR_NULL_ARG:
                            fprintf(stderr, "Внутренняя ошибка.\n");
                            break;
                        default:
                            fprintf(stderr, "Неизвестная ошибка.\n");
                            break;
                    }
                    break;
                }
                if (size == 0) printf("Записей не найдено.\n");
                for (int i = 0; i < size; i++) {
                    char buf[128];
                    rc = toLineMail(mail[i], buf, sizeof buf);
                    if (rc) { 
                        free(mail);
                        fprintf(stderr, "Произошла ошибка при сериализации Mail.\n");
                        continue;
                    }
                    printf("%s", buf);
                }
                free(mail);
                break;
            }
            case '5': {
                Mail **mail = NULL;
                int size;
                String *now = getCurrentTimeString(); 
                rc = getMailList(post, now, &mail, &size);
                freeString(now);
                if (rc) {
                    switch (rc) {
                        case STATUS_ERR_INVALID:
                            fprintf(stderr, "Недействительное время.\n");
                            break;
                        case STATUS_ERR_ALLOC:
                                fprintf(stderr, "Не удалось выделить память.\n");
                            break;
                        case STATUS_ERR_NULL_ARG:
                            fprintf(stderr, "Внутренняя ошибка.\n");
                            break;
                        default:
                            fprintf(stderr, "Неизвестная ошибка.\n");
                            break;
                    }
                    break;
                }
                if (size == 0) printf("Записей не найдено.\n");
                for (int i = 0; i < size; i++) {
                    char buf[128];
                    rc = toLineMail(mail[i], buf, sizeof buf);
                    if (rc) { 
                        free(mail);
                        fprintf(stderr, "Произошла ошибка при сериализации Mail.\n");
                        continue;
                    }
                    printf("%s", buf);
                }
                free(mail);
                break;
            }
            default:{
                printf("Операции нет в списке.\n");
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    status_t rc;

    Address *postAddress = NULL;
    String *city = NULL;       newString("Химки", &city);
    String *street = NULL;     newString("Дружбы", &street);
    String *postalCode = NULL; newString("788812", &postalCode);
    newAddress(
        city, 
        street, 
        10, 
        NULL, 
        121, 
        postalCode,
        &postAddress
    );

    Post *post = NULL;
    rc = newPost(postAddress, &post);

    handleDialog(post);

    freePost(post);
}