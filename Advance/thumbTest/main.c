#include "thumbnail.h"

int main(int argc, char *argv[])
{
    GstStateChangeReturn ret;
    Thumb_t user_data;
    GstBus *bus;

    //for uri
    gchar *uri = "/home/zhangqianbo/Desktop/Gstreamer/out.mp4"; //default
    if (argc > 1) {
        uri = "";
        uri = g_strdup (argv[1]);
    }
    g_print("play :%s\n", uri);

    /* Initialize GStreamer */
    g_print("Init pipeline\n");
    gst_init (&argc, &argv);

    reset(&user_data);
    user_data.loop = g_main_loop_new(NULL, FALSE);
    //create element
    user_data.pipeline = gst_pipeline_new ("thumbnail-pipeline");
    user_data.source = gst_element_factory_make("filesrc", "source");
    user_data.typefind = gst_element_factory_make("typefind", "typefind_demux");
    //add to bin
    gst_bin_add_many (GST_BIN(user_data.pipeline), user_data.source, user_data.typefind, NULL);
    //link
    if (gst_element_link(user_data.source, user_data.typefind))
    {
        g_print("source ===>> typefind success.\n");
    } else {
        gst_object_unref (user_data.pipeline);
        return -1;
    }
    //set property
    g_object_set(G_OBJECT(user_data.source), "location", uri, NULL);
    g_signal_connect(G_OBJECT(user_data.typefind), "have-type", G_CALLBACK(type_found), &user_data);

    //watch bus
    bus = gst_element_get_bus (user_data.pipeline);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT(bus), "message::error", G_CALLBACK (event_notify), &user_data);
    g_signal_connect (G_OBJECT(bus), "message::eos", G_CALLBACK (event_notify), &user_data);
    g_signal_connect (G_OBJECT(bus), "message::state-changed", G_CALLBACK (state_changed), &user_data);

    //set playing
    g_print("set playing.\n");
    ret = gst_element_set_state (user_data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (user_data.pipeline);
        return -1;
    }

    /*GMainLoop run*/
    g_print("set mainloop run\n");
    g_main_loop_run(user_data.loop);

    //Free
    gst_object_unref (bus);
    gst_element_set_state (user_data.pipeline, GST_STATE_NULL);
    gst_object_unref (user_data.pipeline);
    return 0;
}