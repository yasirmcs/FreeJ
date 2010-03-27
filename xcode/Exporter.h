/*  FreeJ
 *  (c) Copyright 2010 Robin Gareus <robin@gareus.org>
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
 */

#ifndef __EXPORTER_H__
#define __EXPORTER_H__

@class CVScreenView;
//@class Exporter;

@protocol Exporter 
- (BOOL)setOutputFile:(NSString *)path;
- (BOOL)startExport:(int)fps width:(int)w height:(int)h;
- (void)stopExport;
- (BOOL)isRunning;
@end

#endif
