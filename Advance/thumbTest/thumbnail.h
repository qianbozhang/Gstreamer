#ifndef _THUMBNAIL_H_
#define _THUMBNAIL_H_
//include gst
#include <gst/gst.h>

typedef enum thumb_state
{
    STATE_IDLE = 0,
    STATE_PREPARING,
    STATE_PREPARED,
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_STOPPING,
    STATE_STOPPED
}Thumb_State;

typedef struct _thumbnail
{
    GstElement *pipeline;       /* player pipeline */
	GstElement *source;         /* file source */
	GstElement *typefind;       /* typefind */

    GMainLoop  *loop;           /* main loop */

    guint cookie;               /* factories change flag */
    GList *factories;           /* All DECODABLE factories */
    GList *decoder_factories;   /* Only DECODER factories */
    GList *decodable_factories; /* DECODABLE but not DECODER factories */

    gint width;
    gint height;

    gboolean isSeeked;
    gboolean isGot;

    GThread *p_thread;

    Thumb_State tState;
}Thumb_t;

/**
 * @brief: init thumb param
 * @param[in] thumb: thumbnail struct 
 */ 
void reset(Thumb_t* thumb);

/**
 * @brief: handle gstreamer state change msg
 * @param[in] bus: 
 * @param[in] msg: new state msg
 * @param[in] user_data: means Thumb_t
 */ 
void state_changed (GstBus *bus, GstMessage *msg, Thumb_t* user_data);

/**
 * @brief: handle gstreamer state change msg
 * @param[in] bus: 
 * @param[in] msg: new event msg
 * @param[in] user_data: means Thumb_t
 */
void event_notify (GstBus *bus, GstMessage *msg, Thumb_t* user_data);

/**
 * @brief: handle typefind plugin have-type signal
 * @param[in] typefind: the typefind instance
 * @param[in] probability: the probability of the type found
 * @param[in] caps: the caps of the type found
 * @param[in] user_data: means Thumb_t
 */
void type_found(GstElement* typefind, guint probability, GstCaps* caps, Thumb_t* user_data);

/**
 * @brief: handle demuxer pad-added signal
 * @param[in] typefind: the typefind instance
 * @param[in] newpad: the pad that has been added
 * @param[in] user_data: means Thumb_t
 */
void pad_added(GstElement *obj, GstPad *newpad, Thumb_t* user_data);

/**
 * @brief: handle urisourcebin pad-added signal
 * @param[in] typefind: the sourcebin instance
 * @param[in] newpad: the pad that has been added
 * @param[in] user_data: means Thumb_t
 */
void sourcebin_pad_added(GstElement *obj, GstPad *newpad, Thumb_t* user_data);

/**
 * @brief: update plugin factories
 * @param[in] user_data: means Thumb_t
 */
void update_factories_list(Thumb_t* thumb);

/**
 * @brief: thumbnail player thread
 * @param[in] user_data: means Thumb_t
 */
void THUMBNAIL_PLAYER_THREAD(void* handle);

/**
 * @brief: prob the sink buffer data
 * @param[in] pad: the prob pad
 * @param[in] info: buffer data
 * @param[in] user_data: user data
 * 
 * @return: --
 */
GstPadProbeReturn cb_have_data ( GstPad * pad, GstPadProbeInfo * info, gpointer user_data);


#endif