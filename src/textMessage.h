#define MAX_SMS_LENGTH  160
#define MAX_PHONE_NUM_LENGTH 20

typedef struct
{
    bool rec;
    int index;

}NEXT_SMS;


typedef struct
{
    bool gotNumber;
    char * phoneNumber;
    bool gotMessage;
    char * message;

}READ_SMS;
