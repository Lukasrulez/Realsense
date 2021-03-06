#include <gst/gst.h>
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering
#include <algorithm>            // std::min, std::max
#include <convert.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <librealsense2/hpp/rs_internal.hpp>

#include "example.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <int-rs-splash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define CHUNK_SIZE 7372800
using namespace std;


const int W = 640;
const int H = 480;
const int BPP = 2;
const int BPP1 = 8;

struct synthetic_frame
{
    int x, y, bpp;
    std::vector<uint8_t> frame;
};

class custom_frame_source
{
public:
    custom_frame_source()
    {
        depth_frame.x = W;
        depth_frame.y = H;
        depth_frame.bpp = BPP;

        color_frame.x = W;
        color_frame.y = H;
        color_frame.bpp = BPP1;
    }

    synthetic_frame& get_synthetic_texture()
    {
        auto realsense_logo = stbi_load_from_memory(splash, (int)splash_size, &color_frame.x, &color_frame.y, &color_frame.bpp, false);

        std::vector<uint8_t> pixels_color(color_frame.x * color_frame.y * color_frame.bpp, 0);

        memcpy(pixels_color.data(), realsense_logo, color_frame.x * color_frame.y * 4);

        for (auto i = 0; i < color_frame.y; i++)
            for (auto j = 0; j < color_frame.x * 4; j += 4)
            {
                if (pixels_color.data()[i * color_frame.x * 4 + j] == 0)
                {
                    pixels_color.data()[i * color_frame.x * 4 + j] = 200;
                    pixels_color.data()[i * color_frame.x * 4 + j + 1] = 0;
                    pixels_color.data()[i * color_frame.x * 4 + j + 2] = 200;

                }
            }
        color_frame.frame = std::move(pixels_color);

        return color_frame;
    }

    synthetic_frame& get_gst_color(GstAppSink* sink1)
    {
        //auto realsense_logo = stbi_load_from_memory(splash, (int)splash_size, &color_frame.x, &color_frame.y, &color_frame.bpp, false);

        std::vector<uint8_t> pixels_color(color_frame.x * color_frame.y * 4, 0);
        color_frame.frame = std::move(pixels_color);
        //std::cout << "W1:" << color_frame.x << std::endl << "H1:" << color_frame.y << std::endl << "BPP1:" << color_frame.bpp << std::endl;
        //memcpy(pixels_color.data(), realsense_logo, color_frame.x * color_frame.y*4);

        GstSample* sample = gst_app_sink_pull_sample(sink1);

        if (sample)
        {
            GstBuffer* buffer = gst_sample_get_buffer(sample);
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);
            // g_print(" get gst color ! ");

            // memcpy(pixels_color.data(), (char*)map2.data, color_frame.x * color_frame.y * 4);

            for (auto i = 0; i < color_frame.y; i++)
            {
                for (auto j = 0; j < color_frame.x; j++)
                {
                    ((uint16_t*)color_frame.frame.data())[i * color_frame.x + j] = (int)(map.data[i * color_frame.x + j] * 0xff);
                    //(pixels_color.data())[i * color_frame.x*4 + j] = (int)(map2.data[i * color_frame.x + j] );
                   // (pixels_color.data())[i * color_frame.x*4+ j + 1] = (int)(map2.data[i * color_frame.x + j+1] );
                  //  (pixels_color.data())[i * color_frame.x*4 + j + 2] = (int)(map2.data[i * color_frame.x + j+2] );

                }
            }

            //color_frame.frame = std::move(pixels_color);

            gst_buffer_unmap(buffer, &map);

        }

