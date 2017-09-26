/*
*  CSCI 352 Assignment 3 - GDK+ 3 Process Monitor
*  Garrett Gregory, 30 May 2016
*
*       This is a very modern process monitor for linux-based systems. It displays a TreeView containing a
*       ListStore of rows containing the name, owner name, %CPU, PID, and memory usage of each running process.
*       To anyone reading through this code, I aplogize for the grotesque lengthiness of updateStore().
*
*   NOTE: run with...    gcc `pkg-config --cflags gtk+-3.0` -o procmonit procmonit.c `pkg-config --libs gtk+-3.0` -g
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#define MIN_DIM 600
#define COL_WIDTH 100
#define MAX_PATH 100
#define MAX_STR 1024
#define MAX_PID 4194304
#define MEGA 1048576
#define KILO 1024
#define ONE_SECOND 1000
#define RAT 20
#define PIGEON 2000
#define RSS_IDX 2
#define SH_PAGES_IDX 3
#define UTIME_IDX 14
#define STIME_IDX 15

enum {
    ICON,
    PROC_NAME,
    USER_NAME,
    PERC_CPU,
    PROC_ID,
    MEM_USED,
    MEM_ACTUAL, // not a displayed value
    COLUMNS
};

int proctimes[MAX_PID][2] = {0}; // utime + stime for each process. [0] = old, [1] = new
int cpproctimes[MAX_PID][2] = {0}; // total added up time for cpu. [0] = old, [1] = new



/* getInfoToken()
*
* this function grabs a char* token from a file at the specified index
*
* PARAMS:
* char *path  -  path of file to read
* int n  -  index of token
* char **outString  -  string that gets written to
*/
int getInfoToken(char *path, int n, char **outString){
    FILE *fileptr;

    fileptr = fopen(path, "r");
    if (fileptr == NULL) return -1;  // process has ended, directory was closed
    // skip tokens until we reach index n
    while (n > 0) {
        fscanf(fileptr, "%s", *outString);
        n--;
    }
    fclose(fileptr);
    return 0;
}

/* getTotalCpuTimes()
*
* this function parses the /proc/stat file and adds up the numbers in it, then returns that sum.
*/
int getTotalCpuTimes(){
    FILE *fileptr;
    int i, total=0;
    char *tempStr;
    tempStr = malloc(MAX_STR);

    fileptr = fopen("/proc/stat", "r");
    fscanf(fileptr, "%s", tempStr); // skip first token
    for (i=2; i<=10; i++) {
        fscanf(fileptr, "%s", tempStr);
        total += atoi(tempStr);
    }
    fclose(fileptr);
    free(tempStr);
    return total;
}

