#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h>
#include <ctype.h>
#include "stock_analyzer.h"


// add new node to linked list
void append(Node** head, StockData data){
    // allocate space for new node and assign its data to it
    Node* newNode = malloc(sizeof(Node));
    newNode -> data = data;
    newNode ->next = NULL;

    if( *head == NULL){
        // if list has no nodes, make new node the head
        *head = newNode;
    }
    else{
        // move to end of list and add in new node
        Node* current = *head;
        while(current -> next != NULL){
            current = current -> next; 
        }
        current -> next = newNode;
    }
}

// parses the csv line and returns StockData
// csv format date, open, high, low, close, volume
StockData parseData(char* line){
    StockData data;

    // split line by commas
    char* token = strtok(line, ",");

    // skips the first line of csv or invalid ones
    if (token == NULL || strcmp(token, "timestamp") == 0) {
        return data;
    }

    // parses data
    strncpy(data.date, token, sizeof(data.date));
    //ensure null terminator exists
    data.date[sizeof(data.date) - 1] = '\0';

    // get the rest of the data
    data.open = atof(strtok(NULL, ","));
    data.high = atof(strtok(NULL, ","));
    data.low = atof(strtok(NULL, ","));
    data.close = atof(strtok(NULL, ","));
    data.volume = atoi(strtok(NULL, ","));

    // set the data as valid
    data.flags.valid = 1;

    return data;
}


// reads csv line by line, adding each entry into the list
void readCSV(const char* filename, Node** head){
    FILE* file = fopen(filename, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];

    // read each line of the file
    while(fgets(line, sizeof(line), file)){
        // add each line of data to the list
        StockData data = parseData(line);
        // dont add if invalid
        if(data.flags.valid){
            append(head, data);
        }
    }

    //close file
    fclose(file);
}


// analyze all the data in the list
void analyze(Node* head, const char *startDateRange, const char *endDateRange, const char *symbol){
    if(head == NULL){
        printf("Error: No data in list\n");
        return;
    }

    float sum = 0;
    float min = -1;
    float max = -1;
    int counter = 0;
    char startDate[11] = "";
    char endDate[11] = "";

    Node* current = head;
    while(current != NULL){
        const char* date = current->data.date;

        // Only analyze if within range (inclusive)
        if(startDateRange != NULL && endDateRange != NULL){
            if(strcmp(date, startDateRange) > 0 || strcmp(date, endDateRange) < 0){
                current = current->next;
                continue;
            }
        }

        float close = current->data.close;
        sum += close;

        if(min < 0 || close < min) min = close;
        if(max < 0 || close > max) max = close;

        if(counter == 0) strncpy(startDate, date, sizeof(startDate));
        strncpy(endDate, date, sizeof(endDate));

        counter++;
        current = current->next;
    }

    if(counter == 0){
        printf("No data in specified date range.\n");
        return;
    }

    float average = sum / counter;

    // Calculate variance
    float variance = 0;
    current = head;
    while(current != NULL){
        const char* date = current->data.date;

        if(startDateRange != NULL && endDateRange != NULL){
            if(strcmp(date, startDateRange) > 0 || strcmp(date, endDateRange) < 0){
                current = current->next;
                continue;
            }
        }

        float diff = current->data.close - average;
        variance += diff * diff;

        current = current->next;
    }

    float stddev = sqrt(variance / counter);

    // Results
    printf("\n============= %s %d Day Analysis =============\n", symbol ,counter);
    printf("Moving Average: $%.2f\n", average);
    printf("Closing High: $%.2f\n", max);
    printf("Closing Low: $%.2f\n", min);
    printf("Volatility: %.2f\n", stddev);
    printf("Data analyzed from %s to %s\n", startDate, endDate);

    // Free the entire list after analysis
    Node* temp;
    while(head != NULL){
        temp = head;
        head = head->next;
        free(temp);
    }
}

//function to help curl work
size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream){
    return fwrite(ptr, size, nmemb, stream);
}

