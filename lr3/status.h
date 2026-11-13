#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_OK = 0,
	STATUS_ERR_NULL_ARG,
    STATUS_ERR_ALLOC,
    STATUS_ERR_RANGE,
    STATUS_ERR_INVALID,
	STATUS_ERR_NOT_FOUND,
	STATUS_ERR_ALR_EXIST,
    STATUS_ERR_OVERFLOW,
    STATUS_ERR_DIV_ZERO,
    STATUS_ERR_OPEN,
    STATUS_ERR_READ,
    STATUS_ERR_WRITE,
    STATUS_ERR_CLOSE,
    STATUS_ERR_PARSE,
    STATUS_ERR_PATH  
} status_t;

#define MSG_OPEN        "Не удалось открыть.\n"  
#define MSG_CLOSE       "Не удалось закрыть.\n"  
#define MSG_WRITE       "Не удалось записать.\n"  
#define MSG_READ        "Не удалось прочитать.\n"  
#define MSG_PATH        "Недействительный путь.\n" 
#define MSG_INVALID     "Недействительное значение.\n" 
#define MSG_PARSE       "Недействительный формат парсирующихся данных.\n"  
#define MSG_DIV_ZERO    "Деление на ноль.\n"  
#define MSG_NOT_FOUND   "Значение не найдено.\n" 
#define MSG_ALR_EXIST   "Значение уже существует.\n" 
#define MSG_ALLOC       "Не удалось выделить память.\n" 
#define MSG_RANGE       "Значение вышло за пределы допустимого диапазона.\n" 
#define MSG_OVERFLOW    "Произошло переполнение.\n"  
#define MSG_NULL_ARG    "Внутренняя ошибка.\n" 
#define MSG_UNKNOWN     "Неизвестная ошибка.\n" 


#endif