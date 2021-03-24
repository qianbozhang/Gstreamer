#include "player_mp4.h"
#include <gst/gst.h>

void init_CustomData(CustomData *user_data)
{
	//memset(user_data, 0 , sizeof(user_data));

	user_data->c_video = -1;
	user_data->c_audio = -1;
	user_data->c_subtitle = -1;
}

//handle err msg
void call_back_Err (GstBus *bus, GstMessage *msg, CustomData *user_data)
{
	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_ERROR: {
		GError *err;
		gchar *debug;

		gst_message_parse_error (msg, &err, &debug);
		g_print ("Rev Error: %s\n", err->message);
		g_error_free (err);
		g_free (debug);

		gst_element_set_state (user_data->pipeline, GST_STATE_READY);
		g_main_loop_quit (user_data->loop);
		break;
	}
	default:
		/* Unhandled message */
		break;
	}

}

//handle eos msg
void call_back_Eos (GstBus *bus, GstMessage *msg, CustomData *user_data)
{
	switch (GST_MESSAGE_TYPE (msg)) {

	case GST_MESSAGE_EOS:
		/* end-of-stream */
		g_print ("Rev Eos-of-stream.\n");
		gst_element_set_state (user_data->pipeline, GST_STATE_READY);
		g_main_loop_quit (user_data->loop);
		break;
	default:
		/* Unhandled message */
		break;
	}

}

//handle state_change msg
void call_back_state_changed (GstBus *bus, GstMessage *msg, CustomData *user_data)
{
	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_BUFFERING: {
		break;
	}
	case GST_MESSAGE_CLOCK_LOST:
		/* Get a new clock */
		g_print ("Rev GST_MESSAGE_CLOCK_LOST\n");
		gst_element_set_state (user_data->pipeline, GST_STATE_PAUSED);
		gst_element_set_state (user_data->pipeline, GST_STATE_PLAYING);
		break;
	case GST_MESSAGE_STATE_CHANGED:
		/* We are only interested in state-changed messages from the pipeline */
		if (GST_MESSAGE_SRC (msg) == GST_OBJECT (user_data->pipeline)) {
			GstState old_state, new_state, pending_state;
			gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
			g_print ("Pipeline state changed from %s to %s:\n", gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));

			//update state
			user_data->cur_state = new_state;

			if ( new_state == GST_STATE_PLAYING ) {
				/* Read some properties */
				g_object_get (user_data->pipeline, "n-video", &user_data->n_video, NULL);
				g_object_get (user_data->pipeline, "n-audio", &user_data->n_audio, NULL);
				g_object_get (user_data->pipeline, "n-text", &user_data->n_subtitle, NULL);

				g_print ("%d video stream(s), %d audio stream(s), %d text stream(s)\n", user_data->n_video, user_data->n_audio, user_data->n_subtitle);

			}
		}
		break;
	default:
		/* Unhandled message */
		break;
	}

}

//watch demux pad-added signal
void qtdemux_pad_added_handle(GstElement *obj, GstPad *new_pad, CustomData *user_data)
{
	gchar *new_pad_caps_name = NULL;
	GstPad *v_sinkpad = NULL;
	GstPad *a_sinkpad = NULL;

	g_object_get(new_pad, "name", &new_pad_caps_name, NULL);
	g_print("Rev demuxer' pad(%s).\n", new_pad_caps_name);

	v_sinkpad = gst_element_get_static_pad (user_data->video_queue, "sink");
	a_sinkpad = gst_element_get_static_pad (user_data->audio_queue, "sink");

	gboolean link_video = gst_pad_is_linked (v_sinkpad);
	gboolean link_audio = gst_pad_is_linked (a_sinkpad);

	if (link_video && link_audio) {
		g_print("audio/video already linked. ignore new_pad(%s).\n", new_pad_caps_name);
		goto FREE;
	}

	if (g_str_has_prefix(new_pad_caps_name, "video") && !link_video )
	{
		if (!gst_pad_link (new_pad, v_sinkpad))
			g_print("%s linked to vdecoder.\n", new_pad_caps_name);
		else
			g_print("ERROR: gst_pad_link (new_pad, v_sinkpad), pad_name = %s\n", new_pad_caps_name);
	}

	if (g_str_has_prefix(new_pad_caps_name, "audio") && !link_audio )
	{
		if (!gst_pad_link (new_pad, a_sinkpad))
			g_print("%s linked to adecoder.\n", new_pad_caps_name);
		else
			g_print("ERROR: gst_pad_link (new_pad, a_sinkpad), pad_name = %s\n", new_pad_caps_name);
	}

FREE:
	gst_object_unref (a_sinkpad);
	gst_object_unref (v_sinkpad);
}