// grabs the data from the API saves it
void fetchData(const char* symbol, const char* function, const char* filename, int isDateRange){

    CURL *curl;
    FILE *file;
    CURLcode res;

    // create url to request csv from the API
    const char *baseUrl = "https://www.alphavantage.co/query?function=";
    const char *apikeyPrefix = "&apikey=";
    const char *symbolPrefix = "&symbol=";
    char *datatype;
    if(isDateRange){
        datatype = "&datatype=csv&outputsize=full";
    }
    else{
        datatype = "&datatype=csv";
    }
    // figure out url length/size to allocate space for it
    size_t urlLength = strlen(baseUrl) + strlen(function)
                     + strlen(symbolPrefix) + strlen(symbol)
                     + strlen(apikeyPrefix) + strlen(APIKEY)
                     + strlen(datatype) + 1;
                     // +1 for null terminator at end of string
    char *fullURL = malloc(urlLength);
    if(fullURL == NULL){
        fprintf(stderr, "Failed allocation of memory\n");
        return;
    }

    // create the url
    strcpy(fullURL, baseUrl);
    strcat(fullURL, function);
    strcat(fullURL, symbolPrefix);
    strcat(fullURL, symbol);
    strcat(fullURL, apikeyPrefix);
    strcat(fullURL, APIKEY);
    strcat(fullURL, datatype);

    // do curl operations
    curl = curl_easy_init();
    if(curl){
        file = fopen(filename, "wb");
        if(file == NULL){
            perror("File opening failure");
            free(fullURL);
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, fullURL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        printf("Getting data from: %s\n", fullURL);

        // performs curl operation
        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "CURL error %s\n", curl_easy_strerror(res));
        }
        else{
            printf("Data saved into %s\n", filename);
        }

        // cleanup & close stream
        fclose(file);
        curl_easy_cleanup(curl);
    }
    free(fullURL);
}

// gets current price & daily change
void getCurrentPrice(const char *symbol){
    const char *filename = "current.csv";
    fetchData(symbol, CURRENT, filename, 0);

    FILE *file = fopen(filename, "r");
    // error handling
    if(file == NULL){
        fprintf(stderr, "Error opening current price file");
        return;
    }

    char line[256];
    // skip header of csv
    fgets(line, sizeof(line), file);

    // iterates through columns to get the data
    if(fgets(line, sizeof(line), file) != NULL){

        char *token;
        int column = 0;
        char *price = NULL;
        char *change = NULL;
        char *changePercent = NULL;

        token = strtok(line, ",");
        while(token != NULL){
            if(column == 4){
                price = token;
            }
            else if(column == 8){
                change = token;
            }
            else if(column == 9){
                changePercent = token;
                break;
            }
            token = strtok(NULL,",");
            column++;
        }
        if (price != NULL && change != NULL && changePercent != NULL){

            float priceFloat = atof(price);
            float changeFloat = atof(change);
            
            size_t len = strlen(changePercent);
            if(len > 0 && changePercent[len - 1] == '\n'){
                
                changePercent[len - 1] = '\0';
            
            }

            char cleanPercent[16];
            strncpy(cleanPercent, changePercent, sizeof(cleanPercent));
            cleanPercent[sizeof(cleanPercent) - 1] = '\0';
            cleanPercent[strcspn(cleanPercent, "%")] = '\0';
            float changePercentFloat = atof(cleanPercent);

            printf("\n==== Current Price Analysis ====\n");
            printf("Current Price: $%.2f\n", priceFloat);
            printf("Daily Change: $%.2f(%.2f%%)\n", changeFloat,changePercentFloat);
            

        }
        else{
            printf("Failed to get all current price data\n");
        }
    }
    else{
        printf("Data not returned for symbol %s, \n", symbol);
    }

    // close file stream
    fclose(file);
}

void getHistoricPrice(const char *symbol){

    const char *filename = "historic.csv";
    fetchData(symbol, HISTORIC, filename, 0);

    Node *head = NULL;
    readCSV(filename, &head);

    analyze(head, NULL, NULL, symbol);
}

void getDateRangePrice(const char *symbol, const char *startDate, const char *endDate){

    const char *filename = "dateRange.csv";
    fetchData(symbol, HISTORIC, filename, 1);
    
    Node *head = NULL;
    readCSV(filename, &head);

    analyze(head, startDate, endDate, symbol);

}