        gst_sample_unref(sample);
        return color_frame;
    }
    synthetic_frame& get_gst_color_pixel(GstAppSink* sink1)
    {
        //auto realsense_logo = stbi_load_from_memory(splash, (int)splash_size, &color_frame.x, &color_frame.y, &color_frame.bpp, false);

        std::vector<uint8_t> pixels_color(color_frame.x * color_frame.y * color_frame.bpp * 4, 0);

        //std::cout << "W1:" << color_frame.x << std::endl << "H1:" << color_frame.y << std::endl << "BPP1:" << color_frame.bpp << std::endl;
        //memcpy(pixels_color.data(), realsense_logo, color_frame.x * color_frame.y*4);

        GstSample* sample = gst_app_sink_pull_sample(sink1);

        if (sample)
        {
            GstBuffer* buffer = gst_sample_get_buffer(sample);
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);
            // g_print(" get gst color ! ");

            // memcpy(pixels_color.data(), (char*)map2.data, color_frame.x * color_frame.y * 4);

            for (auto i = 0; i < color_frame.y; i++)
            {
                for (auto j = 0; j < color_frame.x * 4; j += 4)
                {
                    // ((uint16_t*)color_frame.frame.data())[i * color_frame.x + j] = (int)(map.data[i * color_frame.x + j] * 0xff);
                    pixels_color.data()[i * color_frame.x * 4 + j] = map.data[i * color_frame.x * 4 + j] * 0xff;
                    pixels_color.data()[i * color_frame.x * 4 + j + 1] = map.data[i * color_frame.x * 4 + j + 1] * 0xff;
                    pixels_color.data()[i * color_frame.x * 4 + j + 2] = map.data[i * color_frame.x * 4 + j + 2] * 0xff;

                }
            }


            color_frame.frame = std::move(pixels_color);


            gst_buffer_unmap(buffer, &map);

        }

        gst_sample_unref(sample);
        return color_frame;
    }



    synthetic_frame& get_synthetic_depth(glfw_state& app_state)
    {
        draw_text(50, 50, "This point-cloud is generated from a synthetic device:");

        std::vector<uint8_t> pixels_depth(depth_frame.x * depth_frame.y * depth_frame.bpp, 0);
        depth_frame.frame = std::move(pixels_depth);

        auto now = std::chrono::high_resolution_clock::now();
        if (now - last > std::chrono::milliseconds(1))
        {
            app_state.yaw -= 1;
            wave_base += 0.1f;
            last = now;

            for (int i = 0; i < depth_frame.y; i++)
            {
                for (int j = 0; j < depth_frame.x; j++)
                {
                    auto d = 2 + 0.1 * (1 + sin(wave_base + j / 50.f));
                    ((uint16_t*)depth_frame.frame.data())[i * depth_frame.x + j] = (int)(d * 0xff);
                }
            }
        }
        return depth_frame;
    }

    synthetic_frame& get_gst_depth(GstAppSink* sink2)
    {
        draw_text(50, 50, "This point-cloud is generated from a gstreamer pipeline:");


        std::vector<uint8_t> pixels_depth(depth_frame.x * depth_frame.y * depth_frame.bpp, 0);
        depth_frame.frame = std::move(pixels_depth);


        //GstSample* sample1;
        GstSample* sample1 = gst_app_sink_pull_sample(sink2);

        // g_signal_emit_by_name(sink, "pull-sample", &sample1);
        if (sample1)
        {

            // sample1 = gst_app_sink_pull_sample(sink);
            GstBuffer* buffer = gst_sample_get_buffer(sample1);
            // const GstStructure* info = gst_sample_get_info(sample1);

            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);
            //g_print(" get gst depth ! ");
            //(uint16_t*)depth_frame.frame.data() = (uint16_t)map.data;
            for (auto i = 0; i < depth_frame.y; i++)
            {
                for (auto j = 0; j < depth_frame.x; j++)
                {
                    //auto d = 2;
                    ((uint16_t*)depth_frame.frame.data())[i * depth_frame.x + j] = (int)(map.data[i * depth_frame.x + j] * 0xff);
                }
            }

            gst_buffer_unmap(buffer, &map);

        }

        gst_sample_unref(sample1);
        //gst_object_unref(sink);

        return depth_frame;
    }


    rs2_intrinsics create_texture_intrinsics()
    {
        rs2_intrinsics intrinsics = { color_frame.x, color_frame.y,
            (float)319.699, (float)237.435,
            (float)606.322, 606.028 ,
            RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };

        return intrinsics;
    }

    rs2_intrinsics create_depth_intrinsics()
    {
        rs2_intrinsics intrinsics = { depth_frame.x, depth_frame.y,
            (float)322.349, (float)240.59,
            (float)383.94 , (float)383.94 ,
            RS2_DISTORTION_BROWN_CONRADY ,{ 0,0,0,0,0 } };

        return intrinsics;
    }

