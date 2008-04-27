/*  FreeJ WiiMote controller
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
 *
 * based on libwiimote by Hector Martin (marcan)
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
 *
 * "$Id: $"
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <config.h>

#include <wiimote_ctrl.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h>
#include <jsparser_data.h>

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))


/////// Javascript WiiController
JS(js_wii_ctrl_constructor);

DECLARE_CLASS_GC("WiiController", js_wii_ctrl_class, js_wii_ctrl_constructor, js_ctrl_gc);

JS(js_wii_ctrl_connect);

JSFunctionSpec js_wii_ctrl_methods[] = {
  {"connect",      js_wii_ctrl_connect,      0},
  {0}
};

JS(js_wii_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];
  
  WiiController *wii = new WiiController();
  // initialize with javascript context
  if(! wii->init(cx, obj) ) {
    sprintf(excp_msg, "failed initializing WiiMote controller");
    goto error;
  }

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)wii) ) {
    sprintf(excp_msg, "failed assigning WiiMote controller to javascript");
    goto error;
  }

  notice("WiiMote controller attached");

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  //  cx->newborn[GCX_OBJECT] = NULL;
  delete wii; return JS_FALSE;
}

JS(js_wii_ctrl_connect) {
    func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
    WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
    if(!wii) JS_ERROR("Wii core data is NULL");

    if(argc>0) {
      char *addr;
      JS_ARG_STRING(addr,1);
      wii->connect(addr);
    } else
      wii->connect(NULL);
    
    return JS_TRUE;
}

WiiController *tmp; // ARG. this is not reentrant because C sucks.
                    // PLEASE add a void *user_data in the cwiid_wiimote struct!!!!
void cwiid_callback(cwiid_wiimote_t *wii, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp) {
    
  tmp->accel(     mesg[mesg_count-1].acc_mesg.acc[CWIID_X],
		  mesg[mesg_count-1].acc_mesg.acc[CWIID_Y],
		  mesg[mesg_count-1].acc_mesg.acc[CWIID_Z]   );

}

void WiiController::accel(uint8_t nx, uint8_t ny, uint8_t nz) {
  // dumb simple for now, todo threshold here to calibrate
  func("accel callback: x%u y%u z%u");
  if(nx!=x) x=nx;
  if(ny!=y) y=ny;
  if(nz!=z) z=nz;
  accel_changed = true;
}

WiiController::WiiController()
:Controller() {
  
  accel_changed = false;

  tmp = this; // this shouldn't be here, when cwiid callback gets fixed

  set_name("WiiCtrl");
}

WiiController::~WiiController() {

  // need to find out how to cleanup?
  // no mention about it in libwii
  //
  // disconnect!!! ftw !!!
  
}

bool WiiController::connect(char *hwaddr) {
  
  // if argument is NULL look for any wiimote
  if(hwaddr == NULL) bdaddr = *BDADDR_ANY;
  else str2ba(hwaddr,&bdaddr);
  
  act("Detecting WiiMote (press A+B on it to handshake)");
  
  wiimote = cwiid_open(&bdaddr, 0);
  if(!wiimote) {
    error("unable to connect to WiiMote");
    return false;
  } else
    act("WiiMote connected");
  
  if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
    error("unable to set wiimote message callback");
    cwiid_close(wiimote);
    return false;
  }

  // activate acceleration by default (todo switches)
 
  // for more activation switches see wmdemo.c in cwiid
  unsigned char rpt_mode = 0;

  toggle_bit(rpt_mode, CWIID_RPT_ACC);
  cwiid_set_rpt_mode(wiimote, rpt_mode);

  cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC); // enable messages
  
}

bool WiiController::init(JSContext *env, JSObject *obj) {

  jsenv = env;
  jsobj = obj;

  initialized = true;
  return(true);
  
}

int WiiController::poll() {

  dispatch();

}

int WiiController::dispatch() {
  int res;
  char funcname[512];
  char keyname[512];

  if(accel_changed)
    Controller::JSCall("acceleration", 3, "uuu", x, y, z );


}

void WiiController::print_state(struct cwiid_state *state) {
	int i;
	int valid_source = 0;

	act("Report Mode:");
	if (state->rpt_mode & CWIID_RPT_STATUS) act(" STATUS");
	if (state->rpt_mode & CWIID_RPT_BTN) act(" BTN");
	if (state->rpt_mode & CWIID_RPT_ACC) act(" ACC");
	if (state->rpt_mode & CWIID_RPT_IR) act(" IR");
	if (state->rpt_mode & CWIID_RPT_NUNCHUK) act(" NUNCHUK");
	if (state->rpt_mode & CWIID_RPT_CLASSIC) act(" CLASSIC");
	
	act("Active LEDs:");
	if (state->led & CWIID_LED1_ON) act(" 1");
	if (state->led & CWIID_LED2_ON) act(" 2");
	if (state->led & CWIID_LED3_ON) act(" 3");
	if (state->led & CWIID_LED4_ON) act(" 4");

	act("Rumble: %s", state->rumble ? "On" : "Off");

	act("Battery: %d%%",
	       (int)(100.0 * state->battery / CWIID_BATTERY_MAX));

	act("Buttons: %X", state->buttons);

	act("Acc: x=%d y=%d z=%d", state->acc[CWIID_X], state->acc[CWIID_Y],
	       state->acc[CWIID_Z]);

	act("IR: ");
	for (i = 0; i < CWIID_IR_SRC_COUNT; i++) {
		if (state->ir_src[i].valid) {
			valid_source = 1;
			act("(%d,%d) ", state->ir_src[i].pos[CWIID_X],
			                   state->ir_src[i].pos[CWIID_Y]);
		}
	}
	if (!valid_source) {
		act("no sources detected");
	}

	switch (state->ext_type) {
	case CWIID_EXT_NONE:
		act("No extension");
		break;
	case CWIID_EXT_UNKNOWN:
		act("Unknown extension attached");
		break;
	case CWIID_EXT_NUNCHUK:
		act("Nunchuk: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
		       "acc.z=%d", state->ext.nunchuk.buttons,
		       state->ext.nunchuk.stick[CWIID_X],
		       state->ext.nunchuk.stick[CWIID_Y],
		       state->ext.nunchuk.acc[CWIID_X],
		       state->ext.nunchuk.acc[CWIID_Y],
		       state->ext.nunchuk.acc[CWIID_Z]);
		break;
	case CWIID_EXT_CLASSIC:
		act("Classic: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
		       "l=%d r=%d", state->ext.classic.buttons,
		       state->ext.classic.l_stick[CWIID_X],
		       state->ext.classic.l_stick[CWIID_Y],
		       state->ext.classic.r_stick[CWIID_X],
		       state->ext.classic.r_stick[CWIID_Y],
		       state->ext.classic.l, state->ext.classic.r);
		break;
	}
}