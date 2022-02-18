#include "thumbnail.h"
#include "log.h"

#undef CLASSNAME
#define CLASSNAME "THUMBNAIL"

/**
 * @brief: init thumb param
 * @param[in] thumb: thumbnail struct 
 */ 
void reset(Thumb_t* thumb)
{

    thumb->pipeline = NULL;
    thumb->source = NULL;
    thumb->typefind = NULL;
    thumb->loop = NULL;

    thumb->cookie = 0;
    thumb->factories = NULL;
    thumb->decoder_factories = NULL;
    thumb->decodable_factories = NULL;
}

/**
 * @brief: handle gstreamer state change msg
 * @param[in] bus: 
 * @param[in] msg: new state msg
 * @param[in] user_data: means Thumb_t
 */ 
void state_changed (GstBus *bus, GstMessage *msg, Thumb_t* user_data)
{
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_BUFFERING:
        {
            break;
        }
        case GST_MESSAGE_CLOCK_LOST:
        {
            /* Get a new clock */
            LOG_PRINTF(ALWAYS, "Rev GST_MESSAGE_CLOCK_LOST");
            gst_element_set_state (user_data->pipeline, GST_STATE_PAUSED);
            gst_element_set_state (user_data->pipeline, GST_STATE_PLAYING);
            break;
        }  
        case GST_MESSAGE_STATE_CHANGED:
        {
            /* We are only interested in state-changed messages from the pipeline */
            if (GST_MESSAGE_SRC (msg) == GST_OBJECT (user_data->pipeline)) {
                GstState old_state, new_state, pending_state;
                gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
                LOG_PRINTF(ALWAYS, "Pipeline state changed from %s to %s pending:%s.", 
                    gst_element_state_get_name (old_state),
                    gst_element_state_get_name (new_state),
                    gst_element_state_get_name (pending_state));

                if(new_state == GST_STATE_PLAYING)
                {
                    gst_debug_bin_to_dot_file_with_ts(GST_BIN (user_data->pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"playing");
                }
            }
            break;
        }
        default:
        {
            /* Unhandled message */
            break;
        }  
    }
}

/**
 * @brief: handle gstreamer state change msg
 * @param[in] bus: 
 * @param[in] msg: new event msg
 * @param[in] user_data: means Thumb_t
 */
void event_notify (GstBus *bus, GstMessage *msg, Thumb_t* user_data)
{
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
        {
            /* end-of-stream */
            LOG_PRINTF(ALWAYS, "Rev Eos-of-stream.");
            gst_element_set_state (user_data->pipeline, GST_STATE_READY);
            g_main_loop_quit (user_data->loop);
            break;
        }
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;

            gst_message_parse_error (msg, &err, &debug);
            LOG_PRINTF(ALWAYS, "Rev Error: %s.", err->message);
            g_error_free (err);
            g_free (debug);

            gst_element_set_state (user_data->pipeline, GST_STATE_READY);
            g_main_loop_quit (user_data->loop);
            break;
        }
        default:
        {
            /* Unhandled message */
            break;
        }
    }
}

/**
 * @brief: handle typefind plugin have-type signal
 * @param[in] typefind: the typefind instance
 * @param[in] probability: the probability of the type found
 * @param[in] caps: the caps of the type found
 * @param[in] user_data: means Thumb_t
 */