private:
    synthetic_frame depth_frame;
    synthetic_frame color_frame;

    std::chrono::high_resolution_clock::time_point last;
    float wave_base = 0.f;
};

//#define CHUNK_SIZE  614400       /* Amount of bytes we are sending in each buffer */
//#define SAMPLE_RATE 44100       /* Samples per second we are sending */




typedef struct _CustomData
{
    GstElement* pipeline1, * source1, * buffer1, * depay1, * decode1, * convert1, * sink1,
        * pipeline2, * source2, * buffer2, * depay2, * decode2, * convert2, * sink2;

    guint64 num_samples;          /* Number of samples generated so far (for timestamp generation) */
    gfloat a, b, c, d;            /* For waveform generation */

    guint sourceid;               /* To control the GSource */

    GMainLoop* main_loop;         /* GLib's Main Loop */
} CustomData;



/*synthetic_frame depth_frame;
std::vector<uint8_t> pixels_depth(depth_frame.x* depth_frame.y* depth_frame.bpp, 0);
// The appsink has received a buffer
GstFlowReturn new_sample(GstAppSink* sink, CustomData* data)
{
    //GstSample* sample;
    GstSample* sample = gst_app_sink_pull_sample(sink);


    depth_frame.frame = std::move(pixels_depth);

    //Retrieve the buffer
   // g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample) {

        g_print(" sink get signal ! ");
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        const GstStructure* info = gst_sample_get_info(sample);
        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_READ);


        for (int i = 0; i < depth_frame.y; i++)
        {
            for (int j = 0; j < depth_frame.x; j++)
            {

                ((uint16_t*)depth_frame.frame.data())[i * depth_frame.x + j] = (int)map.data[i * depth_frame.x + j];
            }
        }

        // g_print((char*)map.data[1]);
        gst_buffer_unmap(buffer, &map);

        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}
*/


