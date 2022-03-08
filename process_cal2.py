#!/usr/bin/env python3

import sys
import datetime

# This main function will read the input from command line and then call all the functions
def main():
    s_in = sys.argv[1]
    e_in = sys.argv[2]
    filename_in = sys.argv[3]
    if s_in[15] == "/":
        st_dt = datetime.datetime(int(s_in[8:12]), int(s_in[13:15]), int(s_in[16:]))
    else:
        st_dt = datetime.datetime(int(s_in[8:12]), int(s_in[13:14]), int(s_in[15:]))
    if e_in[13] == "/":
        end_dt = datetime.datetime(int(e_in[6:10]), int(e_in[11:13]), int(e_in[14:]), 23, 59, 59)
    else:
        end_dt = datetime.datetime(int(e_in[6:10]), int(e_in[11:12]), int(e_in[13:]), 23, 59, 59)

    ev_list = file_reader(filename_in[7:])
    full_list = ev_repeat(ev_list)
    full_list.sort(key=lambda full_list: full_list["sdt"])
    event_printer(full_list, st_dt, end_dt)

# This function reads the calender input and puts it in a list of events (each event is a dictionary)
def file_reader(filename):
    ev_list = []
    with open(filename) as f:
        for line in f:

            if line[0:12] == "BEGIN:VEVENT":
                r_check = 0

            if line[0:8] == "DTSTART:":
                s_str = line[8:]
                s_dt = datetime.datetime(int(s_str[0:4]), int(s_str[4:6]), int(s_str[6:8]),
                                         int(s_str[9:11]), int(s_str[11:13]), int(s_str[13:]))

            if line[0:6] == "DTEND:":
                e_str = line[6:]
                e_dt = datetime.datetime(int(e_str[0:4]), int(e_str[4:6]), int(e_str[6:8]),
                                         int(e_str[9:11]), int(e_str[11:13]), int(e_str[13:]))

            if line[0:6] == "RRULE:":
                index = line.find("UNTIL=")
                r_str = line[index + 6: index + 21]
                r_check = 1
                r_dt = datetime.datetime(int(r_str[0:4]), int(r_str[4:6]), int(r_str[6:8]),
                                         int(r_str[9:11]), int(r_str[11:13]), int(r_str[13:]))

            if line[0:9] == "LOCATION:":
                loc = line[9:]

            if line[0:8] == "SUMMARY:":
                smry = line[8:]

            if line[0:10] == "END:VEVENT":
                if r_check == 0:
                    ev_dict = {"sdt": s_dt, "edt": e_dt, "loc": loc, "smry": smry, "rep": r_check, "rdt": 0}
                    ev_list.append(ev_dict)
                else:
                    ev_dict = {"sdt": s_dt, "edt": e_dt, "loc": loc, "smry": smry, "rep": r_check, "rdt": r_dt}
                    ev_list.append(ev_dict)

    return ev_list

# This function checks if an event repeats. If it does then it increments the start date until it reaches
# the repeat until date. It puts all the repeated events in the list of events
def ev_repeat(list):
    ev_num = len(list)
    i = 0
    while i < ev_num:
        if list[i]["rep"] == 1:
            s_dt = list[i]["sdt"]
            while s_dt <= list[i]["rdt"]:
                s_dt += datetime.timedelta(weeks=1)
                ev_dict = {"sdt": s_dt, "edt": list[i]["edt"] + datetime.timedelta(weeks=1), "loc": list[i]["loc"],
                           "smry": list[i]["smry"], "rep": list[i]["rep"], "rdt": list[i]["rdt"]}
                list.append(ev_dict)
            list.pop(len(list)-1)
        i += 1

    return list

# This function formats the time to follow the spacing guidelines
def format_time(time):
    if time.hour != 10 and time.hour != 11 and time.hour != 12 and \
            time.hour != 22 and time.hour != 23 and time.hour != 0:
        str = time.strftime("%-I:%M %p")
        t = str.rjust(len(str) + 1)
    else:
        t = time.strftime("%-I:%M %p")
    return t

# This function prints the events that fall in the given time duration
def event_printer(ev_list, sdt, edt):
    ev_num = len(ev_list)
    i = 0
    same_d = "empty"
    while i < ev_num:
        if sdt <= ev_list[i]["sdt"] <= edt:
            if same_d == "empty":
                dt = ev_list[i]["sdt"].strftime("%B %d, %Y (%a)")
                print(dt)
                print("-" * len(dt))
                st = format_time(ev_list[i]["sdt"])
                et = format_time(ev_list[i]["edt"])
                print(st, "to", et + ":", ev_list[i]["smry"].strip(), "{{" + ev_list[i]["loc"].strip() + "}}")
                same_d = ev_list[i]["sdt"].day
            elif same_d != ev_list[i]["sdt"].day:
                dt = ev_list[i]["sdt"].strftime("%B %d, %Y (%a)")
                print("\n" + dt)
                print("-" * len(dt))
                st = format_time(ev_list[i]["sdt"])
                et = format_time(ev_list[i]["edt"])
                print(st, "to", et + ":", ev_list[i]["smry"].strip(), "{{" + ev_list[i]["loc"].strip() + "}}")
                same_d = ev_list[i]["sdt"].day
            elif same_d == ev_list[i]["sdt"].day:
                st = format_time(ev_list[i]["sdt"])
                et = format_time(ev_list[i]["edt"])
                print(st, "to", et + ":", ev_list[i]["smry"].strip(), "{{" + ev_list[i]["loc"].strip() + "}}")
                same_d = ev_list[i]["sdt"].day
        i += 1

if __name__ == "__main__":
    main()
