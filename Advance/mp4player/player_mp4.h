#ifndef _PLAYER_MP4_H_
#define _PLAYER_MP4_H_
//import gstreamer
#include <gst/gst.h>

//dump dot
#define DOT_DUMP(PIPELINE)  do { \
                                gchar *file_name = NULL;\
                                file_name = g_strdup_printf("%s::%s():%d",__FILE__,__func__ ,__LINE__);\
                                gst_debug_bin_to_dot_file(GST_BIN (PIPELINE),GST_DEBUG_GRAPH_SHOW_ALL,file_name);\
                                g_free(file_name);\
                            }while(0);

typedef struct _CustomData {
	GMainLoop  *loop;           /* main loop */
	GstElement *pipeline;       /* player pipeline */
	GstElement *source;         /* file source */
	GstElement *demuxer;        /* media demuxer */
	//video
	GstElement *video_queue;    /*video queue*/
	GstElement *video_decoder;  /*vide decoder*/
	GstElement *video_convert;  /*vide convert*/
	GstElement *video_show;     /*vide show*/
	//audio
	GstElement *audio_queue;    /*audio queue*/
	GstElement *audio_decoder;  /*audio decoder*/
	GstElement *audio_convert;  /*audio convert*/
	GstElement *audio_show;     /*audio show*/

    //subtitle
 //    GstElement *subtitle_queue;    /*subtitle queue*/
	// GstElement *subtitle_decoder;  /*subtitle decoder*/
	// GstElement *subtitle_convert;  /*subtitle convert*/
	// GstElement *subtitle_show;     /*subtitle show*/

    //current media stream info
    gint        n_video;       /* the num of video stream */
    gint        n_audio;       /* the num of audio stream */
    gint        n_subtitle;    /* the num of subtitle stream */
    
    gint        c_video;       /* cur video stream */
    gint        c_audio;       /* cur audio stream */
    gint        c_subtitle;    /* cur subtitle stream */

    GstState    cur_state;      /* Current state of the pipeline */
    gint64      duration;       /* How long does this media last, in nanoseconds */
    gint64      postion;        /* current pos, in nanoseconds */
} CustomData;


void init_CustomData(CustomData *user_data);

//handle err msg
void call_back_Err (GstBus *bus, GstMessage *msg, CustomData *user_data);
//handle eos msg
void call_back_Eos (GstBus *bus, GstMessage *msg, CustomData *user_data);
//handle state_change msg
void call_back_state_changed (GstBus *bus, GstMessage *msg, CustomData *user_data);


//watch demux pad-added signal
void qtdemux_pad_added_handle(GstElement *obj, GstPad *new_pad, CustomData *user_data);

#endif