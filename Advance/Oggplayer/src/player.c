#include "player.h"
#include "dot.h"
#include <gst/gst.h>

void dot_dump(GstElement *obj){
	do { 
           gchar *file_name = NULL;
           file_name = g_strdup_printf("%s():%d" ,__func__ ,__LINE__);
           gst_debug_bin_to_dot_file(GST_BIN (obj),GST_DEBUG_GRAPH_SHOW_ALL,file_name);
           g_free(file_name);
     }while(0);
}

void init_CustomData(CustomData *user_data)
{
	//memset(user_data, 0 , sizeof(user_data));

	user_data->c_video = -1;
	user_data->c_audio = -1;
	user_data->c_subtitle = -1;

	user_data->n_video = 0;
	user_data->n_audio = 0;
	user_data->n_subtitle = 0;

	user_data->cur_type = 0;

	//create element
    user_data->pipeline = gst_pipeline_new ("my_pipeline");
    user_data->source = gst_element_factory_make("filesrc", "sourcestream");
    user_data->demuxer = gst_element_factory_make("oggdemux", "oggdemux");
    user_data->streamsync = gst_element_factory_make("streamsynchronizer", "streamsync");
    user_data->multiqueue = gst_element_factory_make("multiqueue", "multiqueue");
    //video
    user_data->video_queue = gst_element_factory_make("queue", "video_queue");             /*video queue*/
    user_data->video_decoder = gst_element_factory_make("theoradec", "video_decoder");    /*vide decoder*/
    user_data->video_convert = gst_element_factory_make("videoconvert", "video_convert");  /*vide convert*/
    user_data->video_show = gst_element_factory_make("autovideosink", "video_show");       /*vide show*/
    //video
    user_data->audio_queue = gst_element_factory_make("queue", "audio_queue");             /*audio queue*/
    user_data->audio_decoder = gst_element_factory_make("vorbisdec", "audio_decoder");    /*audio decoder*/
    user_data->audio_convert = gst_element_factory_make("audioconvert", "audio_convert");  /*audio convert*/
    user_data->audio_show = gst_element_factory_make("autoaudiosink", "audio_show");       /*audio show*/


    user_data->sub_input_select = gst_element_factory_make("input-selector", "sub_select"); 
    user_data->sub_queue = gst_element_factory_make("queue", "qu"); 
    user_data->sub_decoder0 = gst_element_factory_make("katedec", NULL);       
    user_data->sub_decoder1 = gst_element_factory_make("katedec", NULL);       
    user_data->sub_decoder2 = gst_element_factory_make("katedec", NULL);       
    user_data->sub_decoder3 = gst_element_factory_make("katedec", NULL);       
    user_data->sub_decoder4 = gst_element_factory_make("katedec", NULL);       
    user_data->sub_lay = gst_element_factory_make("subtitleoverlay", "sub_lay");       
}

//link streamsync
void link_streamsync(CustomData *user_data)
{
	GstPad *new_sub_pad ,   *sub_dec_src;
    GstPad *new_video_pad , *video_dec_src;
    GstPad *new_audio_pad , *audio_dec_src;

    new_audio_pad = gst_element_get_request_pad (user_data->streamsync, "sink_%u");
    new_video_pad = gst_element_get_request_pad (user_data->streamsync, "sink_%u");
    new_sub_pad = gst_element_get_request_pad (user_data->streamsync, "sink_%u");

    sub_dec_src = gst_element_get_static_pad (user_data->sub_queue, "src");
    video_dec_src = gst_element_get_static_pad (user_data->video_decoder, "src");
    audio_dec_src = gst_element_get_static_pad (user_data->audio_decoder, "src");

    if (gst_pad_link (sub_dec_src, new_sub_pad) != GST_PAD_LINK_OK ||
        gst_pad_link (video_dec_src, new_video_pad) != GST_PAD_LINK_OK ||
        gst_pad_link (audio_dec_src, new_audio_pad) != GST_PAD_LINK_OK ) {
        g_printerr ("link_streamsync could not be linked.\n");
        gst_object_unref (user_data->pipeline);
       return ;
    }
    gst_object_unref (sub_dec_src);
    gst_object_unref (video_dec_src);
    gst_object_unref (audio_dec_src);

}

