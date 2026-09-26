#include "plClipboard.h"
#include <string_theory/string>
#import <AppKit/AppKit.h>

bool plClipboard::IsTextInClipboard()
{
    return [[NSPasteboard generalPasteboard] availableTypeFromArray:@[NSPasteboardTypeString]] != nil;
}

ST::string plClipboard::GetClipboardText()
{
    NSString* text = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
    return text ? ST::string::from_utf8([text UTF8String]) : ST::string();
}

void plClipboard::SetClipboardText(const ST::string& text)
{
    if (text.empty())
        return;

    NSPasteboard* pb = [NSPasteboard generalPasteboard];
    [pb clearContents];
    [pb setString:[NSString stringWithUTF8String:text.c_str()] forType:NSPasteboardTypeString];
}
