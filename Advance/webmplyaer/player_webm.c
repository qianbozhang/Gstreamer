#include <gst/gst.h>
#include "../../dot.h"

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GMainLoop  *loop;
  //pipeline
  GstElement *pipeline;
  GstElement *source;
  GstElement *demuxer;
  /*audio pipeline*/
  GstElement *aqueue;
  GstElement *adecoder;
  GstElement *aconvert;
  GstElement *asink;
  /*video pipeline*/
  GstElement *vqueue;
  GstElement *vcoder;
  GstElement *vconvert;
  GstElement *vsink;
} CustomData;


/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);
static gboolean  printCaps(GQuark field_id, GValue * value, gpointer user_data);
static void cb_message (GstBus *bus, GstMessage *msg, CustomData *data);

int main(int argc, char *argv[]) {
  CustomData data;
  GstBus *bus;
  GstStateChangeReturn ret;
  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("my_pipeline");
  //source
  data.source = gst_element_factory_make ("filesrc", "source");
  data.demuxer = gst_element_factory_make ("matroskademux", "demuxer");
  //audio
  data.aqueue = gst_element_factory_make ("queue", "aqueue");
  data.adecoder = gst_element_factory_make ("vorbisdec", NULL);
  data.aconvert = gst_element_factory_make ("audioconvert", "convert");
  data.asink = gst_element_factory_make ("autoaudiosink", "sink");
  //video
  data.vqueue = gst_element_factory_make ("queue", "vueue");
  data.vcoder = gst_element_factory_make ("vp8dec", "vcoder");
  data.vconvert = gst_element_factory_make ("videoconvert", NULL);
  //data.videosink = gst_element_factory_make ("ximagesink", "vsink");
  data.vsink = gst_element_factory_make ("autovideosink", NULL);



  if (  !data.pipeline || !data.source || !data.demuxer
      || !data.aqueue || !data.adecoder || !data.aconvert || !data.asink
      || !data.vqueue || !data.vcoder || !data.vconvert || !data.vsink ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }


  /* Build the pipeline. Note that we are NOT linking the source at this
   * point. We will do it later. */
  gst_bin_add_many (GST_BIN (data.pipeline),
                    data.source, data.demuxer,
                    data.vqueue, data.vcoder, data.vconvert, data.vsink,
                    data.aqueue, data.adecoder, data.aconvert, data.asink,
                    NULL);

  //link all element.
  if ( gst_element_link ( data.source, data.demuxer ) &&
       gst_element_link_many ( data.vqueue, data.vcoder, data.vconvert, data.vsink, NULL )  &&
       gst_element_link_many ( data.aqueue, data.adecoder, data.aconvert, data.asink, NULL )){
      g_print("ALL linked ok.\n");
  }

  DOT_DUMP(data.pipeline);

  bus = gst_element_get_bus (data.pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT(bus), "message", G_CALLBACK (cb_message), &data);

  /* Connect to the pad-added signal */
  g_signal_connect (data.demuxer, "pad-added", G_CALLBACK (pad_added_handler), &data);

  /* Set the URI to play */
  g_object_set (G_OBJECT(data.source), "location", "../mediafile/video.webm", NULL);

  //DOT_DUMP(data.pipeline);
  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  //loop
  data.loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(data.loop);


  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *element, GstPad *pad, CustomData *data) {
  gchar *pad_name = NULL;
  GstPad *v_sinkpad = NULL;
  GstPad *a_sinkpad = NULL;

  g_object_get(pad, "name", &pad_name, NULL);
  g_print("Rev demuxer' pad(%s).\n", pad_name);
  
  v_sinkpad = gst_element_get_static_pad (data->vqueue, "sink");
  a_sinkpad = gst_element_get_static_pad (data->aqueue, "sink");

  gboolean link_video = gst_pad_is_linked (v_sinkpad);
  gboolean link_audio = gst_pad_is_linked (a_sinkpad);

  if (g_str_has_prefix(pad_name, "video") && !link_video )
  {
    if (!gst_pad_link (pad, v_sinkpad))
      g_print("%s linked to vdecoder.\n", pad_name);
    else
      g_print("ERROR: gst_pad_link (pad, v_sinkpad), pad_name = %s\n", pad_name);
  }

  if (g_str_has_prefix(pad_name, "audio") && !link_audio )
  {
    if (!gst_pad_link (pad, a_sinkpad))
      g_print("%s linked to adecoder.\n", pad_name);
    else
      g_print("ERROR: gst_pad_link (pad, a_sinkpad), pad_name = %s\n", pad_name);
  }

   gst_object_unref (a_sinkpad);
   gst_object_unref (v_sinkpad);

}


static gboolean  printCaps(GQuark field_id, GValue * value, gpointer user_data)
{
  gchar *str = gst_value_serialize (value);
  g_print ("%s : %s.\n", g_quark_to_string(field_id), str);
  g_free(str);
  return TRUE;
}

static void cb_message (GstBus *bus, GstMessage *msg, CustomData *data) {

  switch (GST_MESSAGE_TYPE (msg)) {
  case GST_MESSAGE_ERROR: {
    GError *err;
    gchar *debug;

    gst_message_parse_error (msg, &err, &debug);
    g_print ("Rev Error: %s\n", err->message);
    g_error_free (err);
    g_free (debug);

    gst_element_set_state (data->pipeline, GST_STATE_READY);
    g_main_loop_quit (data->loop);
    break;
  }
  case GST_MESSAGE_EOS:
    /* end-of-stream */
  g_print ("Rev Eos-of-stream.\n");
    gst_element_set_state (data->pipeline, GST_STATE_READY);
    g_main_loop_quit (data->loop);
    break;
  case GST_MESSAGE_BUFFERING: {
    break;
  }
  case GST_MESSAGE_CLOCK_LOST:
    /* Get a new clock */
    g_print ("Rev GST_MESSAGE_CLOCK_LOST\n");
    gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
    break;
  case GST_MESSAGE_STATE_CHANGED:
    /* We are only interested in state-changed messages from the pipeline */
    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline)) {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
      g_print ("Pipeline state changed from %s to %s:\n",
               gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
      DOT_DUMP(data->pipeline);
    }
    break;
  default:
    /* Unhandled message */
    break;
  }
}