void type_found(GstElement* typefind, guint probability, GstCaps* caps, Thumb_t* user_data)
{
    LOG_PRINTF(ALWAYS, "rev probability:%u, caps: %s.", probability, gst_caps_to_string (caps));
    GValueArray *factories = NULL;
    GList *list, *tmp;
    GstPad* srcpad;
    //start analyze srcpad & caps
    if(!gst_caps_is_fixed(caps))
    {
        LOG_PRINTF(ERROR, "caps is not fixed.");
        return;
    }
    //find factories
    update_factories_list(user_data);
    {
        list = gst_element_factory_list_filter (user_data->factories, caps, GST_PAD_SINK, gst_caps_is_fixed (caps));
        factories = g_value_array_new (g_list_length (list));
        for (tmp = list; tmp; tmp = tmp->next) {
            GstElementFactory *ft = GST_ELEMENT_FACTORY_CAST (tmp->data);
            GValue val = { 0, };

            g_value_init (&val, G_TYPE_OBJECT);
            g_value_set_object (&val, ft);
            g_value_array_append (factories, &val);
            g_value_unset (&val);
        }
        gst_plugin_feature_list_free (list);
    }

    if(factories->n_values == 0)
    {
        g_value_array_free (factories);
        LOG_PRINTF(ERROR,"cannot find demux.");
        return;
    }else
    {
        LOG_PRINTF(ERROR,"element factories n_values = %d.", factories->n_values);
    }

    //try to create an element and link it 
    while(factories->n_values > 0)
    {
        GstElementFactory *factory;
        GstElement *element;
        GstPad *sinkpad = NULL;

        /* take first factory */
        factory = g_value_get_object (g_value_array_get_nth (factories, 0));
        /* Remove selected factory from the list. */
        g_value_array_remove (factories, 0);
        LOG_PRINTF(INFO, "trying factory %p.", factory);
        //condition 1: caps is subset of the factory sink pad caps ?
        if (gst_caps_is_fixed (caps)) {
            const GList *templs;
            gboolean skip = FALSE;

            templs = gst_element_factory_get_static_pad_templates (factory);
            while (templs) {
                GstStaticPadTemplate *templ = (GstStaticPadTemplate *) templs->data;
                if (templ->direction == GST_PAD_SINK) {
                    GstCaps *templcaps = gst_static_caps_get (&templ->static_caps);
                    if (!gst_caps_is_subset (caps, templcaps)) {
                        LOG_PRINTF(ERROR, "caps %s not subset of %s.", gst_caps_to_string(caps), gst_caps_to_string(templcaps));
                        gst_caps_unref (templcaps);
                        skip = TRUE;
                        break;
                    }
                    gst_caps_unref (templcaps);
                }
                templs = g_list_next (templs);
            }
            if (skip)
                continue;
        }

        //condition 2: is a demuxer factory ?

        //create element
        LOG_PRINTF(INFO, "prepare to create demuxer.");
        if ((element = gst_element_factory_create (factory, NULL)) == NULL) {
            LOG_PRINTF(ERROR, "Could not create an element from %s", gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)));
            continue;
        }

        //add element to bin
        if (!(gst_bin_add (GST_BIN_CAST (user_data->pipeline), element))) {
            LOG_PRINTF(ERROR, "Couldn't add %s to the bin", GST_ELEMENT_NAME (element));
            gst_object_unref (element);
            continue;
        }

        //got element sink pad
        if (element->sinkpads != NULL)
            sinkpad = gst_object_ref (element->sinkpads->data);
        if (sinkpad == NULL) {
            LOG_PRINTF(ERROR, "Element %s doesn't have a sink pad", GST_ELEMENT_NAME (element));
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }

        //try link
        srcpad = gst_element_get_static_pad(typefind, "src");
        if ((gst_pad_link_full (srcpad, sinkpad, GST_PAD_LINK_CHECK_NOTHING)) != GST_PAD_LINK_OK) {
            LOG_PRINTF(ERROR, "Link failed on pad %s:%s", GST_DEBUG_PAD_NAME (sinkpad));
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }
        LOG_PRINTF(INFO, "link success .");

        //active it to ready
        if ((gst_element_set_state (element, GST_STATE_READY)) == GST_STATE_CHANGE_FAILURE) {
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }
        LOG_PRINTF(INFO, "NULL ==>> READY .");

        /* check if we still accept the caps on the pad after setting
        * the element to READY */
        if (!gst_pad_query_accept_caps (sinkpad, caps)) {
            LOG_PRINTF(ERROR, "Element %s does not accept caps", GST_ELEMENT_NAME (element));
            gst_element_set_state (element, GST_STATE_NULL);
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }

        gst_object_unref (sinkpad);
        LOG_PRINTF(ALWAYS, "linked on pad %s:%s.", GST_DEBUG_PAD_NAME (srcpad));

        //link the element further
        g_signal_connect (element, "pad-added", G_CALLBACK (pad_added), user_data);
        //ready ==>> paused
        if ((gst_element_set_state (element, GST_STATE_PAUSED)) == GST_STATE_CHANGE_FAILURE) {
            gst_element_set_state (element, GST_STATE_NULL);
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }
        LOG_PRINTF(INFO, "READY ==>> PAUSED .");
        break;
    }
    g_value_array_free (factories);
}

/**
 * @brief: handle demuxer pad-added signal
 * @param[in] typefind: the typefind instance
 * @param[in] newpad: the pad that has been added
 * @param[in] user_data: means Thumb_t
 */
