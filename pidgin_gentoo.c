/*
Copyright (C) 2008 Simon Poirier <simpoir@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define PURPLE_PLUGINS

#include <glib.h>

#include <string.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "savedstatuses.h"
#include "status.h"


void set_savedstatus(const char* message);
char get_currentCompile();
static gboolean stupidloop(gpointer unused);
static gboolean plugin_unload(PurplePlugin* plugin);


char currentCompile[100];
guint timerHandle = -1;
int timeout = -1;

static gboolean
plugin_load(PurplePlugin *plugin) 
{
    timeout = purple_prefs_get_int("/plugins/core/pidgin_gentoo/delay");
    timerHandle = purple_timeout_add(timeout * 1000, stupidloop, NULL);

    return TRUE;
}

static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin) {
	PurplePluginPrefFrame *frame;
	PurplePluginPref *ppref;

	frame = purple_plugin_pref_frame_new();

	ppref = purple_plugin_pref_new_with_label("Settings");
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
									"/plugins/core/pidgin_gentoo/delay",
									"Refresh delay(s)");
	purple_plugin_pref_set_bounds(ppref, 5, 255);
	purple_plugin_pref_frame_add(frame, ppref);

	ppref = purple_plugin_pref_new_with_name_and_label(
								"/plugins/core/pidgin_gentoo/logfile",
								"emerge log path");
	purple_plugin_pref_frame_add(frame, ppref);

	return frame;
}

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,   /* page_num (Reserved) */
	NULL, /* frame (Reserved) */
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    "core-pigin_gentoo",
    "Pidgin Gentoo",
    "0.2",

    "pidgin gentoo",
    "This is a plugin to update status to package currently building",
    "Simon Poirier <simpoir@hotmail.com>",
    "http://nowebsite.nodomain",

    plugin_load,                        //load
    plugin_unload,                      //unload
    NULL,                               //destroy
    
    NULL,
    NULL,
    &prefs_info,                        //prefs info
    NULL,                               //action info

    //unused
    NULL,
    NULL,
    NULL,
    NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
	purple_prefs_add_none("/plugins/core/pidgin_gentoo");
	purple_prefs_add_int("/plugins/core/pidgin_gentoo/delay", 30);
	purple_prefs_add_string("/plugins/core/pidgin_gentoo/logfile",
							"/var/log/emerge.log");
}

static gboolean
plugin_unload(PurplePlugin* plugin)
{
    purple_timeout_remove(timerHandle);
    timerHandle = -1;

    return TRUE;
}

void
set_savedstatus(const char* message)
{
    PurpleSavedStatus* savedStatus = NULL;
    PurpleSavedStatus* currentSavedStatus = purple_savedstatus_get_current();
    savedStatus = purple_savedstatus_find("GENTOO");
    //create new status if no gentoo status exists
    if (!savedStatus)
    {
        savedStatus = purple_savedstatus_new("GENTOO", purple_savedstatus_get_type(currentSavedStatus));
    }

    //fill status with emerge state
    if (strcmp("GENTOO", purple_savedstatus_get_title(currentSavedStatus)))
    {
        //set new status same type as current
        purple_savedstatus_set_type(savedStatus, purple_savedstatus_get_type(currentSavedStatus));
    }
    purple_savedstatus_set_message(savedStatus, message);
}

static gboolean
stupidloop(gpointer unused)
{
    char msg[1024];
    int newTimeout = -1;

    //reload timer option
    newTimeout = purple_prefs_get_int("/plugins/core/pidgin_gentoo/delay");
    if (newTimeout != timeout)
    {
        //remove timer
        if (timerHandle != -1) 
        {
            purple_timeout_remove(timerHandle);
            timerHandle = -1;
        }
        //add new timer
        newTimeout = timeout;
        timerHandle = purple_timeout_add(timeout * 1000, stupidloop, NULL);
    }

    //fetch compilation status
    get_currentCompile();
    strcpy(msg, "GENTOO building: ");
    strcat(msg, currentCompile);
    //set status string
    purple_debug_info("core-pigin_gentoo", "new status %s\n", msg);
    set_savedstatus(msg);

    return TRUE;
}

char
get_currentCompile()
{
    FILE* logfile = NULL;
    char line[1024] = "";
    char* readptr = NULL;
    char gotdata = FALSE;
    char emergedone = FALSE;
    char* readend = NULL;

    logfile = fopen(purple_prefs_get_string("/plugins/core/pidgin_gentoo/logfile"), "r");
    if (logfile == NULL) return;

    // fetch end of emerge log
    fseek(logfile, -1024, SEEK_END);
    fread(line, 1024, 1, logfile);
    readptr = line;
    readend = line+1024;
    
    while (readptr!=readend)
    {
        // look for build
        if (!strncmp(readptr, ">>> emerge", 10))
        {
            int matched = 0;
            matched = sscanf(readptr, ">>> emerge (%*i of %*i) %s", currentCompile);
            if (matched)
            {
                emergedone = FALSE;
            }
        }
        // also look for en of compilation
        else if (!strncmp(readptr, "*** terminating", 15))
        {
            emergedone = FALSE;
            strcpy(currentCompile, "nothing! :D");
        }
        ++readptr;
    }

    fclose(logfile);
    return emergedone;
}

PURPLE_INIT_PLUGIN(pidgin_emerge, init_plugin, info)

