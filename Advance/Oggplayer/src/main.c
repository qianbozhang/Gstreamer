#include <gst/gst.h>
#include "player.h"
#include "dot.h"


int main(int argc, char *argv[])
{
    GstStateChangeReturn ret;
    CustomData user_data;
    GstBus *bus;
    gchar *uri = "/home/zhangqianbo/Desktop/GstreamerProject/Gstreamer/Advance/Oggplayer/video_subtitle.ogv";

    if (argc > 1) {
        uri = "";
        if (gst_uri_is_valid (argv[1])) {
            uri = g_strdup (argv[1]);
        } else {
            uri = gst_filename_to_uri (argv[1], NULL);
        }
    }

    g_print("play :%s\n", uri);


    /* Initialize GStreamer */
    g_print("Init pipeline\n");
    gst_init (&argc, &argv);

    //init data
    init_CustomData(&user_data);
    user_data.loop = g_main_loop_new(NULL, FALSE);

    
    //check element create success
    if ( !user_data.pipeline ||
            !user_data.source || !user_data.demuxer || !user_data.streamsync ||
            !user_data.video_queue || !user_data.video_decoder || !user_data.video_convert || !user_data.video_show ||
            !user_data.audio_queue || !user_data.audio_decoder || !user_data.audio_convert || !user_data.audio_show ||
            !user_data.sub_input_select || !user_data.sub_lay)
    {
        g_printerr("some element create fail\n");
        return -1;
    }

    //add element to bin
    gst_bin_add_many (GST_BIN(user_data.pipeline), user_data.source, user_data.demuxer, user_data.streamsync, user_data.multiqueue, 
                      user_data.video_queue, user_data.video_decoder, user_data.video_convert, user_data.video_show,
                      user_data.audio_queue, user_data.audio_decoder, user_data.audio_convert, user_data.audio_show,
                      user_data.sub_input_select, user_data.sub_queue, user_data.sub_lay,
                      user_data.sub_decoder0, user_data.sub_decoder1, user_data.sub_decoder2, user_data.sub_decoder3, user_data.sub_decoder4,
                      NULL);

    //link element
    if (!gst_element_link(user_data.source, user_data.demuxer)){
        g_printerr ("source <<==>> demuxer link fail.\n");
        gst_object_unref (user_data.pipeline);
        return -1;
    } 
    
    if (!gst_element_link_many(user_data.video_queue, user_data.video_decoder, NULL) ||
        !gst_element_link_many(user_data.sub_lay, user_data.video_convert, user_data.video_show, NULL) ){
        g_printerr ("video link fail.\n");
        gst_object_unref (user_data.pipeline);
        return -1;
    } 

   if (!gst_element_link_many(user_data.sub_input_select, user_data.sub_queue, NULL)){
        g_printerr ("sub link fail.\n");
        gst_object_unref (user_data.pipeline);
        return -1;
    } 


    if (!gst_element_link_many(user_data.audio_queue, user_data.audio_decoder, NULL) || 
        !gst_element_link_many(user_data.audio_convert, user_data.audio_show, NULL)){
        g_printerr ("audio link fail.\n");
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
    //demux
    g_signal_connect (G_OBJECT(user_data.demuxer), "pad-added", G_CALLBACK (pad_add_catch), &user_data);
    //sync
    g_signal_connect (G_OBJECT(user_data.streamsync), "pad-added", G_CALLBACK (streamsync_src), &user_data);

    g_signal_connect (G_OBJECT(user_data.multiqueue), "pad-added", G_CALLBACK (multiqueue_src), &user_data);

    //link sub_input
    link_sub_input_select(&user_data);

    //link sync
    link_streamsync(&user_data);

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