//link sub_input_select
void link_sub_input_select(CustomData *user_data)
{
	GstPad *input_sink0 , *dec_src0;
	GstPad *input_sink1 , *dec_src1;
	GstPad *input_sink2 , *dec_src2;
	GstPad *input_sink3 , *dec_src3;
	GstPad *input_sink4 , *dec_src4;

	input_sink0 = gst_element_get_request_pad (user_data->sub_input_select, "sink_%u");
	input_sink1 = gst_element_get_request_pad (user_data->sub_input_select, "sink_%u");
	input_sink2 = gst_element_get_request_pad (user_data->sub_input_select, "sink_%u");
	input_sink3 = gst_element_get_request_pad (user_data->sub_input_select, "sink_%u");
	input_sink4 = gst_element_get_request_pad (user_data->sub_input_select, "sink_%u");

	dec_src0 = gst_element_get_static_pad (user_data->sub_decoder0, "src");
	dec_src1 = gst_element_get_static_pad (user_data->sub_decoder1, "src");
	dec_src2 = gst_element_get_static_pad (user_data->sub_decoder2, "src");
	dec_src3 = gst_element_get_static_pad (user_data->sub_decoder3, "src");
	dec_src4 = gst_element_get_static_pad (user_data->sub_decoder4, "src");

    if (gst_pad_link (dec_src0, input_sink0) != GST_PAD_LINK_OK ||
        gst_pad_link (dec_src1, input_sink1) != GST_PAD_LINK_OK ||
        gst_pad_link (dec_src2, input_sink2) != GST_PAD_LINK_OK ||
        gst_pad_link (dec_src3, input_sink3) != GST_PAD_LINK_OK ||
        gst_pad_link (dec_src4, input_sink4) != GST_PAD_LINK_OK ) 
    {
        g_printerr ("link_sub_input_select could not be linked.\n");
        gst_object_unref (user_data->pipeline);
       return ;
    }

    gst_object_unref (dec_src0);
    gst_object_unref (dec_src1);
    gst_object_unref (dec_src2);
    gst_object_unref (dec_src3);
    gst_object_unref (dec_src4);

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

			if ( new_state == GST_STATE_READY ) {
				/* Read some properties */
				/*g_object_get (user_data->pipeline, "n-video", &user_data->n_video, NULL);
				g_object_get (user_data->pipeline, "n-audio", &user_data->n_audio, NULL);
				g_object_get (user_data->pipeline, "n-text", &user_data->n_subtitle, NULL);
                */
				g_print ("%d video stream, %d audio stream, %d text stream\n", user_data->n_video, user_data->n_audio, user_data->n_subtitle);
                /*dump dot*/
                //DOT_DUMP(user_data->pipeline);
			}

			if ( new_state == GST_STATE_PLAYING ) {
				/* Read some properties */
				/*g_object_get (user_data->pipeline, "n-video", &user_data->n_video, NULL);
				g_object_get (user_data->pipeline, "n-audio", &user_data->n_audio, NULL);
				g_object_get (user_data->pipeline, "n-text", &user_data->n_subtitle, NULL);
                */
				g_print ("%d video stream, %d audio stream, %d text stream\n", user_data->n_video, user_data->n_audio, user_data->n_subtitle);
                /*dump dot*/
                DOT_DUMP(user_data->pipeline);

               /* g_print("change subtitle 2.\n");
                g_object_set(user_data->sub_input_select, "active-pad", &user_data->c_subtitle, NULL);
                DOT_DUMP(user_data->pipeline);*/
			}
		}
		break;
	default:
		/* Unhandled message */
		break;
	}

}

