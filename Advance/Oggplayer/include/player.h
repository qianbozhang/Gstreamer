#ifndef _PLAYER_MP4_H_
#define _PLAYER_MP4_H_
//import gstreamer
#include <gst/gst.h>

typedef enum _MEDIATYPE
{
	MEDIA_TYPE_VIDEO = 0,
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_SUB
	
}Mediatype_e;

typedef struct _CustomData {
	GMainLoop  *loop;           /* main loop */
	GstElement *pipeline;       /* player pipeline */
	GstElement *source;         /* file source */
	GstElement *demuxer;        /* media demuxer */
	GstElement *streamsync;     /* media sync */
	GstElement *multiqueue;     /* media sync */
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
    GstElement *sub_decoder0;         /*subtitle decoder*/
    GstElement *sub_decoder1;         /*subtitle decoder*/
    GstElement *sub_decoder2;         /*subtitle decoder*/
    GstElement *sub_decoder3;         /*subtitle decoder*/
    GstElement *sub_decoder4;         /*subtitle decoder*/
    GstElement *sub_input_select;    /*subtitle select*/
    GstElement *sub_queue;           /*subtitle queue*/
	GstElement *sub_lay;             /*subtitle convert*/

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

    Mediatype_e  cur_type;
} CustomData;


void init_CustomData(CustomData *user_data);

//handle err msg
void call_back_Err (GstBus *bus, GstMessage *msg, CustomData *user_data);
//handle eos msg
void call_back_Eos (GstBus *bus, GstMessage *msg, CustomData *user_data);
//handle state_change msg
void call_back_state_changed (GstBus *bus, GstMessage *msg, CustomData *user_data);


//watch demux pad-added signal
void pad_add_catch(GstElement *obj, GstPad *new_pad, CustomData *user_data);

void streamsync_src(GstElement *obj, GstPad *new_pad, CustomData *user_data);

void multiqueue_src(GstElement *obj, GstPad *new_pad, CustomData *user_data);


//link streamsync
void link_streamsync(CustomData *user_data);

//link sub_input_select
void link_sub_input_select(CustomData *user_data);

void link_sub_dec(CustomData *user_data, GstPad *new_pad);

void dot_dump(GstElement *obj);


#endif