/* updateStore()
*
* this function updates the contents of the store displayed by this program. First, check if any
* processes being displayed have been closed and remove them from the store if necessary. Next,
* update any information for processes already displayed. Finally, display new processes that have
* been started.
*
* PARAMS:
* GtkListStore *store  -  store to be updated
*/
gboolean updateStore(GtkListStore *store){
    GtkTreeIter iter, iter2;
    DIR *d;
    struct dirent *dir;
    struct stat stats;
    char path[MAX_PATH], *procName, *ownerName, *memUsageString, *timeString;
    int i, doubleEntry=0, removeRow, r;
    double memUsage;
    GValue value = {0}, newCPU = {0}, newMEM = {0};
    GError *error = NULL;
    GdkPixbuf *image1 = gdk_pixbuf_new_from_file("pigeon.png", &error);
    GdkPixbuf *image2 = gdk_pixbuf_new_from_file("gimp.png", &error);
    GdkPixbuf *image3 = gdk_pixbuf_new_from_file("penguin.png", &error);
    GdkPixbuf *image;

    // check if any processes have ended. if so, remove them.
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter2);
    while (gtk_list_store_iter_is_valid(store, &iter2)) {
        d = opendir("/proc/");
        removeRow = 1;
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter2, PROC_ID, &value);
                // check if folder has same pid as process row
                if (atoi(dir->d_name)==(int)g_value_get_int(&value)) removeRow = 0;
                g_value_unset(&value);
            }
            closedir(d);
            if (removeRow) {
                gtk_list_store_remove(GTK_LIST_STORE(store), &iter2);
                continue;
            }
        }
        gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2);
    }

    d = opendir("/proc/");
    if (d) {
        // do updates for processes
        while ((dir = readdir(d)) != NULL) {
            if (atoi(dir->d_name)==0) continue;
            for (i=0; i < strlen(dir->d_name); i++) {
                if (!isdigit(dir->d_name[i])) break;
                if (i==strlen(dir->d_name)-1) {
                    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter2);
                    while (gtk_list_store_iter_is_valid(store, &iter2)) {
                        gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter2, PROC_ID, &value);
                        if (atoi(dir->d_name)==(int)g_value_get_int(&value)) {
                            doubleEntry = 1;
                            //get memory usage
                            strcpy(path, "/proc/");
                            strcat(path, dir->d_name);
                            strcat(path, "/statm");
                            memUsageString = (char *) malloc(MAX_STR);
                            r = getInfoToken(path, RSS_IDX, &memUsageString); // get resident set size
                            if (r == -1) {
                                gtk_list_store_remove(GTK_LIST_STORE(store), &iter2);
                                g_value_unset(&value);
                                gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2);
                                continue;
                            }
                            memUsage = atoi(memUsageString); //convert to bytes
                            r = getInfoToken(path, SH_PAGES_IDX, &memUsageString); // get shared pages
                            if (r == -1) {
                                gtk_list_store_remove(GTK_LIST_STORE(store), &iter2);
                                g_value_unset(&value);
                                gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2);
                                continue;
                            }
                            memUsage = (memUsage - atoi(memUsageString)) * getpagesize(); // subtract shared pages from resident set size
                            if (memUsage == 0) strcpy(memUsageString, "N/A");
                            else if (memUsage >= MEGA) snprintf(memUsageString, MAX_STR, "%.1f MiB", memUsage/MEGA);
                            else if (memUsage >= KILO) snprintf(memUsageString, MAX_STR, "%.1f KiB", memUsage/KILO);
                            g_value_init(&newMEM, G_TYPE_STRING);
                            g_value_init(&newCPU, G_TYPE_INT);

                            // get total cpu time
                            cpproctimes[atoi(dir->d_name)][1] = getTotalCpuTimes();

                            //get cpu usage %
                            strcpy(path, "/proc/");
                            strcat(path, dir->d_name);
                            strcat(path, "/stat");
                            timeString = (char *) malloc(MAX_STR);
                            r = getInfoToken(path, UTIME_IDX, &timeString);
                            if (r == -1) {
                                gtk_list_store_remove(GTK_LIST_STORE(store), &iter2);
                                g_value_unset(&value);
                                gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2);
                                continue;
                            }
                            proctimes[atoi(dir->d_name)][1] = atoi(timeString);
                            r = getInfoToken(path, STIME_IDX, &timeString);
                            if (r == -1) {
                                gtk_list_store_remove(GTK_LIST_STORE(store), &iter2);
                                g_value_unset(&value);
                                gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2);
                                continue;
                            }
                            proctimes[atoi(dir->d_name)][1] += atoi(timeString);

                            g_value_set_string(&newMEM, memUsageString);
                            g_value_set_int(&newCPU, 100*((double)(proctimes[atoi(dir->d_name)][1] - proctimes[atoi(dir->d_name)][0])
                            / (double)(cpproctimes[atoi(dir->d_name)][1]-cpproctimes[atoi(dir->d_name)][0])));

                            // store new memory/cpu usage in row
                            gtk_list_store_set_value(store, &iter2, MEM_USED, &newMEM);
                            gtk_list_store_set_value(store, &iter2, PERC_CPU, &newCPU);
                            proctimes[atoi(dir->d_name)][0] = proctimes[atoi(dir->d_name)][1];
                            cpproctimes[atoi(dir->d_name)][0] = cpproctimes[atoi(dir->d_name)][1];

                            g_value_unset(&newMEM);
                            g_value_unset(&newCPU);
                            g_value_unset(&value);
                            free(memUsageString);
                            free(timeString);
                            break;
                        }
                        g_value_unset(&value);
                        gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2);
                    }
                    if (doubleEntry) {
                        doubleEntry = 0;
                        continue;
                    }
                    // add new process
                    // get process name
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);
                    strcat(path, "/status");
                    procName = (char *) malloc(MAX_STR);
                    getInfoToken(path, 2, &procName);
                    // get owner name
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);
                    stat(path, &stats);
                    ownerName = getpwuid(stats.st_uid)->pw_name;
                    // get memory usage
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);
                    strcat(path, "/statm");
                    memUsageString = (char *) malloc(MAX_STR);
                    getInfoToken(path, RSS_IDX, &memUsageString); // get resident set size
                    memUsage = atoi(memUsageString); //convert to bytes
                    getInfoToken(path, SH_PAGES_IDX, &memUsageString); // get shared pages
                    memUsage = (memUsage - atoi(memUsageString)) * getpagesize(); // subtract shared pages from resident set size
                    if (memUsage == 0) strcpy(memUsageString, "N/A");
                    else if (memUsage >= MEGA) snprintf(memUsageString, MAX_STR, "%.1f MiB", memUsage/MEGA);
                    else if (memUsage >= KILO) snprintf(memUsageString, MAX_STR, "%.1f KiB", memUsage/KILO);
                    // get cpu usage %
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);
                    strcat(path, "/stat");
                    timeString = (char *) malloc(MAX_STR);
                    getInfoToken(path, UTIME_IDX, &timeString);
                    proctimes[atoi(dir->d_name)][0] = atoi(timeString);
                    getInfoToken(path, STIME_IDX, &timeString);
                    proctimes[atoi(dir->d_name)][0] += atoi(timeString);
                    // determine appropriate icon
                    if (atoi(dir->d_name) <= RAT) image = image2;
                    else if (atoi(dir->d_name) <= PIGEON) image = image1;
                    else image = image3;

                    // store new process entry
                    gtk_list_store_append (store, &iter);
                    gtk_list_store_set (store, &iter,
                        ICON, image,
                        PROC_NAME, procName,
                        USER_NAME, ownerName,
                        PERC_CPU, 0,
                        PROC_ID, atoi(dir->d_name),
                        MEM_USED, memUsageString,
                        MEM_ACTUAL, (gint)memUsage, -1);

                        free(procName);
                        free(memUsageString);
                        free(timeString);
                }
            }
        }
        closedir(d);
    }
    return 1;
}

