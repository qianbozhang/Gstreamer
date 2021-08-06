#include<iostream>
#include<string.h>

using namespace std;

#define MAX_ROUTE       2
#define MAX_THUMBNAIL  12

#define my_printf(fmt, ...) printf("\t%s  %s()::%d " fmt , __TIME__,  __func__, __LINE__, ##__VA_ARGS__)

/*
------------------------------------------------------
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
-----------------------------------------------------
0~ 1:allow to work
2~11:meybe invalid or idle
insert a array:
a.if 11 is idle,means the tQueue is full.  we should go to move forward by a value[2 ~ 11],then insert value at 11
b.if 11 is invalid,means the tQueue is not full, we insert the value after the last no invalid value.

delete a working:
a.delete 0:It starts at 1, so the values all move forward once
b.delete 1:It starts at 2, so the values all move forward once

clear:reset tQueue
*/

// enum array value state
typedef enum
{
    VALUE_INVALID = -1,     /**无效值，占位作用*/
    VALUE_IDLE,            /**刚刚入队的值，pending*/
    VALUE_WORKING          /**正在处理的值*/
}Value_State_e;

typedef struct _ArrayValue
{
    string URI;            /**thumbnail uri*/
    int MEDIAPLAYERID;    /**shadow id*/
    Value_State_e STATE;  /**thumbnail state*/

    void reset()
    {
        this->URI = " ";
        this->MEDIAPLAYERID = -1;
        this->STATE = VALUE_INVALID;
    }
}ArrayValue_s;

string convertStateToString(Value_State_e state)
{
    switch (state)
    {
        case VALUE_INVALID:
        {
            return "invalid";
        }
        case VALUE_IDLE:
        {
            return "idle";
        }
        case VALUE_WORKING:
        {
            return "working";
        }
        default:
        {
            return "unknown";
        }
    }
}


class ArrayTest
{
public:
    ArrayTest();
    ~ArrayTest();

    //reset tqueue
    void initArray();
    
    //clear array
    void clearArray(bool force);

    //enqueue
    bool enArray(string uri, int mediaplayerid);

    //dequeue
    bool deArray(int mediaplayerid);

    //print tqueue
    void printQueue();

    //array
    ArrayValue_s tQueue[MAX_THUMBNAIL];

private:
    //move forward once in a range
    void moveForwardOnceFromPos(int pos);

    //tqueue condition
    int checkQueueCondition();
    //array
    //ArrayValue_s tQueue[MAX_THUMBNAIL];
};

ArrayTest::ArrayTest(/* args */)
{
    //reest
    initArray();
}

ArrayTest::~ArrayTest()
{
}

void ArrayTest::initArray()
{
    int value;
    my_printf("reset tQueue.\n");
    for( value = 0; value < MAX_THUMBNAIL; value++ )
    {
        tQueue[value].reset();
    }
}

void ArrayTest::clearArray(bool force = false)
{
    my_printf("clear %s element in the tQueue.\n", (force? "all" : "idle"));
    if(force){
        this->initArray();
    }
    else
    {
        for(int value = 0; value < MAX_THUMBNAIL; value++ )
        {
            if(tQueue[value].STATE == VALUE_IDLE)
            {
                my_printf("clear value uri:%s \n", tQueue[value].URI.c_str());
                tQueue[value].reset();
            }
        }
    }
}

//enqueue
bool ArrayTest::enArray(string uri, int mediaplayerid)
{
    bool result = true;
    my_printf("called mediaplayerid[%d], uri[%s]\n", mediaplayerid, uri.c_str() );
    //check ths tqueue if already full of elements
    int value = this->checkQueueCondition();
    if(value < 0)
    {
        my_printf("the tqueue is full, insert value to %d.\n", MAX_THUMBNAIL - 1 );
        //we think it is full
        //we need fonud the first idle value pos.
        int pos = 0;
        for(pos; pos < MAX_ROUTE + 1; pos++ )
        {
            if(tQueue[pos].STATE == VALUE_IDLE)
            {
                break;
            }
        }
        this->moveForwardOnceFromPos(pos);
        //insert to pos 11.
        tQueue[MAX_THUMBNAIL - 1].URI = uri;
        tQueue[MAX_THUMBNAIL - 1].MEDIAPLAYERID = mediaplayerid;
        tQueue[MAX_THUMBNAIL - 1].STATE = VALUE_IDLE;
    }
    else
    {
        my_printf("the tqueue is no full, insert value to %d.\n", value );
        //the queue is no full
        tQueue[value].URI = uri;
        tQueue[value].MEDIAPLAYERID = mediaplayerid;
        tQueue[value].STATE = VALUE_IDLE;
    }

    return result;
}

