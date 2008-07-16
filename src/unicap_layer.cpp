/*  FreeJ
 *  (c) Copyright 2008 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <config.h>

#ifdef WITH_UNICAP

#include <unicap_layer.h>

#include <ccvt.h>

#include <jutils.h>


UnicapLayer::UnicapLayer()
  :Layer() {
  unicap_device_t device;
  int i=0;

  m_handle = NULL;

  notice("Unicap layer initialized, devices detected:");
  
  while( SUCCESS(unicap_enumerate_devices( NULL, &device, i++)))
      act("%u - %s",i,device.identifier);

  swap = 0;
  rgba[0] = NULL;
  rgba[1] = NULL;
  feed_ready = NULL;

}

UnicapLayer::~UnicapLayer() {

  close();

}


static void new_frame_cb (unicap_event_t event, unicap_handle_t handle,
			  unicap_data_buffer_t * buffer, void *usr_data) {
  UnicapLayer *lay = (UnicapLayer*)usr_data;

  ccvt_yuyv_bgr32(lay->geo.w, lay->geo.h, buffer->data, lay->rgba[lay->swap]);

  lay->feed_ready = lay->rgba[lay->swap];

  lay->swap = (lay->swap+1)%2;
}

bool UnicapLayer::open(const char *devfile) {
  bool res = false;
  int i = 0;
  int fourcc;
  unicap_device_t device;
  unicap_format_t format, format_spec;
  
  while( unicap_enumerate_devices( NULL, &device, i++)
	 == STATUS_SUCCESS) {

    if(strcmp(devfile,device.device)==0) { // found

      m_device = device;  

      if( unicap_open(&m_handle, &m_device ) == STATUS_SUCCESS ) {
	res = true;
	break;
      }

    } 

  }

  if(!res) {
    error("Unicap device %s is not found", m_device.identifier);
    return(false);
  }
  
  notice("Unicap device opened: %s",m_device.identifier);

  act("available video formats:");
  unicap_void_format( &format_spec );

  i=0;
  fourcc=0;
  while(SUCCESS(unicap_enumerate_formats( m_handle, &format_spec, &format, i++))) {
    act("%u - %s - 0x%x - %u bpp",i, format.identifier, format.fourcc, format.bpp);
    switch(format.fourcc) {

    case 0x56595559:
    case 0x32595559:
      fourcc = format.fourcc;
      break;

    case 0x30323449:
    case 0x56555949:
      fourcc = format.fourcc;
      break;

    default: break;

    }
  }
  if(!fourcc) {
    warning("no known colorspace supported - trying YUV422");
    fourcc = 0x56595559;
  }

  if( ! SUCCESS( unicap_get_format (m_handle, &format) ))
    error("format get failed on capture device");

  format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;
  format.size.width =  geo.w;
  format.size.height = geo.h;
  format.fourcc = fourcc;

  if( ! SUCCESS( unicap_set_format(m_handle, &format) )) {
    error("format setting failed on capture device");
    error("probably the size is not supported by this camera");
    error("else report your model and format strings");
    unicap_close(m_handle);
    return(false);
  }
      

  rgba[0] = jalloc(format.size.width * format.size.height * 4);
  rgba[1] = jalloc(format.size.width * format.size.height * 4);
  feed_ready = rgba[1];

  unicap_register_callback (m_handle,
			    UNICAP_EVENT_NEW_FRAME,
			    (unicap_callback_t) new_frame_cb, (void*)this);

  if( ! SUCCESS( unicap_start_capture( m_handle ) ))
    error("start capture failed on capture device");

  set_name(m_device.device);
  opened = true;
  return(res);
}

bool UnicapLayer::init(Context *freej) {
  return init(freej, freej->screen->w, freej->screen->h);
}

bool UnicapLayer::init(Context *freej, int width, int height) {
  _init(width, height);
  return true;
}


void *UnicapLayer::feed() {
  return feed_ready;
}


void UnicapLayer::close() {
  if(!opened) return;
  
  if( ! SUCCESS(unicap_stop_capture(m_handle)))
    error("unicap reports error in stop_capture");

  unicap_close(m_handle);

  jfree(rgba[0]);
  jfree(rgba[1]);

  opened = false;
}


#endif