/* buildWindow()
*
* this function builds and displays the main application window, then sets the
* updateStore() function to be run every minute.
*/
static void buildWindow(GtkApplication *app, gpointer user_data){
    GtkWidget *window;
    GtkWidget *tree;
    GtkListStore *store = gtk_list_store_new (COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // create window
    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Process Monitor  -  2016 Edition!");
    gtk_window_set_default_size (GTK_WINDOW (window), MIN_DIM, MIN_DIM);

    // set minimum window size
    GdkGeometry hints;
    hints.min_width = MIN_DIM;
    hints.min_height = 0;
    gtk_window_set_geometry_hints(GTK_WINDOW(window), window, &hints, (GdkWindowHints) GDK_HINT_MIN_SIZE);

    // create a scrolled window
    GtkWidget* scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", ICON, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", PROC_NAME, NULL);
    gtk_tree_view_column_set_title(column, "Process Name");
    gtk_tree_view_column_set_sort_column_id(column, PROC_NAME);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, COL_WIDTH);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("User Name", renderer, "text", USER_NAME, NULL);
    gtk_tree_view_column_set_sort_column_id(column, USER_NAME);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, COL_WIDTH);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("% CPU", renderer, "text", PERC_CPU, NULL);
    gtk_tree_view_column_set_sort_column_id(column, PERC_CPU);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, COL_WIDTH);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("PID", renderer, "text", PROC_ID, NULL);
    gtk_tree_view_column_set_sort_column_id(column, PROC_ID);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, COL_WIDTH);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Memory Used", renderer, "text", MEM_USED, NULL);
    gtk_tree_view_column_set_sort_column_id(column, MEM_ACTUAL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, COL_WIDTH);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    // this column is purely here for sorting the "Memory Used" column (not visible)
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Memory Actual", renderer, "text", MEM_ACTUAL, NULL);
    gtk_tree_view_column_set_sort_column_id(column, MEM_ACTUAL);
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    // initialization run
    updateStore(store);

    gtk_container_add(GTK_CONTAINER(scroller), tree);
    gtk_container_add(GTK_CONTAINER(window), scroller);

    gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
    gtk_widget_show_all (window);

    g_timeout_add(ONE_SECOND, (GSourceFunc) updateStore, (gpointer) store);

}

int main (int argc, char **argv){
    GtkApplication *prog;
    int status;
    
    prog = gtk_application_new ("proc.monitor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (prog, "activate", G_CALLBACK (buildWindow), NULL);
    status = g_application_run (G_APPLICATION (prog), argc, argv);
    g_object_unref (prog);

    return status;
}
