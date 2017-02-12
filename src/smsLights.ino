#include "ledstrip.h"
#include "textMessage.h"

void setup()
{

    Cellular.on();
    Cellular.connect();
    while(!Cellular.ready());
    Cellular.command("AT+CMGF=1\r\n");      //Set text mode
    deleteAllSMS();

    ledStripInit();

}


void loop()
{

    char message[MAX_SMS_LENGTH];
    char phoneNum[MAX_PHONE_NUM_LENGTH];

//When new sms received, pass to LED strip module

    if(checkNewSMS(phoneNum, message))
    {
      (void)ledStripNewMessage(message);
    }


}


bool checkNewSMS(char * number, char* message)
{
    int index;

    index = getNextSMSIndex();

    if(index == -1)
    {
        return FALSE;
    }

    return readSMS(index, number, message);

}

int getNextSMSIndex(void)
{

    NEXT_SMS nextSMS;

    nextSMS.rec = FALSE;

    //Callback function is called for each unread text message
    if(Cellular.command(CMGLCallback, &nextSMS, "AT+CMGL=\"REC UNREAD\"\r\n") == RESP_OK)
    {
        if(nextSMS.rec)
        {
            return nextSMS.index;
        }

    }

    return -1;
}

bool readSMS(int index, char * phoneNumber, char * message)
{
    READ_SMS readSMS;
    bool commandResponse;

    readSMS.gotNumber = FALSE;
    readSMS.phoneNumber = phoneNumber;
    readSMS.gotMessage = FALSE;
    readSMS.message = message;

    commandResponse = Cellular.command(CMGRCallback, &readSMS, "AT+CMGR=%d\r\n", index);

    return (readSMS.gotNumber && readSMS.gotMessage);
}

//Callback function only takes first unread SMS
int CMGLCallback(int type, const char* buf, int len, NEXT_SMS* nextSMS)
{

    int i;
    int indexFound;


    if(!nextSMS->rec && type== TYPE_PLUS && nextSMS)
    {
        if(sscanf(buf, "\r\n+CMGL: %d,", &i)==1)
        {
            nextSMS->rec = TRUE;
            nextSMS->index = i;
        }
    }

    return WAIT;

}

//Read SMS contents and sender from message
int CMGRCallback(int type, const char* buf, int len, READ_SMS* readSMS)
{

    if(readSMS)
    {
        if(type == TYPE_PLUS)
        {
            if(sscanf(buf, "\r\n+CMGR: \"%*[^\"]\",\"%[^\"]", readSMS->phoneNumber) == 1)
            {
                readSMS->gotNumber = TRUE;
            }
        }
        else if ((type == TYPE_UNKNOWN) && (buf[len-2] == '\r') && (buf[len-1] == '\n'))
        {
            memcpy(readSMS->message, buf, len-2);
            readSMS->message[len-2] = '\0';
            readSMS->gotMessage = TRUE;
        }
    }

    return WAIT;

}


void deleteSMS(int smsNum)
{
    Cellular.command("AT+CMGD=%d\r\n", smsNum);
}

void deleteAllSMS(void)
{
    Cellular.command("AT+CMGD=1,4\r\n");
}
