#ifndef STOCK_ANALYZER_H
#define STOCK_ANALYZER_H


#define APIKEY "4Z81PYCE9QHTYYBF"
#define CURRENT "GLOBAL_QUOTE"
#define HISTORIC "TIME_SERIES_DAILY"

// stockdata struct to contain a days info
typedef struct {
    char date[11]; // YYYY-MM-DD
    float open, high, low, close; //stock info
    int volume; // volume traded
    struct {
        unsigned int valid: 1; // valid bit to define if StockData is valid
    } flags;
} StockData;

// linked list node/ day of data
typedef struct Node {
    StockData data; // data of the day
    struct Node* next; // next days data
} Node;


// functions

void append(Node** head, StockData data); // add new node to list
StockData parseData(char* line); // parse a day's data from CSV
void readCSV(const char* filename, Node** head); // reads a csv line
void analyze(Node* head, const char *startDateRange, const char *endDateRange, const char *symbol); // analyze data
void fetchData(const char* symbol, const char* function, const char* filename, int isDateRange); // fetch data from API
void getCurrentPrice(const char *symbol);
void getHistoricPrice(const char *symbol);
void getDateRangePrice(const char *symbol, const char *startDate, const char *endDate);

#endif