int main(int argc, char* argv[]) try {
    CustomData data;
    GstVideoInfo info;
    //GstCaps* audio_caps;
    GstBus* bus;

    /* Initialize cumstom data structure */
    memset(&data, 0, sizeof(data));

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    data.source1 = gst_element_factory_make("udpsrc", "source1");
    data.buffer1 = gst_element_factory_make("rtpjitterbuffer", "buffer1");
    data.depay1 = gst_element_factory_make("rtph264depay", "depay");
    data.decode1 = gst_element_factory_make("avdec_h264", "decode");
    data.convert1 = gst_element_factory_make("videoconvert", "convert");
    //data.sink1 = gst_element_factory_make("autovideosink", "sink1");
    data.sink1 = gst_element_factory_make("appsink", "sink1");

    data.source2 = gst_element_factory_make("udpsrc", "source2");
    data.buffer2 = gst_element_factory_make("rtpjitterbuffer", "buffer2");
    data.depay2 = gst_element_factory_make("rtph264depay", "depay");
    data.decode2 = gst_element_factory_make("avdec_h264", "decode");
    data.convert2 = gst_element_factory_make("videoconvert", "convert");
    // data.sink2 = gst_element_factory_make("autovideosink", "sink2");
    data.sink2 = gst_element_factory_make("appsink", "sink2");

    /* Create the empty pipeline */
    data.pipeline1 = gst_pipeline_new("test-pipeline1");
    data.pipeline2 = gst_pipeline_new("test-pipeline2");

    if (!data.pipeline1 || !data.source1 || !data.buffer1 || !data.depay1 || !data.decode1 || !data.convert1 || !data.sink1 ||
        !data.pipeline2 || !data.source2 || !data.buffer2 || !data.depay2 || !data.decode2 || !data.convert2 || !data.sink2) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Configure appsrc */
    GstCaps* caps;
    caps = gst_caps_new_simple("application/x-rtp", "encoding-name", G_TYPE_STRING, "H264", NULL);
    if (!GST_IS_CAPS(caps)) {
        g_printerr("caps null ??.\n");
        return -1;
    }
    /* Modify the source's properties */

    g_object_set(data.source1, "port", 8554, NULL);
    g_object_set(data.source1, "caps", caps, NULL);
    g_object_set(data.source2, "port", 8564, NULL);
    g_object_set(data.source2, "caps", caps, NULL);

    /* Configure appsink */
    GstCaps* sink_caps;
    sink_caps = gst_caps_new_simple("video/x-raw", "Format", G_TYPE_STRING, "RGBA", NULL);
    g_object_set(data.sink1, "caps", sink_caps, NULL);
    GstCaps* sink_caps2;
    sink_caps2 = gst_caps_new_simple("video/x-raw", "Format", G_TYPE_STRING, "GRAY16_LE", NULL);
    //g_object_set(data.sink2, "caps", sink_caps2, NULL);
    /* Configure appsrc */
    //gst_video_info_set_format(&info, GST_VIDEO_FORMAT_UNKNOWN,  640,480);
   // audio_caps = gst_video_info_to_caps(&info);

    /* Configure appsink */

   /* bool set_sink = true;
    if (set_sink)
    {
        // g_object_set(data.sink1, "emit-signals", TRUE, NULL);  //  Do we really need caps?
         //g_signal_connect(data.sink2, "new-sample", G_CALLBACK(new_sample), &data);

                 if (new_sample((GstAppSink*)data.sink2, &data) == GST_FLOW_OK)
                 {
                     gst_app_sink_set_emit_signals((GstAppSink*)data.sink2, true);
                     gst_app_sink_set_drop((GstAppSink*)data.sink2, true);
                     gst_app_sink_set_max_buffers((GstAppSink*)data.sink2, 1);
                 }

                 // gst_caps_unref(audio_caps);
    }
    */



    /* Link all elements that can be automatically linked because they have "Always" pads */
    gst_bin_add_many(GST_BIN(data.pipeline1), data.source1, data.buffer1, data.depay1, data.decode1, data.convert1, data.sink1, NULL);
    if (gst_element_link_many(data.source1, data.buffer1, data.depay1, data.decode1, data.convert1, data.sink1)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data.pipeline1);
        return -1;
    }
    gst_bin_add_many(GST_BIN(data.pipeline2), data.source2, data.buffer2, data.depay2, data.decode2, data.convert2, data.sink2, NULL);
    if (gst_element_link_many(data.source2, data.buffer2, data.depay2, data.decode2, data.convert2, data.sink2)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data.pipeline2);
        return -1;
    }




    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    bus = gst_element_get_bus(data.pipeline1);
    gst_bus_add_signal_watch(bus);
    //g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)error_cb, &data);
    gst_object_unref(bus);

    /* Start playing the pipeline */
    gst_element_set_state(data.pipeline1, GST_STATE_PLAYING);
    gst_element_set_state(data.pipeline2, GST_STATE_PLAYING);
    /* Create a GLib Main Loop and set it to run */
    //data.main_loop = g_main_loop_new(NULL, FALSE);
   // g_main_loop_run(data.main_loop);

    /* Release the request pads from the Tee, and unref them */


    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$      Software Device        ??????????????????????????????????????????????????????????????????????


    window app(1280, 1500, "Gstreamer Capture Example");
    glfw_state app_state;
    register_glfw_callbacks(app, app_state);
    //rs2::colorizer color_map; // Save colorized depth for preview

    rs2::pointcloud pc;
    rs2::points points;
    int frame_number = 0;

    custom_frame_source app_data;


    rs2_intrinsics color_intrinsics = app_data.create_texture_intrinsics();
    rs2_intrinsics depth_intrinsics = app_data.create_depth_intrinsics();



    rs2::software_device dev; // Create software-only device

    auto depth_sensor = dev.add_sensor("Depth"); // Define single sensor
    auto color_sensor = dev.add_sensor("Color"); // Define single sensor

    auto depth_stream = depth_sensor.add_video_stream({ RS2_STREAM_DEPTH, 0, 0,
                                W, H, 60, BPP,
                                RS2_FORMAT_Z16, depth_intrinsics });

    depth_sensor.add_read_only_option(RS2_OPTION_DEPTH_UNITS, 0.001f);

    //cout << "texture_x:" << texture.x << endl << "texture_y:" << texture.y << endl << "texture.bpp:" << texture.bpp << endl;
    cout << "W:" << W << endl << "H:" << H << endl << "BPP:" << BPP << endl;
    auto color_stream = color_sensor.add_video_stream({ RS2_STREAM_COLOR, 0, 1,
                                W, H, 60, BPP,                        //THESE parameter is for the size of color frame
                                RS2_FORMAT_Y10BPACK, color_intrinsics }); //


    dev.create_matcher(RS2_MATCHER_DLR_C);
    rs2::syncer sync;

    depth_sensor.open(depth_stream);
    color_sensor.open(color_stream);

    depth_sensor.start(sync);
    color_sensor.start(sync);

    depth_stream.register_extrinsics_to(color_stream, { { 0.999927,-0.0116198,-0.00342654,
                                                          0.0116159,0.999932,-0.00114862,
                                                          0.00343965,0.00110873,0.999993 },{ 0,0.00013954,0.000115604 } });



    //auto color_frame = app_data.get_synthetic_texture();


    while (app) // Application still alive?
    {
        // synthetic_frame& depth_frame = app_data.get_synthetic_depth(app_state);
       // auto color_frame = app_data.get_synthetic_texture();


        auto color_frame = app_data.get_gst_color((GstAppSink*)data.sink1);
        //auto color_frame = app_data.get_gst_color_pixel((GstAppSink*)data.sink1);
        auto depth_frame = app_data.get_gst_depth((GstAppSink*)data.sink2);
        //auto color_frame = app_data.get_gst_color((GstAppSink*)data.sink1);


        depth_sensor.on_video_frame({ depth_frame.frame.data(), // Frame pixels from capture API
            [](void*) {}, // Custom deleter (if required)
            depth_frame.x * depth_frame.bpp, depth_frame.bpp, // Stride and Bytes-per-pixel
            (rs2_time_t)frame_number * 16, RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK, frame_number, // Timestamp, Frame# for potential sync services
            depth_stream });



        color_sensor.on_video_frame({ color_frame.frame.data(), // Frame pixels from capture API
             [](void*) {}, // Custom deleter (if required)
             color_frame.x * color_frame.bpp, color_frame.bpp, // Stride and Bytes-per-pixel
             (rs2_time_t)frame_number * 16, RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK, frame_number, // Timestamp, Frame# for potential sync services
             color_stream });

        ++frame_number;

        rs2::frameset fset = sync.wait_for_frames();
        rs2::frame depth = fset.first_or_default(RS2_STREAM_DEPTH);
        rs2::frame color = fset.first_or_default(RS2_STREAM_COLOR);

        if (depth && color)
        {
            if (auto as_depth = depth.as<rs2::depth_frame>())
                points = pc.calculate(as_depth);
            pc.map_to(color);
            //std::cout << "got synthetic streams! " << std::endl;
            // Upload the color frame to OpenGL
            app_state.tex.upload(color);
        }
        draw_pointcloud(app.width(), app.height(), app_state, points);
    }

    /* Free resources */
    gst_element_set_state(data.pipeline1, GST_STATE_NULL);
    gst_object_unref(data.pipeline1);
    gst_element_set_state(data.pipeline2, GST_STATE_NULL);
    gst_object_unref(data.pipeline2);
    return 0;



}
catch (const rs2::error& e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
