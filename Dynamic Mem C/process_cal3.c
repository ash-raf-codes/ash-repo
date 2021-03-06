/*
 * process_cal3.c
 *
 * Starter file provided to students for Assignment #3, SENG 265,
 * Fall 2021.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emalloc.h"
#include "ics.h"
#include "listy.h"

#define MAX_LINE_LEN 172

node_t* date_reader(char* filename, node_t* linked_list);
void input_dt_formatter(int from_yy, int from_mm, int from_dd,
    int to_yy, int to_mm, int to_dd, char* out_st_dt, char* out_e_dt);
node_t* repeat_events(node_t* linked_list);
void dt_increment(char* after, const char* before);
int reached_until_date(char* og_date, char* until_date);
void date_formatter(const char* in_dt, char* out_dt);
void time_formatter(const char* in_t, char* out_t);
void print_events(const char* s_date, const char* e_date, node_t* linked_list);
int same_day_check(const char* dt_1, const char* dt_2);
void free_mem(node_t* linked_list);

// Main function. It reads the input arguments and calls all the necessary functions.
int main(int argc, char *argv[])
{
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char* filename = NULL;
    int i;

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        }
        else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        }
        else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i] + 7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr,
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }


    node_t* ev_list = NULL;
    ev_list = date_reader(filename, ev_list);

    char s_date[20];
    char e_date[20];
    input_dt_formatter(from_y, from_m, from_d, to_y, to_m, to_d, s_date, e_date);

    ev_list = repeat_events(ev_list);

    print_events(s_date, e_date, ev_list);

    free_mem(ev_list);

    exit(0);
}

// This fucntion reads the .ics file and stores the relevant data into event nodes which utilises a linked list data structure.
node_t* date_reader(char* filename, node_t* linked_list) {
    FILE* fp;
    fp = fopen(filename, "r");
    char line[MAX_LINE_LEN];

    event_t* event = NULL;
    node_t* node = NULL;

    while (fgets(line, MAX_LINE_LEN, fp) != NULL) {

        if (strncmp(line, "BEGIN:VEVENT", strlen("BEGIN:VEVENT")) == 0) {
            event = emalloc(sizeof(event_t));
        }
        if (strncmp("DTSTART:", line, strlen("DTSTART:")) == 0) {
            strcpy(event->dtstart, line + strlen("DTSTART:"));
        }
        else if (strncmp("DTEND:", line, strlen("DTEND:")) == 0) {
            strcpy(event->dtend, line + strlen("DTEND:"));
        }
        char* ptr = strstr(line, "UNTIL=");
        if (ptr != NULL) {
            strncpy(event->until, ptr + strlen("UNTIL="), 15);
            strcpy(event->rrule, "yes");
        }
        else if (strncmp("LOCATION:", line, strlen("LOCATION:")) == 0) {
            strcpy(event->location, line + strlen("LOCATION:"));
            event->location[strlen(event->location) - 1] = '\0';
        }
        else if (strncmp("SUMMARY:", line, strlen("SUMMARY:")) == 0) {
            strcpy(event->summary, line + strlen("SUMMARY:"));
            event->summary[strlen(event->summary) - 1] = '\0';
        }
        else if (strncmp("END:VEVENT", line, strlen("END:VEVENT")) == 0) {
            node = new_node(event);
            linked_list = add_inorder(linked_list, node);
        }
    }
    fclose(fp);
    return linked_list;
}


// This function turns the date input from terminal into a string.
void input_dt_formatter(int from_yy, int from_mm, int from_dd,
    int to_yy, int to_mm, int to_dd, char* out_st_dt, char* out_e_dt) {
    sprintf(out_st_dt, "%d%02d%02dT000000", from_yy, from_mm, from_dd);
    sprintf(out_e_dt, "%d%02d%02dT235959", to_yy, to_mm, to_dd);
}


// This function checks if an event repeats. If it does then a new event node is made with the starting and ending dates incremented by a week.
 node_t* repeat_events(node_t* linked_list) {
    event_t* ev = NULL;
    node_t* node = NULL;
    node_t* curr = linked_list;
    while (curr != NULL) {
        if (strcmp(curr->val->rrule, "yes") == 0) {
            ev = emalloc(sizeof(event_t));
            dt_increment(ev->dtstart, curr->val->dtstart);
            dt_increment(ev->dtend, curr->val->dtend);
            strcpy(ev->until, curr->val->until);
            strcpy(ev->location, curr->val->location);
            strcpy(ev->summary, curr->val->summary);
            
            int res = reached_until_date(ev->dtstart, curr->val->until);
            if (res == 1) {
                strcpy(ev->rrule, "yes");
            }
            node = new_node(ev);
            linked_list = add_inorder(linked_list, node);

        }
        curr = curr->next;
    }
    return linked_list;
}


// Helper function for repeat_events. This function increments the date by seven days (taken from the timeplay.c file and edited for my program).
void dt_increment(char* after, const char* before) {
    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2dT%2d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday, &temp_time.tm_hour,
        &temp_time.tm_min, &temp_time.tm_sec);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += 7;

    full_time = mktime(&temp_time);
    strftime(after, 20, "%Y%m%dT%H%M%S", localtime(&full_time));
    after[strlen(after) - 1] = '\0';
}


// Helper function for repeat_events. This function checks if the newly created node is the final repeated event for a particular event.
int reached_until_date(char* og_date, char* until_date) {
    char temp[17];
    dt_increment(temp, og_date);
    if (strcmp(temp, until_date) <= 0) {
        return 1;
    }
    else {
        return 0;
    }

}


// This function formats the date according to the given specification. 
void date_formatter(const char* in_dt, char* out_dt) {
    char temp[9];
    strncpy(temp, in_dt, 8);
    char format_date[MAX_LINE_LEN];

    struct tm date = { 0 };
    sscanf(temp, "%4d%2d%2d", &date.tm_year, &date.tm_mon, &date.tm_mday);
    date.tm_year -= 1900;
    date.tm_mon -= 1;
    time_t final_date = mktime(&date);
    struct tm* ptr = localtime(&final_date);
    strftime(format_date, MAX_LINE_LEN, "%B %d, %Y (%a)", ptr);

    sprintf(out_dt, "%s\n", format_date);
    for (int i = 0; i < strlen(format_date); i++) {
        sprintf(out_dt + strlen(out_dt), "-");
    }
    sprintf(out_dt + strlen(out_dt), "\n");
}


// This function formats the time according to the given specification.
void time_formatter(const char* in_t, char* out_t) {

    char hour[3], min[3];
    char am_or_pm[3];
    strncpy(hour, in_t + 9, 2);
    strncpy(min, in_t + 11, 2);

    int h = atoi(hour);
    int m = atoi(min);

    if (h > 12) {
        h -= 12;
        strcpy(am_or_pm, "PM");
    }
    else if (h == 12) {
        strcpy(am_or_pm, "PM");
    }
    else {
        strcpy(am_or_pm, "AM");
    }
    sprintf(out_t, "%2d:%02d %s", h, m, am_or_pm);
}


// This function prints the full formatted date, time, location and summary according to the specification.
void print_events(const char* s_date, const char* e_date, node_t* linked_list) {

    char same_day_tracker[MAX_LINE_LEN];
    strcpy(same_day_tracker, "empty");

    node_t* curr = linked_list;

    while(curr != NULL) {

        if (strcmp(curr->val->dtstart, s_date) >= 0 && strcmp(curr->val->dtstart, e_date) <= 0) {
            char print_date[MAX_LINE_LEN];
            date_formatter(curr->val->dtstart, print_date);

            int ret = same_day_check(print_date, same_day_tracker);

            if (ret == -1) {
                printf("%s", print_date);
                strcpy(same_day_tracker, print_date);
            }
            else if (ret == 1) {
                printf("\n");
                printf("%s", print_date);
                strcpy(same_day_tracker, print_date);
            }

            char s_time[MAX_LINE_LEN];
            char e_time[MAX_LINE_LEN];
            time_formatter(curr->val->dtstart, s_time);
            time_formatter(curr->val->dtend, e_time);
            printf("%s to %s: %s {{%s}}\n", s_time, e_time, curr->val->summary, curr->val->location);
        }
        curr = curr->next;
    }
}


// Helper function for print_events. This function checks whether 2 events take place on the same day. Returns -1 at the start of the print_event loop.
int same_day_check(const char* dt_1, const char* dt_2) {

    if (strcmp(dt_2, "empty") == 0) {
        return -1;
    }
    if (strcmp(dt_2, dt_1) != 0) {
        return 1;
    }
    return 0;
}


// This function is used to free all the allocated memory for the linked list.
void free_mem(node_t* linked_list) {
    node_t* curr = NULL;
    while (linked_list != NULL) {
        curr = linked_list;
        linked_list = linked_list->next;
        free(curr->val);
        free(curr);
    }
}