//dequeue a working.
bool ArrayTest::deArray(int mediaplayerid)
{
    bool result = true;
    int value =0;
    for(; value < MAX_ROUTE; value++)
    {
        if(tQueue[value].MEDIAPLAYERID == mediaplayerid)
        {
            break;
        }
    }
    this->moveForwardOnceFromPos(value);
    return result;
}

//print tqueue
void ArrayTest::printQueue()
{
    my_printf("print all value:\n");
    for(int value = 0; value < MAX_THUMBNAIL; value++ )
    {
        my_printf("index[%d]:mediaplayerid = %d, state = %s, uri = %s\n",
        value,
        tQueue[value].MEDIAPLAYERID,
        convertStateToString(tQueue[value].STATE).c_str(),
        tQueue[value].URI.c_str() );
    }
}

//tqueue condition
/*
*-2:error
*-1:the queue is full
*0~11:the queue is no full, return the first invalid value
*/
int ArrayTest::checkQueueCondition()
{
    //full
    if(tQueue[MAX_THUMBNAIL -1].STATE == VALUE_IDLE)
    {
        my_printf("the queue is full.\n");
        return -1;
    }

    //null
    if(tQueue[0].STATE == VALUE_INVALID)
    {
        my_printf("the queue is null.\n");
        return 0;
    }

    //found the first invalid value.
    for(int value = 1; value < MAX_THUMBNAIL; value++ )
    {
        if (tQueue[value].STATE == VALUE_INVALID)
        {
            my_printf("found the first invalid value[%d].\n", value);
            return value;
        }
    }
    //there are some error, print tQueue
    this->printQueue();

    return -2;
}

//move forward once in a range
void ArrayTest::moveForwardOnceFromPos(int pos = 0)
{
    my_printf("called, pos = %d\n", pos);
    if(pos < 0 || pos >= MAX_THUMBNAIL)
    {
        my_printf("invalid pos[%d].\n", pos);
        return;
    }
    int value = this->checkQueueCondition();
    //there is null.
    if(value == 0){
        return;
    }

    //there is full.
    if(value == -1)
    {
        for(int v = pos; v < MAX_THUMBNAIL - 1; v++)
        {
            tQueue[v] = tQueue[v+1];
        }
        tQueue[MAX_THUMBNAIL - 1].reset();
        return;
    }

    //there is no full
    if(value >= 0)
    {
        for(int v = pos; v < MAX_THUMBNAIL - 2; v++)
        {
            if(tQueue[v].STATE == VALUE_INVALID){
                break;
            }
            tQueue[v] = tQueue[v+1];
        }
        return;
    }

    my_printf("ERR:value = %d", value);
}



int main(int argc, char const *argv[])
{
    static int id = 0;
    int input = 0;
    ArrayTest at = ArrayTest();
    do
    {
        my_printf("************************************\n");
        my_printf("0.set pos 0 working.\n");
        my_printf("1.set pos 1 working.\n");
        my_printf("2.set pos 0 working done.\n");
        my_printf("3.set pos 1 working done.\n");
        my_printf("4.add item.\n");
        my_printf("5.stopthumbnail\n");
        my_printf("6.source out\n");
        my_printf("************************************\n");
        my_printf("please input a option:");
        cin>>input;

        switch (input)
        {
            case 0:
            {
                my_printf("set %s to working\n", at.tQueue[0].URI.c_str() );
                at.tQueue[0].STATE = VALUE_WORKING;
                break;
            }
            case 1:
            {
                my_printf("set %s to working\n", at.tQueue[1].URI.c_str() );
                at.tQueue[1].STATE = VALUE_WORKING;
                break;
            }
            case 2:
            {
                my_printf("set %s to working done\n", at.tQueue[0].URI.c_str() );
                at.deArray(at.tQueue[0].MEDIAPLAYERID);
                break;
            }
            case 3:
            {
                my_printf("set %s to working done\n", at.tQueue[1].URI.c_str() );
                at.deArray(at.tQueue[1].MEDIAPLAYERID);
                break;
            }
            case 4:
            {
                int num = 0;
                my_printf("please input num:");
                cin>>num;
                for(int i =0 ; i < num ; i++, id++ ){
                    string uri = "file:///" + to_string(id);
                    my_printf("add item id =%d, uri = %s\n", id, uri.c_str());

                    at.enArray(uri, id);
                }
                break;
            }
            case 5:
            {
                my_printf("stopthumbnail, need clear queue.\n");
                at.clearArray();
                break;
            }
            case 6:
            {
                my_printf("source out, need clear queue." );
                at.clearArray(true);
                break;
            }
            default:
                return 0;
        }

        at.printQueue();
    }while (1);
    
    return 0;
}
