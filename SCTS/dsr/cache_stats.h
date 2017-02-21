/*
 * cache_stats.h
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
 
#ifndef __cache_stats__
#define __cache_stats__

#include "routecache.h"

#define ACTION_ADD_ROUTE        1
#define ACTION_NOTICE_ROUTE     2
#define ACTION_FIND_ROUTE       3
#define ACTION_DEAD_LINK        4
#define ACTION_EVICT            5
#define ACTION_CHECK_CACHE      6

static char *action_name [] = {"",
                              "add-route", "notice-route", "find-route",
                              "dead-link", "evicting-route", "check-cache"};

class cache_stats {
public:
        void reset() {
                route_add_count = 0;
                route_add_bad_count = 0;
                subroute_add_count = 0;
                subroute_add_bad_count = 0;
                link_add_tested = 0;

                route_notice_count = 0;
                route_notice_bad_count = 0;
                subroute_notice_count = 0;
                subroute_notice_bad_count = 0;
                link_notice_tested = 0;

                route_find_count = 0;
		route_find_for_me = 0;
                route_find_bad_count = 0;
                route_find_miss_count = 0;
                subroute_find_count = 0;
                subroute_find_bad_count = 0;

		link_add_count = 0;
		link_add_bad_count = 0;
		link_notice_count = 0;
		link_notice_bad_count = 0;
		link_find_count = 0;
		link_find_bad_count = 0;
		
		link_good_time = 0.0;

        }
                
        int     route_add_count;
        int     route_add_bad_count;
        int     subroute_add_count;
        int     subroute_add_bad_count;
        int     link_add_tested;

        int     route_notice_count;
        int     route_notice_bad_count;
        int     subroute_notice_count;
        int     subroute_notice_bad_count;
        int     link_notice_tested;

        int     route_find_count;
        int     route_find_for_me;
        int     route_find_bad_count;
        int     route_find_miss_count;
        int     subroute_find_count;
        int     subroute_find_bad_count;

	// extras for the Link-State Cache
	int	link_add_count;
	int	link_add_bad_count;
	int	link_notice_count;
	int	link_notice_bad_count;
	int	link_find_count;
	int	link_find_bad_count;

	double     link_good_time;
};


/*===============================================================
  Handler to collect statistics once per second.
----------------------------------------------------------------*/

class MobiHandler : public Handler {
public:
        MobiHandler(RouteCache *C) {
                interval = 1.0;
                cache = C;
        }
        void start() {
                Scheduler::instance().schedule(this, &intr, 1.0);
        }
        void handle(Event *);
private:
        double interval;
        Event intr;
        RouteCache *cache;
};

#endif /* __cache_stats_h__ */
