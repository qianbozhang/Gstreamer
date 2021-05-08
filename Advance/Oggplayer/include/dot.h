#ifndef _DOT_H_
#define _DOT_H_

//#define DOT_DUMP(PIPELINE) (gst_debug_bin_to_dot_file_with_ts(GST_BIN (PIPELINE),GST_DEBUG_GRAPH_SHOW_ALL,DOT_NAME))
#define DOT_DUMP(PIPELINE)  do { \
                                gchar *file_name = NULL;\
                                file_name = g_strdup_printf("%s():%d" ,__func__ ,__LINE__);\
                                gst_debug_bin_to_dot_file(GST_BIN (PIPELINE),GST_DEBUG_GRAPH_SHOW_ALL,file_name);\
                                g_free(file_name);\
                            }while(0);


#endif