//watch demux pad-added signal
void streamsync_src(GstElement *obj, GstPad *new_pad, CustomData *user_data)
{
	GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;


    gchar *pad_name = NULL;
    g_object_get(new_pad, "name", &pad_name, NULL);

    if ( !g_str_has_prefix(pad_name, "src_"))
	{
         return;
	}

    g_print ("streamsync received new pad:%s[%s] from '%s':\n", GST_PAD_NAME (new_pad), new_pad_type, GST_ELEMENT_NAME (obj));

    
	GstPad *a_sinkpad = NULL;
	GstPad *v_sinkpad = NULL;
	GstPad *s_sinkpad = NULL;
	a_sinkpad = gst_element_get_static_pad (user_data->audio_convert, "sink");
	v_sinkpad = gst_element_get_static_pad (user_data->sub_lay, "video_sink");
	s_sinkpad = gst_element_get_static_pad (user_data->sub_lay, "subtitle_sink");
	gboolean link_audio = gst_pad_is_linked (a_sinkpad);
	gboolean link_video = gst_pad_is_linked (v_sinkpad);
    gboolean link_sub = gst_pad_is_linked (s_sinkpad);

	if(!link_audio){
		if (!gst_pad_link (new_pad, a_sinkpad))
			g_print("%s linked to audio_queue.\n", new_pad_type);
		else
			g_print("ERROR: gst_pad_link (new_pad, a_sinkpad), pad_name = %s\n", new_pad_type);

		goto FREE;
	}

    if(!link_video){
		if (!gst_pad_link (new_pad, v_sinkpad)){
		     g_print("%s linked to audio_queue.\n", new_pad_type);
		}else
			g_print("ERROR: gst_pad_link (new_pad, v_sinkpad), pad_name = %s\n", new_pad_type);

		goto FREE;
	}

	if(!link_sub){
		if (!gst_pad_link (new_pad, s_sinkpad))
			g_print("%s linked to audio_queue.\n", new_pad_type);
		else
		    g_print("ERROR: gst_pad_link (new_pad, s_sinkpad), pad_name = %s\n", new_pad_type);
	    goto FREE;
	}  

    //free
FREE:
    gst_object_unref (a_sinkpad);
	gst_object_unref (v_sinkpad);
	gst_object_unref (s_sinkpad);
}


