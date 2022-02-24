#include "thumbnail.h"
#include "log.h"

#undef CLASSNAME
#define CLASSNAME "MAIN"

/**
 * @breif: create pipeline async
 * @param[in] uri: local file/dlna file
 * @param[in] width: thumbnail width
 * @param[in] height: thumbnail height
 * @param[in] handle: user data
 * 
 * @return: 0-->success; 1--->fail
 */  
int Thumb_InitAsync(gchar *uri, int width, int height, Thumb_t* handle);

/**
 * @breif: get thumbnail at position
 * @param[in] pos: seek to @pos ms
 * @param[in] handle: user data
 * 
 * @return: 0-->success; 1--->fail
 */  
int Thumb_GetFrameAtPos(int pos, Thumb_t* handle);

/**
 * @breif: destroy the pipeline
 * @param[in] handle: user data
 * 
 * @return: 0-->success; 1--->fail
 */  
int Thumb_Deinit(Thumb_t* handle);


int main(int argc, char *argv[])
{
    /* Initialize GStreamer */
    LOG_PRINTF(INFO,"Init pipeline");
    gst_init (&argc, &argv);

    Thumb_t thumb;
    
    gchar *uri = "file:///home/zhangqianbo/Desktop/Gstreamer/01Stay With Me.mp4"; //default
    if (argc > 1) {
        if (gst_uri_is_valid (argv[1])) {
            uri = g_strdup (argv[1]);
        } else {
            uri = gst_filename_to_uri (argv[1], NULL);
        }
    }

    Thumb_InitAsync(uri, 480, 360, &thumb);

    while(thumb.tState != STATE_PREPARED)
    {
        LOG_PRINTF(INFO, "wait prepared state!!!");
    }

    Thumb_GetFrameAtPos(5 * 1000, &thumb);

    // while(thumb.isGot != TRUE)
    // {
    //     LOG_PRINTF(INFO, "wait buffer!!!");
    // }
    
    Thumb_Deinit(&thumb);

    for(;;);

    return 0;
}


/**
 * @breif: create pipeline async
 * @param[in] uri: local file/dlna file
 * @param[in] width: thumbnail width
 * @param[in] height: thumbnail height
 * 
 * @return: 0-->success; 1--->fail
 */  
int Thumb_InitAsync(gchar *uri, int width, int height, Thumb_t* handle)
{
    LOG_PRINTF(ALWAYS, "init :%s, width:%d, height:%d", uri, width, height);
    //reset the handle
    reset( handle );

    handle->tState = STATE_PREPARING;
    //create element
    handle->pipeline = gst_pipeline_new ("thumbnail-pipeline");
    handle->source = gst_element_factory_make("urisourcebin", "source");
    handle->typefind = gst_element_factory_make("typefind", "typefind_demux");
    //add to bin
    gst_bin_add_many (GST_BIN(handle->pipeline), handle->source, handle->typefind, NULL);
    //set property
    g_object_set(G_OBJECT(handle->source), "uri", uri, NULL);
    g_signal_connect(G_OBJECT(handle->typefind), "have-type", G_CALLBACK(type_found), handle);
    g_signal_connect(G_OBJECT(handle->source), "pad-added", G_CALLBACK(sourcebin_pad_added), handle);

    //create a thread
    handle->p_thread = g_thread_new("thumbnail player thread", THUMBNAIL_PLAYER_THREAD, handle);

    //set pause
    //set playing
    LOG_PRINTF(ALWAYS, "set PAUSED.");
    GstStateChangeReturn ret = gst_element_set_state (handle->pipeline, GST_STATE_PAUSED);
    // GstStateChangeReturn ret = gst_element_set_state (handle->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LOG_PRINTF(ERROR, "Unable to set the pipeline to the playing state.");
        gst_object_unref (handle->pipeline);
        return 1;
    }

    return 0;
}

/**
 * @breif: get thumbnail at position
 * @param[in] pos: seek to @pos ms
 * @param[in] handle: user data
 * 
 * @return: 0-->success; 1--->fail
 */  
int Thumb_GetFrameAtPos(int pos, Thumb_t* handle)
{
    LOG_PRINTF(ALWAYS, "seek to %d ms", pos);
    //prepare to seek
    GstEvent *seek = gst_event_new_seek(1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, pos * GST_MSECOND,
        GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE);

    if(! gst_element_send_event(handle->pipeline, seek))
    {
        LOG_PRINTF(ERROR, "seek to %d ms fail.", pos);
    }

    LOG_PRINTF(ALWAYS, "seek to %d ms success.", pos);
    handle->isSeeked = TRUE;

    GstSample *sample;
    // g_signal_emit_by_name (handle->appsink, "try-pull-preroll", 10000000, &sample);
    g_signal_emit_by_name (handle->appsink, "pull-preroll", &sample, NULL);
    if (sample) {
        GstBuffer *buffer;
        GstMapInfo map;
        GstCaps *caps;
        GstStructure *s;
        gint width, height;
 
        caps = gst_sample_get_caps (sample);
        if (!caps) {
            g_print ("could not get snapshot format\n");
            LOG_PRINTF(ERROR, "cannot get caps!!");
            return -1;
        }
        s = gst_caps_get_structure (caps, 0);
 
        /* we need to get the final caps on the buffer to get the size */
        gboolean res;
        res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res) {
            LOG_PRINTF(ERROR, "could not get snapshot dimension");
            return -1;
        }
        LOG_PRINTF(ALWAYS, "dimension:%d x %d.", width, height);
 
        buffer = gst_sample_get_buffer (sample);
    
        /* Mapping a buffer can fail (non-readable) */
        if (gst_buffer_map (buffer, &map, GST_MAP_READ)) {
            /* print the buffer data for debug */
            //save buffer
            char header[512];
            sprintf(header, "%dx%d.rgba", width, height);

            FILE *fp = fopen(header, "wb+");
            fwrite(map.data, 1, map.size, fp);
            fclose(fp);

            gst_buffer_unmap (buffer, &map);
        }
        gst_sample_unref (sample);
    } else {
        LOG_PRINTF(ERROR, "sample is null");
    }

    return 0;
}

/**
 * @breif: destroy the pipeline
 * @param[in] handle: user data
 * 
 * @return: 0-->success; 1--->fail
 */  
int Thumb_Deinit(Thumb_t* handle)
{
    //set NULL
    gst_element_set_state (handle->pipeline, GST_STATE_NULL);
    //quit main loop
    if(handle->loop)
    {
        LOG_PRINTF(ERROR, "quit loop");
        g_main_loop_quit (handle->loop);
        // g_main_loop_unref(handle->loop);
        // handle->loop = NULL;
    }

    //unref pipeline
    if(handle->pipeline)
        gst_object_unref (handle->pipeline);
    
    return 0;
}
