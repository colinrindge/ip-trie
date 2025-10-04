// 
// File: place_ip.c
// Description: Reads IPs from file and converts into trie, then allows searching them
// 
// @author Colin Rindge cfr1524
// // // // // // // // // // // // // // // // // // // // // // // // 

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "trie.h"

//A struct containing location data to be put in a trie node
struct ivalue_s {
    char * country_code;    //< The country code
    char * country_name;    //< The name of the country
    char * province;        //< The name of the province
    char * city;            //< The name of the city
};

/*
 * Creates a struct containing the specified location data to be inserted into a trie node
 *
 * @param country_code      The country code
 * @param country_name      The name of the country
 * @param province          The name of the province
 * @param city              The name of the city
 * @returns                 A pointer to a ivalue_s struct containig the location data
 */
Value create_value(char * country_code, char * country_name, char * province, char * city){
    Value v = malloc(sizeof(struct ivalue_s));
    if(v == NULL){
        fprintf(stderr, "error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    v->country_code = strdup(country_code);
    if(v->country_code == NULL){
        fprintf(stderr, "error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    v->country_name = strdup(country_name);
    if(v->country_name == NULL){
        fprintf(stderr, "error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    v->province = strdup(province);
    if(v->province == NULL){
        fprintf(stderr, "error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    v->city = strdup(city);
    if(v->city == NULL){
        fprintf(stderr, "error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return v;
}

/*
 * Prints out the ip in ipv4 format and the location data associated with the IP in 
 *  an entry to the specified stream
 * 
 * @param entry     The entry containing the data to print out
 * @param stream    The stream to print the data to
 */
void show_info(Entry entry, FILE * stream){
    unsigned int int_ip = entry->key;
    unsigned int num1 = int_ip >> (BITSPERBYTE * 3);
    unsigned int num2 = (int_ip >> (BITSPERBYTE * 2)) & 255;
    unsigned int num3 = (int_ip >> (BITSPERBYTE * 1)) & 255;
    unsigned int num4 = int_ip & 255;

    fprintf(stream, "%u: ", int_ip);
    fprintf(stream, "(%d.%d.%d.%d, ", num1, num2, num3, num4);
    fprintf(stream, "%s: ", entry->v->country_code);
    fprintf(stream, "%s, ", entry->v->country_name);
    fprintf(stream, "%s, ", entry->v->province);
    fprintf(stream, "%s)\n", entry->v->city);
}

/*
 * Frees all contents of value struct.
 */
void free_value(Value v){
    free(v->country_code);
    free(v->country_name);
    free(v->city);
    free(v->province);
}


/*
 * Reads a the passed file
 */
int main(int argc, char * argv[]){
    if(argc != 2){
        fprintf(stderr, "usage: place_ip filename\n");
        return EXIT_FAILURE;
    }

    FILE * fp = fopen(argv[1], "r");
    if(fp == NULL){
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    Trie tr = ibt_create(*(show_info), *(free_value));

    char * read_line = NULL;
    size_t len = 0;
    ssize_t read;
    while( (read = getline(&read_line, &len, fp)) != -1 ){
        if(read_line == NULL){
            perror(argv[1]);
            return EXIT_FAILURE;
        }

        ikey_t ip1;
        ikey_t ip2;
        char * country_code;
        char * country_name;
        char * province;
        char * city;

        char * line = read_line;

        line = strtok(line, "\",");
        ip1 = strtol(line, NULL, 10);

        line = strtok(NULL, "\",");
        ip2 = strtol(line, NULL, 10);

        line = strtok(NULL, "\",");
        country_code = line;

        line = strtok(NULL, "\",");
        country_name = line;

        line = strtok(NULL, "\",");
        province = line;

        line = strtok(NULL, "\",");
        city = line;

        Value v1 = create_value(country_code, country_name, city, province);
        Value v2 = create_value(country_code, country_name, city, province);

        ibt_insert(tr, ip1, v1);
        ibt_insert(tr, ip2, v2);
    }

    if(ibt_size(tr) == 0){
        fprintf(stderr, "error: empty dataset\n");
        exit(EXIT_FAILURE);
    }

    printf("\n");
    printf("height: %ld\n", ibt_height(tr));
    printf("size: %ld\n", ibt_size(tr));
    printf("node_count: %ld\n", ibt_node_count(tr));
    printf("\n");
    printf("\n");


    printf("Enter an ipv4 string or a number (or a blank line to quit).\n");
    printf("> ");

    char * read_line2 = NULL;
    size_t len2 = 0;
    ssize_t read2;
    while( (read2 = getline(&read_line2, &len2, stdin)) != -1 ){
        bool invalid = false;
        if(read_line2 == NULL){
            fprintf(stderr, "error: couldn't read input");
            return EXIT_FAILURE;
        }
        
        if(read_line[0] == '\0'){
            break;
        }

        char * line = read_line2;
        ikey_t int_ip = 0;

        if(strcmp(read_line2, "\n") == 0){
            break;
        }

        if(strstr(read_line2, ".") == NULL){
            long int temp = strtol(line, NULL, 10);

            if(temp < 0){
                invalid = true;
            }
            int_ip = temp;

        } else {

            int i = 1;
            line = strtok(line, ".");
            while(line != NULL){
                int_ip <<= BITSPERBYTE;

                if(i > 4){
                    invalid = true;
                    break;
                }

                long long int temp = strtol(line, NULL, 10);

                if(temp < 0 || temp > 255){
                    invalid = true;
                    break;
                }

                int_ip = (int_ip | (ikey_t)temp);
                line = strtok(NULL, ".");
                i++;

            }

            while(i <= 4){
                int_ip <<= BITSPERBYTE;
                i++;
            }

        }

        if(invalid){
            printf("INVALID KEY\n");
        }
        else{
            Entry e = ibt_search(tr, int_ip);
            show_info(e, stdout);
        }
        printf("> ");
    }

    ibt_destroy(tr);
    fclose(fp);
    free(read_line);
    free(read_line2);
}