import curses
from collections import defaultdict
import logging

logger = logging.getLogger()


class UserInterface:
    """
    The class that renders everything on screen.

    :ivar screen: a reference to the curses screen object.
    :ivar list sites: stores the monitored websites
    :ivar defaultdict stored_info: contains the string containing the metrics for each pair site
    :ivar defaultdict stored_plot: contains the plot for each pair (site, delay)
    :ivar defaultdict cum_metrics: contains the last few retrieved metrics
    :ivar defaultdict changed: remembers whether a (site, delay) s plot and info have been changed since the last update
    :ivar defaultdict availability_changes: for each website, stores when it went down or recovered
    :ivar int cursor: the number of the page to render
    :ivar int max_cursor: the maximum value the cursor could have
    :ivar bool set_stop: whether the program should quit
    """

    def __init__(self, sites, screen):
        # Used defaultdict instead of dicts to allow adding / removing sites at run time later without much issues
        self.screen = screen
        self.h, self.w = self.screen.getmaxyx()
        self.init_curses()
        self.sites = sites
        self.stored_info = defaultdict(list)
        self.stored_plot = defaultdict(list)
        self.stored_metrics = defaultdict(lambda: defaultdict(lambda: None))
        self.changed = defaultdict(lambda: True)
        self.current_page = 0
        self.cursor = 0
        self.max_cursor = len(sites)
        self.set_stop = False

    def init_curses(self):
        """
        Initiates the screen with the right settings.
        """
        self.screen.keypad(1)
        self.screen.timeout(10)
        curses.noecho()
        curses.curs_set(0)
        curses.cbreak()
        curses.start_color()
        curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_WHITE)
        curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
        curses.init_pair(3, curses.COLOR_RED, curses.COLOR_BLACK)

    def stop(self):
        """
        Stops the user interface and restores the terminal
        """
        self.set_stop = True
        curses.endwin()

    def update_and_display(self, metrics):
        """
        Updates the UI's data and renders the screen

        :param metrics:
        """
        for site, metric in metrics.items():
            self.changed[(1, site)] = True
            self.changed[(2, site)] = True
            self.changed[(3, site)] = True
            for delay, values in metric:
                # This is to avoid having both unavailable_since and recovered_at set at the same time
                self.stored_metrics[(site, delay)]['unavailable_since'] = None
                self.stored_metrics[(site, delay)]['recovered_at'] = None
                for k, v in values.items():
                    self.stored_metrics[(site, delay)][k] = v
                    self.cum_metrics[(site, delay)][k].append(v)
        #  Clears the screen and reads key presses
        res = self.get_keypress()
        self.screen.erase()
        # If screen is resized, update the height and width
        if curses.is_term_resized(self.h, self.w):
            self.h, self.w = self.screen.getmaxyx()
        # renders the current page
        if self.current_page == 0:
            self.welcome_screen()
        elif self.current_page == 1:
            self.summary_screen()
        elif self.current_page == 2:
            self.log_screen()
        else:
            self.site_info()
        return res

    def get_keypress(self):
        """
        Reads the user input and initiates the right actions

        """
        ch = self.screen.getch()
        if ch == curses.KEY_UP:
            self.cursor = max(self.cursor - 1, 0)
        elif ch == curses.KEY_DOWN:
            self.cursor = min(self.cursor + 1, self.max_cursor)
        elif ch == ord('q') or ch == ord('Q'):
            return 'q'
        elif ch == ord('h') or ch == ord('H'):
            self.cursor = 0
            self.current_page = 0
        elif ch == curses.KEY_ENTER or ch == 10 or ch == 13:
            if not self.current_page:
                self.current_page = self.cursor + 1
                self.cursor = 0

    def welcome_screen(self):
        """
        Renders the screen for the main menu
        The text is as follow:

        .. aafig::
            :textual:

                 ________________
                |                |
                |                |
                | Site Monitorer |
                |                |
                |________________|

        Please choose an option:

        |  0001 - Summary
        |  0002 - Logs
        |  0003 - site 1
        |  0004 - site 2

        """
        #  If this screen has been changed (as in a new website has been added), recalculate the string
        if self.changed[0]:
            text = [" ________________", "|                |", "|                |", "| Site Monitorer |",
                    "|                |", "|________________|", "", "Please choose an option:", "", "0001 - Summary",
                    "0002 - Logs"]
            text.extend([f"{idx + 3:04d} - {site[0]}" for idx, site in enumerate(self.sites)])
            self.changed[0] = False
            self.stored_info[0] = text
        welcome_message = self.stored_info[0]
        curs = max(self.cursor - self.h + 10, 0)
        for i in range(curs, min(curs + self.h, len(welcome_message))):
            #  If part of the header
            if i < 7:
                self.screen.addstr(i - curs, round(self.w / 2) - 8, welcome_message[i])
            #   Print the instruction
            elif i == 7:
                self.screen.addstr(i - curs, 5, welcome_message[i])
            #   Print the list
            else:
                if self.cursor + 9 == i:
                    self.screen.addstr(i - curs, 7, welcome_message[i], curses.color_pair(1))
                else:
                    self.screen.addstr(i - curs, 7, welcome_message[i])
        self.max_cursor = max(len(self.sites) + 1, 0)
        #  Render
        self.screen.refresh()
