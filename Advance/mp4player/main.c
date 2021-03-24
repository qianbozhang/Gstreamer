#include <gst/gst.h>
#include "player_mp4.h"


int main(int argc, char *argv[])
{
    GstStateChangeReturn ret;
    CustomData user_data;
    GstBus *bus;
    gchar *uri = "../../mediafile/out.mp4";

    if (argc > 1) {
        uri = "";
        if (gst_uri_is_valid (argv[1])) {
            uri = g_strdup (argv[1]);
        } else {
            uri = gst_filename_to_uri (argv[1], NULL);
        }
    }

    g_print("play :%s\n", uri);

    //init data
    init_CustomData(&user_data);
    user_data.loop = g_main_loop_new(NULL, FALSE);

    /* Initialize GStreamer */
    g_print("Init pipeline\n");
    gst_init (&argc, &argv);

    //create element
    user_data.pipeline = gst_pipeline_new ("my_pipeline");
    user_data.source = gst_element_factory_make("filesrc", "sourcestream");
    user_data.demuxer = gst_element_factory_make("qtdemux", "demux_Quicktime");
    //video
    user_data.video_queue = gst_element_factory_make("queue", "video_queue");             /*video queue*/
    user_data.video_decoder = gst_element_factory_make("avdec_h264", "video_decoder");    /*vide decoder*/
    user_data.video_convert = gst_element_factory_make("videoconvert", "video_convert");  /*vide convert*/
    user_data.video_show = gst_element_factory_make("autovideosink", "video_show");       /*vide show*/
    //video
    user_data.audio_queue = gst_element_factory_make("queue", "audio_queue");             /*audio queue*/
    user_data.audio_decoder = gst_element_factory_make("avdec_aac", "audio_decoder");    /*audio decoder*/
    user_data.audio_convert = gst_element_factory_make("audioconvert", "audio_convert");  /*audio convert*/
    user_data.audio_show = gst_element_factory_make("autoaudiosink", "audio_show");       /*audio show*/
    //check element create success
    if ( !user_data.pipeline ||
            !user_data.source || !user_data.demuxer ||
            !user_data.video_queue || !user_data.video_decoder || !user_data.video_convert || !user_data.video_show ||
            !user_data.audio_queue || !user_data.audio_decoder || !user_data.audio_convert || !user_data.audio_show)
    {
        g_printerr("some element create fail\n");
        return -1;
    }

    //add element to bin
    gst_bin_add_many (GST_BIN(user_data.pipeline), user_data.source, user_data.demuxer,
                      user_data.video_queue, user_data.video_decoder, user_data.video_convert, user_data.video_show,
                      user_data.audio_queue, user_data.audio_decoder, user_data.audio_convert, user_data.audio_show,
                      NULL);

    //link element
    if (gst_element_link(user_data.source, user_data.demuxer) &&
            gst_element_link_many(user_data.video_queue, user_data.video_decoder, user_data.video_convert, user_data.video_show, NULL) &&
            gst_element_link_many(user_data.audio_queue, user_data.audio_decoder, user_data.audio_convert, user_data.audio_show, NULL) )
    {
        g_print("All element link success.\n");
    } else {
        g_printerr ("All element link fail.\n");
        gst_object_unref (user_data.pipeline);
        return -1;
    }

    //set element property
    g_object_set(G_OBJECT(user_data.source), "location", uri, NULL);

    //watch bus
    bus = gst_element_get_bus (user_data.pipeline);
    gst_bus_add_signal_watch (bus);

    g_signal_connect (G_OBJECT(bus), "message::error", G_CALLBACK (call_back_Err), &user_data);
    g_signal_connect (G_OBJECT(bus), "message::eos", G_CALLBACK (call_back_Eos), &user_data);
    g_signal_connect (G_OBJECT(bus), "message::state-changed", G_CALLBACK (call_back_state_changed), &user_data);

    /* Connect to the pad-added signal */
    g_signal_connect (G_OBJECT(user_data.demuxer), "pad-added", G_CALLBACK (qtdemux_pad_added_handle), &user_data);

    //refresh
    // g_timeout_add_seconds (1, (GSourceFunc)refresh_position, &data);


    /*dump dot*/
    DOT_DUMP(user_data.pipeline);

    //set playing
    g_print("set playing\n");
    ret = gst_element_set_state (user_data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (user_data.pipeline);
        return -1;
    }

    /*GMainLoop run*/
    //start play
    g_print("set mainloop run\n");
    g_main_loop_run(user_data.loop);

    //Free
    gst_object_unref (bus);
    gst_element_set_state (user_data.pipeline, GST_STATE_NULL);
    gst_object_unref (user_data.pipeline);
    return 0;
}