void multiqueue_src(GstElement *obj, GstPad *new_pad, CustomData *user_data)
{
	gchar *pad_name = NULL;
    g_object_get(new_pad, "name", &pad_name, NULL);

    if ( !g_str_has_prefix(pad_name, "src_"))
	{
         return;
	}

    g_print ("multiqueue_src received new pad:%s from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (obj));

    if( user_data->cur_type == MEDIA_TYPE_VIDEO){
        gst_pad_link (new_pad, gst_element_get_static_pad (user_data->video_queue, "sink") );

    }else if(user_data->cur_type == MEDIA_TYPE_AUDIO){
    	gst_pad_link (new_pad, gst_element_get_static_pad (user_data->audio_queue, "sink") );

    }else if(user_data->cur_type == MEDIA_TYPE_SUB){
        link_sub_dec(user_data, new_pad);
    }

}

void link_sub_dec(CustomData *user_data, GstPad *new_pad)
{
	GstPad *s_sinkpad = NULL;
	//dec0
	if( !gst_pad_is_linked( gst_element_get_static_pad (user_data->sub_decoder0, "sink") ) ){
         s_sinkpad = gst_element_get_static_pad (user_data->sub_decoder0, "sink");
         gst_pad_link (new_pad, s_sinkpad);
         g_print("linked to sub_decoder0.\n");
         goto FREE;
	}
	//dec1
	if( !gst_pad_is_linked( gst_element_get_static_pad (user_data->sub_decoder1, "sink") ) ){
         s_sinkpad = gst_element_get_static_pad (user_data->sub_decoder1, "sink");
         gst_pad_link (new_pad, s_sinkpad);
         g_print("linked to sub_decoder1.\n");
         goto FREE;
	}
	//dec2
	if( !gst_pad_is_linked( gst_element_get_static_pad (user_data->sub_decoder2, "sink") ) ){
         s_sinkpad = gst_element_get_static_pad (user_data->sub_decoder2, "sink");
         gst_pad_link (new_pad, s_sinkpad);
         g_print("linked to sub_decoder2.\n");
         goto FREE;
	}
	//dec3
	if( !gst_pad_is_linked( gst_element_get_static_pad (user_data->sub_decoder3, "sink") ) ){
         s_sinkpad = gst_element_get_static_pad (user_data->sub_decoder3, "sink");
         gst_pad_link (new_pad, s_sinkpad);
         g_print("linked to sub_decoder3.\n");
         goto FREE;
	}
	//dec4
	if( !gst_pad_is_linked( gst_element_get_static_pad (user_data->sub_decoder4, "sink") ) ){
         s_sinkpad = gst_element_get_static_pad (user_data->sub_decoder4, "sink");
         gst_pad_link (new_pad, s_sinkpad);
         g_print("linked to sub_decoder4.\n");
         goto FREE;
	}

FREE:
	gst_object_unref (s_sinkpad);
}



void pad_add_catch(GstElement *obj, GstPad *new_pad, CustomData *user_data)
{
// 	GstPadLinkReturn ret;
//     GstCaps *new_pad_caps = NULL;
//     GstStructure *new_pad_struct = NULL;
//     const gchar *new_pad_type = NULL;

//      /* Check the new pad's type */
//     new_pad_caps = gst_pad_get_current_caps (new_pad);
//     new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
//     new_pad_type = gst_structure_get_name (new_pad_struct);

//     g_print ("Received new pad:%s[%s] from '%s':\n", GST_PAD_NAME (new_pad), new_pad_type, GST_ELEMENT_NAME (obj));
//     //video count
//     if ( g_str_has_prefix(new_pad_type, "video") )
//     {
//          user_data->n_video += 1;
//     }
//     //audio count
//     if ( g_str_has_prefix(new_pad_type, "audio") )
//     { 
//          user_data->n_audio += 1;
//     }
//     //sub count
//     if ( g_str_has_prefix(new_pad_type, "subtitle") )
//     {
//          user_data->n_subtitle += 1;
//          //dec
//          GstPad *s_sinkpad = NULL;
//          //check
//          link_sub_dec(user_data, new_pad);

//          return;
//     }

//     GstPad *v_sinkpad = NULL;
// 	GstPad *a_sinkpad = NULL;

// 	v_sinkpad = gst_element_get_static_pad (user_data->video_queue, "sink");
// 	a_sinkpad = gst_element_get_static_pad (user_data->audio_queue, "sink");

// 	gboolean link_video = gst_pad_is_linked (v_sinkpad);
// 	gboolean link_audio = gst_pad_is_linked (a_sinkpad);

// 	if (link_video && link_audio) {
// 		g_print("audio/video already linked. ignore new_pad(%s).\n", new_pad_type);
// 		goto FREE;
// 	}

//     if (g_str_has_prefix(new_pad_type, "video") && !link_video )
// 	{
// 		if (!gst_pad_link (new_pad, v_sinkpad))
// 			g_print("%s linked to video_queue.\n", new_pad_type);
// 		else
// 			g_print("ERROR: gst_pad_link (new_pad, v_sinkpad), pad_name = %s\n", new_pad_type);
// 	}

// 	if (g_str_has_prefix(new_pad_type, "audio") && !link_audio )
// 	{
// 		if (!gst_pad_link (new_pad, a_sinkpad))
// 			g_print("%s linked to audio_queue.\n", new_pad_type);
// 		else
// 			g_print("ERROR: gst_pad_link (new_pad, a_sinkpad), pad_name = %s\n", new_pad_type);
// 	}

// FREE:
// 	gst_object_unref (a_sinkpad);
// 	gst_object_unref (v_sinkpad);
	GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

     /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps (new_pad);
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);

    g_print ("Received new pad:%s[%s] from '%s':\n", GST_PAD_NAME (new_pad), new_pad_type, GST_ELEMENT_NAME (obj));
    //video count
    if ( g_str_has_prefix(new_pad_type, "video") )
    {
         user_data->n_video += 1;
         user_data->cur_type = MEDIA_TYPE_VIDEO;
    }
    //audio count
    if ( g_str_has_prefix(new_pad_type, "audio") )
    { 
         user_data->n_audio += 1;
         user_data->cur_type = MEDIA_TYPE_AUDIO;
    }
    //sub count
    if ( g_str_has_prefix(new_pad_type, "subtitle") )
    {
         user_data->n_subtitle += 1;
         user_data->cur_type = MEDIA_TYPE_SUB;
    }

    GstPad *m_pad = NULL;
    m_pad = gst_element_get_request_pad (user_data->multiqueue, "sink_%u");

    if(gst_pad_link (new_pad, m_pad) != GST_PAD_LINK_OK){
        g_print("pad[%s] link to mutiqueue fail.\n", new_pad_type);
    }

}