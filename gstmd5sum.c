/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2012 Francisco Rocha <rocha.francisco.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-md5sum
 *
 * Lets data go through and prints MD5 digest and size for each buffer
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! md5sum ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/controller/gstcontroller.h>

#include "gstmd5sum.h"

GST_DEBUG_CATEGORY_STATIC (gst_md5sum_debug);
#define GST_CAT_DEFAULT gst_md5sum_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
};

/* the capabilities of the inputs and outputs.
 *
 */
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

/* debug category for fltering log messages
 *
 */
#define DEBUG_INIT(bla) \
  GST_DEBUG_CATEGORY_INIT (gst_md5sum_debug, "md5sum", 0, "md5sum");

GST_BOILERPLATE_FULL (Gstmd5sum, gst_md5sum, GstBaseTransform,
    GST_TYPE_BASE_TRANSFORM, DEBUG_INIT);

static void gst_md5sum_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_md5sum_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_md5sum_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

/* GObject vmethod implementations */

static void
gst_md5sum_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_set_details_simple (element_class,
    "md5sum",
    "Generic/Filter",
    "MD5 digest computing filter",
    "Francisco Rocha <rocha.francisco.a@gmail.com>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));
}

/* initialize the md5sum's class */
static void
gst_md5sum_class_init (Gstmd5sumClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;
  gobject_class->set_property = gst_md5sum_set_property;
  gobject_class->get_property = gst_md5sum_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
    g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_md5sum_transform_ip);
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_md5sum_init (Gstmd5sum *filter, Gstmd5sumClass * klass)
{
  filter->silent = FALSE;
}

static void
gst_md5sum_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstmd5sum *filter = GST_MD5SUM (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_md5sum_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstmd5sum *filter = GST_MD5SUM (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_md5sum_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  Gstmd5sum *filter = GST_MD5SUM (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (G_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));

  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in.\n");

  /* Compute buffer digest */
  g_message("Received buffer: %d bytes", GST_BUFFER_SIZE(outbuf));
  GChecksum* md5sum = g_checksum_new(G_CHECKSUM_MD5);
  g_checksum_update(md5sum, outbuf, GST_BUFFER_SIZE(outbuf));
  const gchar* digest = g_checksum_get_string(md5sum);
  g_message("Buffer's digest: %s", digest);

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
md5sum_init (GstPlugin * md5sum)
{
  /* initialize gst controller library */
  gst_controller_init(NULL, NULL);

  return gst_element_register (md5sum, "md5sum", GST_RANK_NONE,
      GST_TYPE_MD5SUM);
}

/* gstreamer looks for this structure to register md5sums
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "md5sum",
    "MD5 digest computing filter",
    md5sum_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
