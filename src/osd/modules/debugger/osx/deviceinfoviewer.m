// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  deviceinfoviewer.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "deviceinfoviewer.h"


@implementation MAMEDeviceInfoViewer

- (NSTextField *)makeLabel:(NSString *)text {
	NSTextField *const result = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 14)];
	[result setAutoresizingMask:NSViewMinYMargin];
	[[result cell] setControlSize:NSSmallControlSize];
	[result setEditable:NO];
	[result setSelectable:NO];
	[result setBezeled:NO];
	[result setBordered:NO];
	[result setDrawsBackground:NO];
	[result setAlignment:NSRightTextAlignment];
	[result setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	[result setStringValue:text];
	[result sizeToFit];
	return result;
}


- (NSTextField *)makeField:(NSString *)text {
	NSTextField *const result = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 14)];
	[result setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[[result cell] setControlSize:NSSmallControlSize];
	[result setEditable:NO];
	[result setSelectable:YES];
	[result setBezeled:NO];
	[result setBordered:NO];
	[result setDrawsBackground:NO];
	[result setAlignment:NSLeftTextAlignment];
	[result setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	[result setStringValue:text];
	[result sizeToFit];
	return result;
}


- (NSBox *)makeBox:(NSString *)t toFit:(NSView *)v {
	NSBox *const result = [[NSBox alloc] initWithFrame:NSMakeRect(0, 0, [v frame].size.width - 34, 32)];
	[result setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[result setTitle:t];
	[result setBoxType:NSBoxPrimary];
	[result setBorderType:NSLineBorder];
	[result setContentViewMargins:NSMakeSize(0, 0)];
	[result setAutoresizesSubviews:YES];
	return result;
}


- (void)addLabel:(NSString *)l withWidth:(CGFloat)w andField:(NSString *)f toView:(NSView *)v {
	NSTextField *const label = [self makeLabel:l];
	NSTextField *const field = [self makeField:f];
	CGFloat const height = MAX([label frame].size.height, [field frame].size.height);
	NSSize space = [v bounds].size;
	space.width = MAX(space.width, [field frame].size.width + w + 52);
	space.height += height + 8;
	[label setFrame:NSMakeRect(25, 20, w, height)];
	[field setFrame:NSMakeRect(w + 27, 20, space.width - w - 52, height)];
	[v setFrameSize:space];
	[v addSubview:label];
	[v addSubview:field];
	[label release];
	[field release];
}


- (void)addField:(NSString *)f toBox:(NSBox *)b {
	NSTextField *const field = [self makeField:f];
	NSSize space = [b frame].size;
	space.width = MAX(space.width, [field frame].size.width + 32);
	space.height += [field frame].size.height + 8;
	[field setFrame:NSMakeRect(15, 14, space.width - 32, [field frame].size.height)];
	[b setFrameSize:space];
	[[b contentView] addSubview:field];
	[field release];
}


- (void)addBox:(NSBox *)b toView:(NSView *)v {
	[b setFrameOrigin:NSMakePoint(17, 16)];
	NSSize space = [v frame].size;
	space.width = MAX(space.width, [b frame].size.width + 34);
	space.height += [b frame].size.height + 4;
	[v setFrameSize:space];
	[v addSubview:b];
}


- (id)initWithDevice:(device_t &)d machine:(running_machine &)m console:(MAMEDebugConsole *)c {
	NSView			*contentView;
	NSScrollView	*contentScroll;

	if (!(self = [super initWithMachine:m
								  title:[NSString stringWithFormat:@"Device %s", d.tag()]
								console:c]))
	{
		return nil;
	}
	device = &d;

	// Create a view to hold everything
	contentView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 320, 32)];
	[contentView setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[contentView setAutoresizesSubviews:YES];

	// add the stuff that's always present
	[self addLabel:@"Tag:"
		 withWidth:100
		  andField:[NSString stringWithUTF8String:device->tag()]
			toView:contentView];
	[self addLabel:@"Name:"
		 withWidth:100
		  andField:[NSString stringWithUTF8String:device->name()]
			toView:contentView];
	[self addLabel:@"Shortname:"
		 withWidth:100
		  andField:[NSString stringWithUTF8String:device->shortname()]
			toView:contentView];

	// add interfaces if present
	device_interface *interface = device->first_interface();
	if (interface != NULL)
	{
		NSBox *const interfacesBox = [self makeBox:@"Interfaces" toFit:contentView];
		while (interface != NULL)
		{
			[self addField:[NSString stringWithUTF8String:interface->interface_type()]
					 toBox:interfacesBox];
			interface = interface->interface_next();
		}
		[self addBox:interfacesBox toView:contentView];
		[interfacesBox release];
	}

	// add memory maps if present
	device_memory_interface *memory;
	if (device->interface(memory))
	{
		NSBox *memoryBox = nil;
		for (address_spacenum i = AS_0; i < ADDRESS_SPACES; i++)
		{
			if (memory->has_space(i))
			{
				if (memoryBox == nil)
					memoryBox = [self makeBox:@"Memory maps" toFit:contentView];
				[self addField:[NSString stringWithUTF8String:memory->space_config(i)->name()]
						 toBox:memoryBox];
			}
		}
		if (memoryBox != nil)
		{
			[self addBox:memoryBox toView:contentView];
			[memoryBox release];
		}
	}

	// create a scroll view for holding everything
	NSSize desired = [NSScrollView frameSizeForContentSize:[contentView frame].size
									 hasHorizontalScroller:YES
									   hasVerticalScroller:YES
												borderType:NSNoBorder];
	[window setContentSize:desired];
	contentScroll = [[NSScrollView alloc] initWithFrame:[[window contentView] bounds]];
	[contentScroll setDrawsBackground:NO];
	[contentScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[contentScroll setHasHorizontalScroller:YES];
	[contentScroll setHasVerticalScroller:YES];
	[contentScroll setAutohidesScrollers:YES];
	[contentScroll setBorderType:NSNoBorder];
	[contentScroll setDocumentView:contentView];
	[contentView release];
	[[window contentView] addSubview:contentScroll];
	[contentScroll release];

	// calculate the optimal size for everything
	[self cascadeWindowWithDesiredSize:NSMakeSize(320, 240) forView:contentScroll];

	// don't forget the result
	return self;
}

@end