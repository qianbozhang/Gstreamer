#ifndef _THUMBNAIL_H_
#define _THUMBNAIL_H_
//include gst
#include <gst/gst.h>

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
 * @brief: update plugin factories
 * @param[in] user_data: means Thumb_t
 */
void update_factories_list(Thumb_t* thumb);

#endif