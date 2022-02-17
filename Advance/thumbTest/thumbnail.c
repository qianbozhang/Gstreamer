#include "thumbnail.h"

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
            g_print ("Rev GST_MESSAGE_CLOCK_LOST\n");
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
                g_print ("Pipeline state changed from %s to %s pending:%s:\n", 
                    gst_element_state_get_name (old_state),
                    gst_element_state_get_name (new_state),
                    gst_element_state_get_name (pending_state));
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
            g_print ("Rev Eos-of-stream.\n");
            gst_element_set_state (user_data->pipeline, GST_STATE_READY);
            g_main_loop_quit (user_data->loop);
            break;
        }
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
    g_print("rev probability:%u, caps: %s.\n", probability, gst_caps_to_string (caps));
    GValueArray *factories = NULL;
    GList *list, *tmp;
    GstPad* srcpad;
    //start analyze srcpad & caps
    if(!gst_caps_is_fixed(caps))
    {
        g_print("caps is not fixed.\n");
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
        g_print("cannot find demux.\n");
        return;
    }else
    {
        g_print("element factories n_values = %d.\n", factories->n_values);
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
        g_print("trying factory %p.\n", factory);
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
                        g_print ("caps %" GST_PTR_FORMAT " not subset of %" GST_PTR_FORMAT "\n", caps, templcaps);
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
        g_print("prepare to create demuxer.\n");
        if ((element = gst_element_factory_create (factory, NULL)) == NULL) {
            g_print ("Could not create an element from %s", gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)));
            continue;
        }

        //add element to bin
        if (!(gst_bin_add (GST_BIN_CAST (user_data->pipeline), element))) {
            g_print ("Couldn't add %s to the bin", GST_ELEMENT_NAME (element));
            gst_object_unref (element);
            continue;
        }

        //got element sink pad
        if (element->sinkpads != NULL)
            sinkpad = gst_object_ref (element->sinkpads->data);
        if (sinkpad == NULL) {
            g_print ( "Element %s doesn't have a sink pad", GST_ELEMENT_NAME (element));
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }

        //try link
        srcpad = gst_element_get_static_pad(typefind, "src");
        if ((gst_pad_link_full (srcpad, sinkpad, GST_PAD_LINK_CHECK_NOTHING)) != GST_PAD_LINK_OK) {
            g_print ("Link failed on pad %s:%s", GST_DEBUG_PAD_NAME (sinkpad));
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }
        g_print("link success .\n");

        //active it to ready
        if ((gst_element_set_state (element, GST_STATE_READY)) == GST_STATE_CHANGE_FAILURE) {
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }
        g_print("NULL ==>> READY .\n");

        /* check if we still accept the caps on the pad after setting
        * the element to READY */
        if (!gst_pad_query_accept_caps (sinkpad, caps)) {
            g_print ("Element %s does not accept caps", GST_ELEMENT_NAME (element));
            gst_element_set_state (element, GST_STATE_NULL);
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }

        gst_object_unref (sinkpad);
        g_print ("linked on pad %s:%s. \n", GST_DEBUG_PAD_NAME (srcpad));

        //link the element further
        g_signal_connect (element, "pad-added", G_CALLBACK (pad_added), user_data);
        //ready ==>> paused
        if ((gst_element_set_state (element, GST_STATE_PAUSED)) == GST_STATE_CHANGE_FAILURE) {
            gst_element_set_state (element, GST_STATE_NULL);
            gst_object_unref (sinkpad);
            gst_bin_remove (GST_BIN (user_data->pipeline), element);
            continue;
        }
        g_print("READY ==>> PAUSED .\n");
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
    g_print("Rev demuxer pad(%s) :%s.\n", new_pad_caps_name, gst_caps_to_string(caps));

    if (!g_str_has_prefix(new_pad_caps_name, "video"))
    {
        g_print(" not video pad ,skip.\n");
        return;
    }

    //prepare to find decoder factory
    update_factories_list(user_data);
    factories = gst_element_factory_list_filter (user_data->decoder_factories, caps, GST_PAD_SINK, TRUE);
    if (caps)
        gst_caps_unref (caps);

    if(factories)
    {
        g_print("prepare to create decode element.\n");
        element = gst_element_factory_create ((GstElementFactory *) factories->data, NULL);
        g_print ("Created element '%s' \n", GST_ELEMENT_NAME (element));
        gst_plugin_feature_list_free (factories);
    }else
    {
        g_print(" factories is null.\n");
        return;
    }

    //add to bin
    if (!gst_bin_add ((GstBin *) user_data->pipeline, element)) {
        g_print ("could not add decoder to pipeline");
        return;
    }

    GstPad* sinkpad = gst_element_get_static_pad (element, "sink");
    GstPad* srcpad = gst_element_get_static_pad (element, "src");

    //try to link
    if (gst_pad_link_full (newpad, sinkpad, GST_PAD_LINK_CHECK_NOTHING) != GST_PAD_LINK_OK) {
        g_print ("could not link to %s:%s",  GST_DEBUG_PAD_NAME (sinkpad));
        return;
    }
    //change state
    gst_element_sync_state_with_parent(element);
    // DOT_DUMP(user_data->pipeline);

    //create videoconvert
    g_print("prepare to create videoconvert.\n");
    GstElement *convert = gst_element_factory_make("videoconvert", "convert");

    GstPad* convert_sink = gst_element_get_static_pad (convert, "sink");
    GstPad* convert_src = gst_element_get_static_pad (convert, "src");
    if (!gst_bin_add ((GstBin *) user_data->pipeline, convert)) {
        g_print ("could not add convert to pipeline \n");
        return;
    }

    if (gst_pad_link_full (srcpad, convert_sink, GST_PAD_LINK_CHECK_NOTHING) != GST_PAD_LINK_OK) {
        g_print ("could not link to %s:%s \n",  GST_DEBUG_PAD_NAME (convert_sink));
        return;
    }
    //change state
    gst_element_sync_state_with_parent(convert);

    //create videosink
    g_print("prepare to create autovideosink.\n");
    GstElement *sinkplug = gst_element_factory_make("autovideosink", "video_show");

    GstPad* videosink_sink = gst_element_get_static_pad (sinkplug, "sink");
    if (!gst_bin_add ((GstBin *) user_data->pipeline, sinkplug)) {
        g_print ("could not add sink to pipeline \n");
        return;
    }

    if (gst_pad_link_full (convert_src, videosink_sink, GST_PAD_LINK_CHECK_NOTHING) != GST_PAD_LINK_OK) {
        g_print ("could not link to %s:%s \n",  GST_DEBUG_PAD_NAME (videosink_sink));
        return;
    }
    //change state
    gst_element_sync_state_with_parent(sinkplug);
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
}