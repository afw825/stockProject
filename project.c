#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stock_analyzer.h"

#define APIKEY "4Z81PYCE9QHTYYBF"

// clears console to make output look better
// making it compatible with windows/linux
// might work on this project more after this class
void clearConsole(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}


int main(){

    int option;
    char symbol[10];

    while(1){

        clearConsole();

        printf("=====================================\n");
        printf("           Stock Analyzer\n");
        printf("=====================================\n");
        printf("1. Get Current Stock Price\n");
        printf("2. Get 100 Analysis of Stock\n");
        printf("3. Get Stock Analysis within a date range\n");
        printf("4. Exit\n");
        printf("Select an option(1, 2, 3, or 4): ");
        scanf("%d", &option);
        if(option == 4){
            printf("Exiting Program\n");
            return 0;
        }
        printf("Enter stock symbol: ");
        scanf("%s", symbol);


        if(option == 1){
            clearConsole();
            // get current stock price
            printf("Getting current stock price of %s\n", symbol);
            getCurrentPrice(symbol);
        }
        else if(option == 2){
            clearConsole();
            // get historical stock data
            printf("Getting historical stock data of %s\n", symbol);
            getHistoricPrice(symbol);
        }
        else if(option == 3){
            // get historical stock data
            char startDate[11];
            char endDate[11];
            printf("Enter start date (YYYY-MM-DD): ");
            scanf("%s", startDate);
            printf("Enter end date (YYYY-MM-DD): ");
            scanf("%s", endDate);
            clearConsole();
            printf("Getting historical stock data of %s from %s to %s\n", symbol, startDate, endDate);
            getDateRangePrice(symbol, startDate, endDate);
        }
        else{
            printf("Invalid option. Please select 1 or 2.\n");
            return 1;
        }

        printf("\nPress Enter to go back to the menu");
        getchar();
        getchar();
    }
}