void pad_added(GstElement *obj, GstPad *newpad, Thumb_t* user_data)
{
    gchar *new_pad_caps_name = NULL;
    GstPad *v_sinkpad = NULL;
    GList *factories;   //all decode factories
    GstElement *element = NULL; //decode
    GstCaps *caps = gst_pad_get_current_caps (newpad);
    if(caps == NULL)
    {
        caps = gst_pad_query_caps (newpad, NULL);
    }

    g_object_get(newpad, "name", &new_pad_caps_name, NULL);
    LOG_PRINTF(ALWAYS, "Rev demuxer pad(%s) :%s.", new_pad_caps_name, gst_caps_to_string(caps));

    if (!g_str_has_prefix(new_pad_caps_name, "video"))
    {
        LOG_PRINTF(ERROR, " not video pad ,skip.\n");
        return;
    }

    //prepare to find decoder factory
    update_factories_list(user_data);
    factories = gst_element_factory_list_filter (user_data->decoder_factories, caps, GST_PAD_SINK, TRUE);
    if (caps)
        gst_caps_unref (caps);

    if(factories)
    {
        LOG_PRINTF(ALWAYS, "prepare to create decode element.");
        element = gst_element_factory_create ((GstElementFactory *) factories->data, NULL);
        LOG_PRINTF(ALWAYS, "Created element '%s'.", GST_ELEMENT_NAME (element));
        gst_plugin_feature_list_free (factories);
    }else
    {
        LOG_PRINTF(ERROR, "factories is null.");
        return;
    }

    //add to bin
    if (!gst_bin_add ((GstBin *) user_data->pipeline, element)) {
        LOG_PRINTF(ERROR, "could not add decoder to pipeline.");
        return;
    }

    GstPad* sinkpad = gst_element_get_static_pad (element, "sink");
    GstPad* srcpad = gst_element_get_static_pad (element, "src");

    //try to link
    if (gst_pad_link_full (newpad, sinkpad, GST_PAD_LINK_CHECK_NOTHING) != GST_PAD_LINK_OK) {
        LOG_PRINTF(ERROR, "could not link to %s:%s",  GST_DEBUG_PAD_NAME (sinkpad));
        return;
    }
    //change state
    gst_element_sync_state_with_parent(element);

    //create videoscale
    LOG_PRINTF(ALWAYS, "prepare to create videoscale.");
    GstElement *scale = gst_element_factory_make("videoscale", "scale");
    GstCaps* newpad_caps = gst_pad_get_current_caps (newpad);
    LOG_PRINTF(ALWAYS, "video stream src caps:%s.", gst_caps_to_string(newpad_caps));
    GstStructure *ins = gst_caps_get_structure (newpad_caps, 0);
    const GValue *par = gst_structure_get_value (ins, "pixel-aspect-ratio");
    GstCaps *filter_caps = NULL;
    if(par)
    {
        gint par_n, par_d;
        par_n = gst_value_get_fraction_numerator (par);
        par_d = gst_value_get_fraction_denominator (par);
        LOG_PRINTF(ALWAYS, "par_n:%d, par_d:%d.", par_n, par_d);
        
        filter_caps = gst_caps_new_simple ("video/x-raw",
            "width", G_TYPE_INT, 480,
            "height", G_TYPE_INT, 360,
            "pixel-aspect-ratio", GST_TYPE_FRACTION, par_n, par_d,
            NULL);
    }else
    {
        filter_caps = gst_caps_new_simple ("video/x-raw",
            "width", G_TYPE_INT, 480,
            "height", G_TYPE_INT, 360,
            NULL);
    }

    GstPad* scale_sink = gst_element_get_static_pad (scale, "sink");
    GstPad* scale_src = gst_element_get_static_pad (scale, "src");
    if (!gst_bin_add ((GstBin *) user_data->pipeline, scale)) {
        LOG_PRINTF(ERROR, "could not add scale to pipeline.");
        return;
    }

    if (gst_pad_link_full (srcpad, scale_sink, GST_PAD_LINK_CHECK_NOTHING) != GST_PAD_LINK_OK) {
        LOG_PRINTF(ERROR, "could not link to %s:%s.",  GST_DEBUG_PAD_NAME (scale_sink));
        return;
    }
    //change state
    gst_element_sync_state_with_parent(scale);

    //create videoconvert
    LOG_PRINTF(ALWAYS, "prepare to create videoconvert.");
    GstElement *convert = gst_element_factory_make("videoconvert", "convert");
    if (!gst_bin_add ((GstBin *) user_data->pipeline, convert)) {
        LOG_PRINTF(ERROR, "could not add convert to pipeline.");
        return;
    }

    if (gst_element_link_filtered (scale, convert, filter_caps) != TRUE) {
        LOG_PRINTF(ERROR, "could not link scale ==>> capsfilter ==>> convert.");
        return;
    }
    //change state
    gst_element_sync_state_with_parent(convert);

    //create rgba convert
    GstCaps *filter_rgba_caps = NULL;
    filter_rgba_caps = gst_caps_new_simple ("video/x-raw","format", G_TYPE_STRING, "YV12", NULL);

    //create videosink
    LOG_PRINTF(ALWAYS, "prepare to create autovideosink.");
    GstElement *sinkplug = gst_element_factory_make("autovideosink", "video_show");
    if (!gst_bin_add ((GstBin *) user_data->pipeline, sinkplug)) {
        LOG_PRINTF(ERROR, "could not add sink to pipeline.");
        return;
    }

    if (gst_element_link_filtered (convert, sinkplug, filter_rgba_caps) != TRUE) {
        LOG_PRINTF(ERROR, "could not link convert ==>> RGBA ==>> sink.");
        return;
    }
    //change state
    gst_element_sync_state_with_parent(sinkplug);
}

