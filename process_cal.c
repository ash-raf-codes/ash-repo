#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500


struct dateObj {
    char DTSTART[20];
    char DTEND[20];
    char RRULE[5];
    char LOCATION[MAX_LINE_LEN];
    char SUMMARY[MAX_LINE_LEN];
    char UNTIL[20];
};

struct dateObj EV[MAX_EVENTS];


int date_reader(char* filename);
void input_dt_formatter(int from_yy, int from_mm, int from_dd,
    int to_yy, int to_mm, int to_dd, char* out_st_dt, char* out_e_dt);
void date_formatter(const char* in_dt, char* out_dt);
void time_formatter(const char* in_t, char* out_t);
int repeat_events(struct dateObj event, int ev_num);
void dt_increment(char* after, const char* before);
void event_sorter(int ev_num);
int ev_comparator(const void* ev1, const void* ev2);
void print_events(const char* s_date, const char* e_date, int ev_num);
int same_day_check(const char* dt_1, const char* dt_2);




int main(int argc, char *argv[])
{
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i; 
    int ev_num = 0;

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);

    /* Starting calling your own code from this point. */
    
    }
    ev_num = date_reader(filename);

    char s_date[20];
    char e_date[20];
    input_dt_formatter(from_y, from_m, from_d, to_y, to_m, to_d, s_date, e_date);

    int temp = ev_num;
    for (int i = 0; i < temp; i++) {
        if (strcmp(EV[i].RRULE, "yes") == 0) {
            ev_num = repeat_events(EV[i], ev_num);
            ev_num--;
        }
    }

    event_sorter(ev_num);

    print_events(s_date, e_date, ev_num);

    exit(0);
}



// This function reads the .ics file and stores the relevant information in dateOjb and returns the number of events read.
int date_reader(char* filename) {
    int i = 0;
    FILE* fp;
    fp = fopen(filename, "r");
    char line[MAX_LINE_LEN];

    while (fgets(line, MAX_LINE_LEN, fp) != NULL) {
        
        if (strncmp("DTSTART:", line, strlen("DTSTART:")) == 0) {
            strcpy(EV[i].DTSTART, line + strlen("DTSTART:"));
        }
        else if (strncmp("DTEND:", line, strlen("DTEND:")) == 0) {
            strcpy(EV[i].DTEND, line + strlen("DTEND:"));
        }
        char* ptr = strstr(line,"UNTIL=");
        if (ptr != NULL) {
            strncpy(EV[i].UNTIL, ptr + strlen("UNTIL="), 15);
            EV[i].UNTIL[strlen(EV[i].UNTIL)] = '\n';
            EV[i].UNTIL[strlen(EV[i].UNTIL)+1] = '\0';
            strcpy(EV[i].RRULE,"yes");
        }
        else if (strncmp("LOCATION:", line, strlen("LOCATION:")) == 0) {
            strcpy(EV[i].LOCATION, line + strlen("LOCATION:"));
            EV[i].LOCATION[strlen(EV[i].LOCATION)-1] = '\0';
        }
        else if (strncmp("SUMMARY:", line, strlen("SUMMARY:")) == 0) {
            strcpy(EV[i].SUMMARY, line + strlen("SUMMARY:"));
            EV[i].SUMMARY[strlen(EV[i].SUMMARY)-1] = '\0';
        }
        else if (strncmp("END:VEVENT", line, strlen("END:VEVENT")) == 0) {
            i++;
        }
    }
    fclose(fp);
    return i;
}

// This function turns the date input from terminal into a string.
void input_dt_formatter(int from_yy, int from_mm, int from_dd,
    int to_yy, int to_mm, int to_dd, char* out_st_dt, char* out_e_dt) {

    sprintf(out_st_dt, "%d%02d%02dT000000", from_yy, from_mm, from_dd);
    sprintf(out_e_dt, "%d%02d%02dT235959", to_yy, to_mm, to_dd);
}

// This function formats the date according to the given specification. 
void date_formatter(const char* in_dt, char* out_dt) {
    char temp[9];
    strncpy(temp, in_dt, 8);
    char format_date[MAX_LINE_LEN];

    struct tm date = {0};
    sscanf(temp, "%4d%2d%2d", &date.tm_year, &date.tm_mon, &date.tm_mday);
    date.tm_year -= 1900;
    date.tm_mon -= 1;
    time_t final_date = mktime(&date);
    struct tm *ptr = localtime(&final_date);
    strftime(format_date, MAX_LINE_LEN, "%B %d, %Y (%a)", ptr);
    
    sprintf(out_dt, "%s\n", format_date);
    for (int i = 0; i < strlen(format_date); i++) {
        sprintf(out_dt + strlen(out_dt),"-");
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
    sprintf(out_t,"%2d:%02d %s", h, m, am_or_pm);
}

// This function creates a new event at the end of the EV array to store the repeated events.
int repeat_events(struct dateObj event, int ev_num) {
    int temp = ev_num;
    struct dateObj ptr = event;
     while (temp <= MAX_EVENTS && strcmp(ptr.DTSTART, event.UNTIL) <= 0) {
         dt_increment(EV[temp].DTSTART, ptr.DTSTART);
         dt_increment(EV[temp].DTEND, ptr.DTEND);
         strcpy(EV[temp].LOCATION, ptr.LOCATION);
         strcpy(EV[temp].SUMMARY, ptr.SUMMARY);
         strcpy(EV[temp].RRULE, "no");
         ptr = EV[temp];
         temp++;
     }
  
     return temp;
}

// Helper function for repeat_events. This function increments the date by seven days (taken from the timeplay.c file and edited for my program).
void dt_increment(char* after, const char* before) {
    struct tm temp_time;
    time_t    full_time;
    char      temp[5];

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

// This function sorts the EV array in an increasing order of starting date.
void event_sorter(int ev_num) {
    qsort(EV, ev_num, sizeof(struct dateObj), ev_comparator);
}

// Helper function for event_sorter. This function compares the starting dates of two events and returns the value.
int ev_comparator(const void* ev1, const void* ev2) {
    const struct dateObj* event1 = (struct dateObj*)ev1;
    const struct dateObj* event2 = (struct dateObj*)ev2;
    int ret = strcmp(event1->DTSTART, event2->DTSTART);
    return ret;
}

// This function prints the full formatted date, time, location and summary according to the specification.
void print_events(const char* s_date, const char* e_date, int ev_num) {

    char same_day_tracker[MAX_LINE_LEN];
    strcpy(same_day_tracker, "empty");

    for (int i = 0; i < ev_num; i++) {

        if (strcmp(EV[i].DTSTART, s_date) >= 0 && strcmp(EV[i].DTEND, e_date) <= 0) {
            char print_date[MAX_LINE_LEN];
            date_formatter(EV[i].DTSTART, print_date);

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
            time_formatter(EV[i].DTSTART, s_time);
            time_formatter(EV[i].DTEND, e_time);
            printf("%s to %s: %s {{%s}}\n", s_time, e_time, EV[i].SUMMARY, EV[i].LOCATION);
        }
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





