#!/usr/bin/env python3

import datetime
import re

class process_cal:
    ''' process_cal class will be used to parse the given ics file.
        It uses its instance variable ev_list to store the events by reading the ics file,
        checking for repeating events and sorting them. The only method that can be accessed in this class is
        get_events_for_day, which returns a formatted string of the events in a human readable form. '''

    def __init__(self, filename):
        ''' This is the constructor of this class '''

        self.ev_list = []
        self.filename = filename
        self.__read_events()
        self.__repeat_events()
        self.__sorter()

    def __read_events(self):
        ''' This private method reads the events from the raw ics file. It stores the dates as datetime
            objects and any other imformation in a dictionary. This dictionary is then appended to the
            instance list self.ev_list. '''

        with open(self.filename) as f:
            for line in f:

                if re.search(r"BEGIN:VEVENT", line):
                    r_check = 0

                if re.search(r"DTSTART:", line):
                    re_match = re.search(r"\d{8}T\d{6}", line)
                    s_str = re_match.group()
                    s_dt = datetime.datetime(int(s_str[0:4]), int(s_str[4:6]), int(s_str[6:8]),
                                             int(s_str[9:11]), int(s_str[11:13]), int(s_str[13:]))

                if re.search(r"DTEND:", line):
                    re_match = re.search(r"\d{8}T\d{6}", line)
                    e_str = re_match.group()
                    e_dt = datetime.datetime(int(e_str[0:4]), int(e_str[4:6]), int(e_str[6:8]),
                                             int(e_str[9:11]), int(e_str[11:13]), int(e_str[13:]))

                if re.search(r"UNTIL=", line):
                    re_match = re.search(r"\d{8}T\d{6}", line)
                    r_str = re_match.group()
                    r_check = 1
                    r_dt = datetime.datetime(int(r_str[0:4]), int(r_str[4:6]), int(r_str[6:8]),
                                             int(r_str[9:11]), int(r_str[11:13]), int(r_str[13:]))

                if re.search(r"LOCATION:", line):
                    re_match = re.search(r"(?<=LOCATION:).*", line)
                    loc = re_match.group()

                if re.search(r"SUMMARY:", line):
                    re_match = re.search(r"(?<=SUMMARY:).*", line)
                    smry = re_match.group()

                if re.search(r"END:VEVENT", line):
                    if r_check == 0:
                        ev_dict = {"sdt": s_dt, "edt": e_dt, "loc": loc, "smry": smry, "rep": r_check, "rdt": 0}
                        self.ev_list.append(ev_dict)
                    else:
                        ev_dict = {"sdt": s_dt, "edt": e_dt, "loc": loc, "smry": smry, "rep": r_check, "rdt": r_dt}
                        self.ev_list.append(ev_dict)

    def __repeat_events(self):
        ''' This private method checks whether an event is repeating. If it is, it increases the starting
            date by a week until the repeat date is reached.'''

        ev_num = len(self.ev_list)
        i = 0
        while i < ev_num:
            if self.ev_list[i]["rep"] == 1:
                s_dt = self.ev_list[i]["sdt"]
                while s_dt <= self.ev_list[i]["rdt"]:
                    s_dt += datetime.timedelta(weeks=1)
                    ev_dict = {"sdt": s_dt, "edt": self.ev_list[i]["edt"] + datetime.timedelta(weeks=1),
                               "loc": self.ev_list[i]["loc"], "smry": self.ev_list[i]["smry"],
                               "rep": self.ev_list[i]["rep"], "rdt": self.ev_list[i]["rdt"]}
                    self.ev_list.append(ev_dict)
                self.ev_list.pop(len(self.ev_list) - 1)
            i += 1

    def __sorter(self):
        ''' This private method is used to sort the self.ev_list. '''

        self.ev_list.sort(key=lambda ev: ev["sdt"])

    def __format_time(self, time):
        ''' This private method is used by the get_events_for_day to format the time according to
            the specification. '''

        if time.hour != 10 and time.hour != 11 and time.hour != 12 and \
                time.hour != 22 and time.hour != 23 and time.hour != 0:
            str = time.strftime("%-I:%M %p")
            t = str.rjust(len(str) + 1)
        else:
            t = time.strftime("%-I:%M %p")
        return t

    def __date_formatter(self, date):
        ''' This private method is used by the get_events_for_day to format the date according to
            the specification. The formatted date is returned along with the dashes in a string. '''

        ret = date.strftime("%B %d, %Y (%a)")
        ret += '\n'
        no_of_dashes = len(ret)
        i = 1
        while i < no_of_dashes:
            ret += '-'
            i += 1
        ret += '\n'
        return ret

    def get_events_for_day(self, date):
        ''' This public method is used to get events on a given day. It creates a string with all the events
            that take place on the provided date and is returned. If no events take place on that day
            None is returned. '''

        ev_num = len(self.ev_list)
        i = 0
        ret_str = 'empty'
        ev_found = 1
        date_cpy = date
        end_time = date_cpy.replace(hour=23, minute=59, second=59)
        while i < ev_num:
            if date <= self.ev_list[i]["sdt"] <= end_time:
                if ev_found == 0:
                    ret_str += '\n'
                if ev_found == 1:
                    ret_str = self.__date_formatter(self.ev_list[i]["sdt"])
                    ev_found = 0
                ret_str += self.__format_time(self.ev_list[i]["sdt"]) + " to " + \
                           self.__format_time(self.ev_list[i]["edt"])
                ret_str += ': ' + self.ev_list[i]["smry"] + ' '
                ret_str += "{{" + self.ev_list[i]["loc"] + "}}"
            i += 1

        if ret_str == 'empty':
            return None
        else:
            ret_str = ret_str.strip()
            return ret_str