/**
 * @brief: handle urisourcebin pad-added signal
 * @param[in] typefind: the sourcebin instance
 * @param[in] newpad: the pad that has been added
 * @param[in] user_data: means Thumb_t
 */
void sourcebin_pad_added(GstElement *obj, GstPad *newpad, Thumb_t* user_data)
{
    LOG_PRINTF(ALWAYS, "Received new pad '%s' from '%s'.", GST_PAD_NAME (newpad), GST_ELEMENT_NAME (obj));
    GstCaps *newpad_caps = gst_pad_get_current_caps (newpad);
    LOG_PRINTF(ALWAYS, "caps:%s.", gst_caps_to_string(newpad_caps));
    if(newpad_caps)
    {
        gst_caps_unref (newpad_caps);
    }
    GstPad *sink_pad = gst_element_get_static_pad (user_data->typefind, "sink");
    if (gst_pad_is_linked (sink_pad)) {
        LOG_PRINTF(ERROR, "We are already linked. Ignoring.");
        gst_object_unref (sink_pad);
        return;
    }

    /* Attempt the link */
    GstPadLinkReturn ret = gst_pad_link (newpad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret)) {
        LOG_PRINTF(ERROR, "Link failed.");
    } else {
        LOG_PRINTF(ALWAYS, "Link succeeded.");
    }
    gst_object_unref (sink_pad);
}

/**
 * @brief: update plugin factories
 * @param[in] user_data: means Thumb_t
 */
void update_factories_list(Thumb_t* thumb)
{
    guint cookie = gst_registry_get_feature_list_cookie (gst_registry_get ());
    //check if update factories
    if(!thumb->factories || thumb->cookie != cookie)
    {
        if(thumb->factories)
        {
            gst_plugin_feature_list_free(thumb->factories);
        }
        if(thumb->decoder_factories)
        {
            g_list_free(thumb->decoder_factories);
        }
        if(thumb->decodable_factories)
        {
            g_list_free(thumb->decodable_factories);
        }

        thumb->factories = gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_DECODABLE, GST_RANK_MARGINAL);
        thumb->factories = g_list_sort (thumb->factories, gst_plugin_feature_rank_compare_func);
        thumb->cookie = cookie;

        //filter decode and other
        thumb->decoder_factories = NULL;
        thumb->decodable_factories = NULL;
        for (GList *tmp = thumb->factories; tmp; tmp = tmp->next) {
            GstElementFactory *fact = (GstElementFactory *) tmp->data;
            if (gst_element_factory_list_is_type (fact, GST_ELEMENT_FACTORY_TYPE_DECODER))
                thumb->decoder_factories = g_list_append (thumb->decoder_factories, fact);
            else
                thumb->decodable_factories = g_list_append (thumb->decodable_factories, fact);
        }
    }
    //print
    LOG_PRINTF(ALWAYS, "cookie:%u, factories:%d, decoder_factories:%d, decodable_factories:%d.\n", 
        thumb->cookie,
        g_list_length(thumb->factories),
        g_list_length(thumb->decoder_factories),
        g_list_length(thumb->decodable_factories));
}